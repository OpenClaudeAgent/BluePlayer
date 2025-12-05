#include "media/FFmpegMediaSource.hpp"

#include "media/FFmpegBridge.hpp"

#include <QImage>
#include <QMetaObject>
#include <QVideoFrame>
#include <QVideoSink>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QDebug>
#include <Qt>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <cmath>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

namespace blueplayer::media {

FFmpegMediaSource::FFmpegMediaSource(QObject* parent) : QObject(parent) {
  FFmpegBridge::ensureInitialized();
}

FFmpegMediaSource::~FFmpegMediaSource() {
  stop();
}

void FFmpegMediaSource::enqueueFrame(QVideoFrame&& frame) {
  std::unique_lock<std::mutex> lock(m_bufferMutex);
  
  // Timeout court pour ne pas bloquer l'audio (5ms max)
  bool hasSpace = m_bufferCondition.wait_for(lock, std::chrono::milliseconds(5), [this]() {
    return m_stopRequested || static_cast<int>(m_frameBuffer.size()) < m_bufferSize;
  });
  
  if (m_stopRequested) return;
  
  // Si pas de place après timeout, skip cette frame (l'audio continue)
  if (!hasSpace) return;
  
  m_frameBuffer.push(std::move(frame));
  m_bufferCondition.notify_one();
  
  int currentSize = static_cast<int>(m_frameBuffer.size());
  if (m_buffering && currentSize >= MIN_BUFFER_BEFORE_PLAY) {
    m_buffering = false;
    emit bufferingChanged(false);
  }
}

bool FFmpegMediaSource::dequeueFrame(QVideoFrame& frame) {
  std::unique_lock<std::mutex> lock(m_bufferMutex);
  
  // Attendre qu'il y ait des frames ou qu'on doive s'arrêter
  m_bufferCondition.wait(lock, [this]() {
    return m_stopRequested || !m_frameBuffer.empty();
  });
  
  if (m_stopRequested && m_frameBuffer.empty()) {
    return false;
  }
  
  if (m_frameBuffer.empty()) {
    return false;
  }
  
  frame = std::move(m_frameBuffer.front());
  m_frameBuffer.pop();
  
  // Signaler qu'il y a de la place
  m_bufferCondition.notify_one();
  
  // Détecter sous-remplissage du buffer
  if (!m_buffering && m_frameBuffer.empty()) {
    m_buffering = true;
    emit bufferingChanged(true);
  }
  
  return true;
}

QVideoSink* FFmpegMediaSource::videoSink() const {
  return m_videoSink;
}

void FFmpegMediaSource::setVideoSink(QVideoSink* sink) {
  if (m_videoSink == sink) {
    return;
  }

  m_videoSink = sink;
  emit videoSinkChanged();
}

bool FFmpegMediaSource::open(QString filePath) {
  stop();
  if (filePath.isEmpty()) {
    return false;
  }

  m_currentFile = std::move(filePath);
  return true;
}

void FFmpegMediaSource::play() {
  if (m_currentFile.isEmpty() || m_running) {
    return;
  }

  m_stopRequested = false;
  m_running = true;
  m_buffering = true;
  m_paused = false;
  m_hasAudio = false;
  
  // Vider les buffers existants
  {
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    while (!m_frameBuffer.empty()) {
      m_frameBuffer.pop();
    }
  }
  {
    std::lock_guard<std::mutex> lock(m_audioMutex);
    while (!m_audioBuffer.empty()) {
      m_audioBuffer.pop();
    }
  }
  
  emit playingChanged(true);
  emit bufferingChanged(true);
  emit pausedChanged(false);

  // Thread de décodage - remplit les buffers vidéo et audio
  m_decodeThread = std::make_unique<std::thread>([this]() {
    decodeLoop(m_currentFile);
  });
  
  // Thread de rendu vidéo - consomme le buffer vidéo
  m_renderThread = std::make_unique<std::thread>([this]() {
    renderLoop();
    m_running = false;
    emit playingChanged(false);
  });
}

void FFmpegMediaSource::stop() {
  if (!m_running) {
    return;
  }

  m_stopRequested = true;
  m_paused = false;
  
  // Réveiller les threads en attente
  m_bufferCondition.notify_all();
  m_pauseCondition.notify_all();
  m_audioCondition.notify_all();
  
  if (m_decodeThread && m_decodeThread->joinable()) {
    m_decodeThread->join();
  }
  m_decodeThread.reset();
  
  if (m_renderThread && m_renderThread->joinable()) {
    m_renderThread->join();
  }
  m_renderThread.reset();
  
  if (m_audioThread && m_audioThread->joinable()) {
    m_audioThread->join();
  }
  m_audioThread.reset();
  
  // Arrêter la sortie audio
  if (m_audioSink) {
    m_audioSink->stop();
    m_audioSink.reset();
  }
  m_audioDevice = nullptr;
  
  // Vider les buffers
  {
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    while (!m_frameBuffer.empty()) {
      m_frameBuffer.pop();
    }
  }
  {
    std::lock_guard<std::mutex> lock(m_audioMutex);
    while (!m_audioBuffer.empty()) {
      m_audioBuffer.pop();
    }
  }
  
  m_running = false;
  m_hasAudio = false;
  emit playingChanged(false);
}

void FFmpegMediaSource::pause() {
  if (!m_running || m_paused) {
    return;
  }
  
  m_paused = true;
  
  // Pause audio output
  if (m_audioSink) {
    m_audioSink->suspend();
  }
  
  emit pausedChanged(true);
}

void FFmpegMediaSource::resume() {
  if (!m_running || !m_paused) {
    return;
  }
  
  m_paused = false;
  
  // Resume audio output
  if (m_audioSink) {
    m_audioSink->resume();
  }
  
  // Réveiller les threads en attente
  m_pauseCondition.notify_all();
  
  emit pausedChanged(false);
}

void FFmpegMediaSource::togglePause() {
  if (m_paused) {
    resume();
  } else {
    pause();
  }
}

void FFmpegMediaSource::setVolume(float vol) {
  vol = std::clamp(vol, 0.0f, 1.0f);
  float oldVolume = m_volume.exchange(vol);
  
  if (std::abs(oldVolume - vol) > 0.001f) {
    if (m_audioSink) {
      m_audioSink->setVolume(vol);
    }
    emit volumeChanged(vol);
  }
}

void FFmpegMediaSource::setMuted(bool muted) {
  bool oldMuted = m_muted.exchange(muted);
  
  if (oldMuted != muted) {
    if (m_audioSink) {
      m_audioSink->setVolume(muted ? 0.0f : m_volume.load());
    }
    emit mutedChanged(muted);
  }
}

void FFmpegMediaSource::enqueueAudio(QByteArray&& audioData) {
  std::unique_lock<std::mutex> lock(m_audioMutex);
  
  // Buffer audio: 10 frames (~200ms) - compromis latence/stabilité
  constexpr size_t MAX_AUDIO_BUFFER = 10;
  
  // Si buffer plein, drop les anciennes données
  if (m_audioBuffer.size() >= MAX_AUDIO_BUFFER) {
    m_audioBuffer.pop();
  }
  
  if (m_stopRequested) return;
  
  m_audioBuffer.push(std::move(audioData));
  m_audioCondition.notify_one();
}

bool FFmpegMediaSource::dequeueAudio(QByteArray& audioData) {
  std::unique_lock<std::mutex> lock(m_audioMutex);
  
  // Don't wait too long - use a short timeout for low latency
  if (m_audioBuffer.empty()) {
    m_audioCondition.wait_for(lock, std::chrono::milliseconds(5), [this]() {
      return m_stopRequested || !m_audioBuffer.empty();
    });
  }
  
  if (m_stopRequested && m_audioBuffer.empty()) {
    return false;
  }
  
  if (m_audioBuffer.empty()) {
    return false;
  }
  
  audioData = std::move(m_audioBuffer.front());
  m_audioBuffer.pop();
  m_audioCondition.notify_one();
  
  return true;
}

void FFmpegMediaSource::initAudioOutput(int sampleRate, int channels) {
  QAudioFormat format;
  format.setSampleRate(sampleRate);
  format.setChannelCount(channels);
  format.setSampleFormat(QAudioFormat::Int16);
  
  QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
  
  qDebug() << "[Audio] Requested format: rate=" << sampleRate << "channels=" << channels << "Int16";
  
  if (!audioDevice.isFormatSupported(format)) {
    qWarning() << "[Audio] Format not supported! Trying 48000Hz stereo...";
    // Essayer un format standard
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    
    if (!audioDevice.isFormatSupported(format)) {
      qWarning() << "[Audio] 48000Hz stereo not supported, using device preferred";
      format = audioDevice.preferredFormat();
    }
  }
  
  // Log le format réellement utilisé
  qDebug() << "[Audio] Using format: rate=" << format.sampleRate() 
           << "channels=" << format.channelCount()
           << "format=" << format.sampleFormat();
  
  m_audioSink = std::make_unique<QAudioSink>(audioDevice, format);
  
  // Buffer: 200ms pour plus de stabilité
  int bytesPerSample = format.bytesPerSample();
  int bufferSize = format.sampleRate() * format.channelCount() * bytesPerSample / 5; // 200ms
  m_audioSink->setBufferSize(bufferSize);
  
  m_audioSink->setVolume(m_muted ? 0.0f : m_volume.load());
  m_audioDevice = m_audioSink->start();
  
  qDebug() << "[Audio] Started with buffer:" << m_audioSink->bufferSize() << "bytes";
  
  m_audioSampleRate = format.sampleRate();
  m_audioChannels = format.channelCount();
  m_hasAudio = true;
}

void FFmpegMediaSource::audioLoop() {
  QByteArray audioData;
  QByteArray pendingData;  // Données en attente d'écriture
  
  while (!m_stopRequested) {
    // Attendre si en pause
    if (m_paused) {
      std::unique_lock<std::mutex> lock(m_bufferMutex);
      m_pauseCondition.wait(lock, [this]() {
        return m_stopRequested || !m_paused;
      });
      if (m_stopRequested) break;
      continue;
    }
    
    // Si on a des données en attente, essayer de les écrire d'abord
    if (!pendingData.isEmpty() && m_audioDevice) {
      qint64 written = m_audioDevice->write(pendingData);
      if (written > 0) {
        pendingData.remove(0, static_cast<int>(written));
      }
      if (!pendingData.isEmpty()) {
        // Device pas prêt, attendre un peu
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }
    }
    
    // Récupérer de nouvelles données audio
    if (!dequeueAudio(audioData)) {
      if (m_stopRequested) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      continue;
    }
    
    if (m_audioDevice && !audioData.isEmpty()) {
      qint64 written = m_audioDevice->write(audioData);
      if (written < audioData.size()) {
        // Garder les données non écrites pour le prochain tour
        pendingData = audioData.mid(static_cast<int>(written));
      }
    }
  }
}

void FFmpegMediaSource::decodeLoop(QString path) {
  AVFormatContext* formatContext = nullptr;
  AVCodecContext* videoCodecContext = nullptr;
  AVCodecContext* audioCodecContext = nullptr;
  SwsContext* swsContext = nullptr;
  SwrContext* swrContext = nullptr;
  AVPacket* packet = av_packet_alloc();
  AVFrame* decodedFrame = av_frame_alloc();
  AVFrame* convertedFrame = av_frame_alloc();
  AVFrame* audioFrame = av_frame_alloc();
  AVDictionary* options = nullptr;

  auto cleanup = [&]() {
    if (options) {
      av_dict_free(&options);
    }
    if (packet) {
      av_packet_free(&packet);
    }
    if (convertedFrame) {
      av_frame_free(&convertedFrame);
    }
    if (decodedFrame) {
      av_frame_free(&decodedFrame);
    }
    if (audioFrame) {
      av_frame_free(&audioFrame);
    }
    if (swsContext) {
      sws_freeContext(swsContext);
    }
    if (swrContext) {
      swr_free(&swrContext);
    }
    if (videoCodecContext) {
      avcodec_free_context(&videoCodecContext);
    }
    if (audioCodecContext) {
      avcodec_free_context(&audioCodecContext);
    }
    if (formatContext) {
      avformat_close_input(&formatContext);
    }
  };

  if (!packet || !decodedFrame || !convertedFrame || !audioFrame) {
    cleanup();
    return;
  }

  auto failEarly = [&]() {
    cleanup();
  };

  // Options spécifiques pour les flux HLS live (Twitch)
  bool isHlsStream = path.contains(QStringLiteral(".m3u8")) || path.contains(QStringLiteral("ttvnw.net"));
  if (isHlsStream) {
    // === BUFFERING RÉSEAU ===
    av_dict_set(&options, "buffer_size", "2097152", 0);
    av_dict_set(&options, "reconnect", "1", 0);
    av_dict_set(&options, "reconnect_streamed", "1", 0);
    av_dict_set(&options, "reconnect_delay_max", "5", 0);
    
    // === HLS SPÉCIFIQUE ===
    av_dict_set(&options, "live_start_index", "-3", 0);
    av_dict_set(&options, "allowed_extensions", "ALL", 0);
    av_dict_set(&options, "timeout", "10000000", 0);
    av_dict_set(&options, "http_persistent", "1", 0);
    av_dict_set(&options, "http_multiple", "0", 0);
    av_dict_set(&options, "max_reload", "5", 0);
    
    // === PERFORMANCE ===
    av_dict_set(&options, "threads", "auto", 0);
  }

  if (avformat_open_input(&formatContext, path.toUtf8().constData(), nullptr, &options) != 0) {
    failEarly();
    return;
  }

  formatContext->probesize = 1024 * 1024;
  formatContext->max_analyze_duration = 3 * AV_TIME_BASE;

  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    failEarly();
    return;
  }

  // Trouver les streams video et audio
  int videoStreamIndex = -1;
  int audioStreamIndex = -1;
  
  qDebug() << "[FFmpeg] Number of streams:" << formatContext->nb_streams;
  
  for (unsigned idx = 0; idx < formatContext->nb_streams; ++idx) {
    auto codecType = formatContext->streams[idx]->codecpar->codec_type;
    qDebug() << "[FFmpeg] Stream" << idx << "type:" << codecType;
    
    if (codecType == AVMEDIA_TYPE_VIDEO && videoStreamIndex < 0) {
      videoStreamIndex = static_cast<int>(idx);
      qDebug() << "[FFmpeg] Found VIDEO stream at index" << idx;
    } else if (codecType == AVMEDIA_TYPE_AUDIO && audioStreamIndex < 0) {
      audioStreamIndex = static_cast<int>(idx);
      qDebug() << "[FFmpeg] Found AUDIO stream at index" << idx;
    }
  }
  
  qDebug() << "[FFmpeg] Video stream:" << videoStreamIndex << "Audio stream:" << audioStreamIndex;

  if (videoStreamIndex < 0) {
    failEarly();
    return;
  }

  // Initialiser le décodeur vidéo
  AVCodecParameters* videoCodecParams = formatContext->streams[videoStreamIndex]->codecpar;
  const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParams->codec_id);
  if (!videoCodec) {
    failEarly();
    return;
  }

  videoCodecContext = avcodec_alloc_context3(videoCodec);
  if (!videoCodecContext) {
    failEarly();
    return;
  }

  if (avcodec_parameters_to_context(videoCodecContext, videoCodecParams) < 0) {
    failEarly();
    return;
  }

  if (avcodec_open2(videoCodecContext, videoCodec, nullptr) < 0) {
    failEarly();
    return;
  }

  // Initialize audio decoder if available
  bool hasAudioStream = false;
  int audioSampleRate = 48000;
  int audioChannels = 2;
  
  if (audioStreamIndex >= 0) {
    AVCodecParameters* audioCodecParams = formatContext->streams[audioStreamIndex]->codecpar;
    const AVCodec* audioCodec = avcodec_find_decoder(audioCodecParams->codec_id);
    
    if (audioCodec) {
      audioCodecContext = avcodec_alloc_context3(audioCodec);
      if (audioCodecContext) {
        if (avcodec_parameters_to_context(audioCodecContext, audioCodecParams) >= 0) {
          // Enable multi-threading for audio decoder
          audioCodecContext->thread_count = 2;
          
          if (avcodec_open2(audioCodecContext, audioCodec, nullptr) >= 0) {
            audioSampleRate = audioCodecContext->sample_rate > 0 ? audioCodecContext->sample_rate : 48000;
            audioChannels = audioCodecContext->ch_layout.nb_channels > 0 ? audioCodecContext->ch_layout.nb_channels : 2;
            
            // Limit to stereo
            if (audioChannels > 2) audioChannels = 2;
            
            // Initialize audio resampler
            swrContext = swr_alloc();
            if (swrContext) {
              AVChannelLayout outLayout;
              av_channel_layout_default(&outLayout, audioChannels);
              
              av_opt_set_chlayout(swrContext, "in_chlayout", &audioCodecContext->ch_layout, 0);
              av_opt_set_chlayout(swrContext, "out_chlayout", &outLayout, 0);
              av_opt_set_int(swrContext, "in_sample_rate", audioCodecContext->sample_rate, 0);
              av_opt_set_int(swrContext, "out_sample_rate", audioSampleRate, 0);
              av_opt_set_sample_fmt(swrContext, "in_sample_fmt", audioCodecContext->sample_fmt, 0);
              av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
              
              if (swr_init(swrContext) >= 0) {
                hasAudioStream = true;
                qDebug() << "[FFmpeg] Audio codec sample rate:" << audioCodecContext->sample_rate;
                qDebug() << "[FFmpeg] Audio output sample rate:" << audioSampleRate;
                qDebug() << "[FFmpeg] Audio channels:" << audioChannels;
                
                // Initialize audio output on main thread
                QMetaObject::invokeMethod(this, [this, audioSampleRate, audioChannels]() {
                  initAudioOutput(audioSampleRate, audioChannels);
                }, Qt::BlockingQueuedConnection);
                
                // Start audio playback thread
                m_audioThread = std::make_unique<std::thread>([this]() {
                  audioLoop();
                });
              } else {
                swr_free(&swrContext);
                swrContext = nullptr;
                qDebug() << "[FFmpeg] Failed to init audio resampler";
              }
            }
          }
        }
      }
    }
  }
  
  if (!hasAudioStream) {
    qDebug() << "[FFmpeg] No audio stream available";
  }

  int currentWidth = videoCodecContext->width;
  int currentHeight = videoCodecContext->height;
  AVPixelFormat currentPixFmt = videoCodecContext->pix_fmt;
  const AVPixelFormat targetFormat = AV_PIX_FMT_RGB32;

  auto createSwsContext = [&](int width, int height, AVPixelFormat pixFmt) -> bool {
    if (swsContext) {
      sws_freeContext(swsContext);
      swsContext = nullptr;
    }
    
    if (width <= 0 || height <= 0) {
      return false;
    }
    
    swsContext = sws_getContext(width, height, pixFmt,
                                width, height, targetFormat,
                                SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    return swsContext != nullptr;
  };

  if (!createSwsContext(currentWidth, currentHeight, currentPixFmt)) {
    failEarly();
    return;
  }

  int bufferSize = av_image_get_buffer_size(targetFormat, currentWidth, currentHeight, 1);
  std::vector<uint8_t> buffer(static_cast<size_t>(bufferSize));
  
  av_image_fill_arrays(convertedFrame->data, convertedFrame->linesize,
                       buffer.data(), targetFormat, currentWidth, currentHeight, 1);

  while (!m_stopRequested) {
    // Attendre si en pause (seulement pour le décodage de fichiers locaux)
    if (m_paused && !isHlsStream) {
      std::unique_lock<std::mutex> lock(m_bufferMutex);
      m_pauseCondition.wait(lock, [this]() {
        return m_stopRequested || !m_paused;
      });
      if (m_stopRequested) break;
    }
    
    if (av_read_frame(formatContext, packet) < 0) {
      if (isHlsStream) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      
      // Flush video decoder
      avcodec_send_packet(videoCodecContext, nullptr);
      while (avcodec_receive_frame(videoCodecContext, decodedFrame) == 0) {
        if (swsContext && currentWidth > 0 && currentHeight > 0) {
          sws_scale(swsContext, decodedFrame->data, decodedFrame->linesize,
                    0, currentHeight, convertedFrame->data, convertedFrame->linesize);

          QImage frameImage(convertedFrame->data[0], currentWidth, currentHeight,
                            convertedFrame->linesize[0], QImage::Format_RGB32);
          enqueueFrame(QVideoFrame(frameImage.copy()));
        }
      }
      break;
    }

    // Traiter les paquets vidéo
    if (packet->stream_index == videoStreamIndex) {
      if (avcodec_send_packet(videoCodecContext, packet) >= 0) {
        while (avcodec_receive_frame(videoCodecContext, decodedFrame) == 0) {
          if (m_stopRequested) break;
          
          if (decodedFrame->width != currentWidth || 
              decodedFrame->height != currentHeight ||
              static_cast<AVPixelFormat>(decodedFrame->format) != currentPixFmt) {
            
            currentWidth = decodedFrame->width;
            currentHeight = decodedFrame->height;
            currentPixFmt = static_cast<AVPixelFormat>(decodedFrame->format);
            
            if (currentWidth <= 0 || currentHeight <= 0) continue;
            if (!createSwsContext(currentWidth, currentHeight, currentPixFmt)) continue;
            
            bufferSize = av_image_get_buffer_size(targetFormat, currentWidth, currentHeight, 1);
            buffer.resize(static_cast<size_t>(bufferSize));
          }
          
          if (currentWidth <= 0 || currentHeight <= 0 || !swsContext) continue;

          av_image_fill_arrays(convertedFrame->data, convertedFrame->linesize,
                               buffer.data(), targetFormat, currentWidth, currentHeight, 1);

          sws_scale(swsContext, decodedFrame->data, decodedFrame->linesize,
                    0, currentHeight, convertedFrame->data, convertedFrame->linesize);

          QImage frameImage(convertedFrame->data[0], currentWidth, currentHeight,
                            convertedFrame->linesize[0], QImage::Format_RGB32);
          enqueueFrame(QVideoFrame(frameImage.copy()));
        }
      }
    }
    // Process audio packets
    else if (hasAudioStream && packet->stream_index == audioStreamIndex && audioCodecContext && swrContext) {
      if (avcodec_send_packet(audioCodecContext, packet) >= 0) {
        while (avcodec_receive_frame(audioCodecContext, audioFrame) == 0) {
          if (m_stopRequested) break;
          
          // Calculate output buffer size
          int outSamples = swr_get_out_samples(swrContext, audioFrame->nb_samples);
          if (outSamples <= 0) continue;
          
          int outBufferSize = outSamples * audioChannels * 2; // 2 bytes per sample (S16)
          QByteArray audioData(outBufferSize, 0);
          
          uint8_t* outBuffer = reinterpret_cast<uint8_t*>(audioData.data());
          int convertedSamples = swr_convert(swrContext, &outBuffer, outSamples,
                                             const_cast<const uint8_t**>(audioFrame->data),
                                             audioFrame->nb_samples);
          
          if (convertedSamples > 0) {
            audioData.resize(convertedSamples * audioChannels * 2);
            enqueueAudio(std::move(audioData));
          }
        }
      }
    }

    av_packet_unref(packet);
  }

  cleanup();
  m_bufferCondition.notify_all();
  m_audioCondition.notify_all();
}

void FFmpegMediaSource::renderLoop() {
  // Attendre un minimum de frames avant de commencer le rendu
  {
    std::unique_lock<std::mutex> lock(m_bufferMutex);
    m_bufferCondition.wait_for(lock, std::chrono::seconds(5), [this]() {
      return m_stopRequested || static_cast<int>(m_frameBuffer.size()) >= MIN_BUFFER_BEFORE_PLAY;
    });
  }
  
  if (m_stopRequested) return;
  
  m_buffering = false;
  emit bufferingChanged(false);
  
  // Framerate cible: 60fps pour les streams Twitch (ou 30fps)
  double targetFps = 60.0;
  auto frameDelay = std::chrono::microseconds(static_cast<int64_t>(1000000.0 / targetFps));
  auto lastFrameTime = std::chrono::steady_clock::now();
  
  QVideoFrame frame;
  
  while (!m_stopRequested) {
    // Attendre si en pause
    if (m_paused) {
      std::unique_lock<std::mutex> lock(m_bufferMutex);
      m_pauseCondition.wait(lock, [this]() {
        return m_stopRequested || !m_paused;
      });
      if (m_stopRequested) break;
      lastFrameTime = std::chrono::steady_clock::now();
    }
    
    // Récupérer une frame du buffer
    if (!dequeueFrame(frame)) {
      if (m_stopRequested) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    
    // Contrôle du framerate
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - lastFrameTime;
    if (elapsed < frameDelay) {
      std::this_thread::sleep_for(frameDelay - elapsed);
    }
    lastFrameTime = std::chrono::steady_clock::now();
    
    // Envoyer la frame au VideoSink
    deliverFrame(frame);
  }
}

void FFmpegMediaSource::deliverFrame(const QVideoFrame& frame) {
  if (!m_videoSink) {
    return;
  }

  // QVideoFrame utilise le copy-on-write, donc pas besoin de copie explicite
  // La frame sera copiée seulement si nécessaire lors de l'accès
  QVideoSink* sink = m_videoSink;
  QMetaObject::invokeMethod(
      sink,
      [sink, frame]() mutable {
        if (sink) {
          sink->setVideoFrame(frame);
        }
      },
      Qt::QueuedConnection);
}

}  // namespace blueplayer::media

