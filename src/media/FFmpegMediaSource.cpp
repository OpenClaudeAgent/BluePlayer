#include "media/FFmpegMediaSource.hpp"

#include "media/FFmpegBridge.hpp"

#include <QImage>
#include <QMetaObject>
#include <QVideoFrame>
#include <QVideoSink>
#include <Qt>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace blueplayer::media {

FFmpegMediaSource::FFmpegMediaSource(QObject* parent) : QObject(parent) {
  FFmpegBridge::ensureInitialized();
}

FFmpegMediaSource::~FFmpegMediaSource() {
  stop();
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
  emit playingChanged(true);

  m_decodeThread = std::make_unique<std::thread>([this]() {
    decodeLoop(m_currentFile);
    m_running = false;
    emit playingChanged(false);
  });
}

void FFmpegMediaSource::stop() {
  if (!m_running) {
    return;
  }

  m_stopRequested = true;
  if (m_decodeThread && m_decodeThread->joinable()) {
    m_decodeThread->join();
  }
  m_decodeThread.reset();
  m_running = false;
  emit playingChanged(false);
}

void FFmpegMediaSource::decodeLoop(QString path) {
  AVFormatContext* formatContext = nullptr;
  AVCodecContext* codecContext = nullptr;
  SwsContext* swsContext = nullptr;
  AVPacket* packet = av_packet_alloc();
  AVFrame* decodedFrame = av_frame_alloc();
  AVFrame* convertedFrame = av_frame_alloc();
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
    if (swsContext) {
      sws_freeContext(swsContext);
    }
    if (codecContext) {
      avcodec_free_context(&codecContext);
    }
    if (formatContext) {
      avformat_close_input(&formatContext);
    }
  };

  if (!packet || !decodedFrame || !convertedFrame) {
    cleanup();
    return;
  }

  auto failEarly = [&]() {
    cleanup();
  };

  // Options spécifiques pour les flux HLS live (Twitch)
  bool isHlsStream = path.contains(QStringLiteral(".m3u8")) || path.contains(QStringLiteral("usher.ttvnw.net"));
  if (isHlsStream) {
    // Démarrer près du live edge (-3 segments avant la fin)
    av_dict_set(&options, "live_start_index", "-3", 0);
    // Permettre tous les types d'extensions
    av_dict_set(&options, "allowed_extensions", "ALL", 0);
    // Réutiliser les connexions HTTP
    av_dict_set(&options, "http_persistent", "1", 0);
    // Timeout de connexion raisonnable (5 secondes)
    av_dict_set(&options, "timeout", "5000000", 0);
    // IMPORTANT: Ne pas ouvrir plusieurs connexions en parallèle
    av_dict_set(&options, "http_multiple", "0", 0);
    // Sélectionner la meilleure qualité par bande passante (au lieu de toutes)
    // max_reload limite les reloads de playlist
    av_dict_set(&options, "max_reload", "3", 0);
  }

  if (avformat_open_input(&formatContext, path.toUtf8().constData(), nullptr, &options) != 0) {
    failEarly();
    return;
  }

  // Options pour la recherche de stream info (limiter le temps de probe)
  formatContext->probesize = 1024 * 1024;  // 1MB max
  formatContext->max_analyze_duration = 3 * AV_TIME_BASE;  // 3 secondes max

  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    failEarly();
    return;
  }

  int videoStreamIndex = -1;
  for (unsigned idx = 0; idx < formatContext->nb_streams; ++idx) {
    if (formatContext->streams[idx]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = static_cast<int>(idx);
      break;
    }
  }

  if (videoStreamIndex < 0) {
    failEarly();
    return;
  }

  AVCodecParameters* codecParameters = formatContext->streams[videoStreamIndex]->codecpar;
  const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
  if (!codec) {
    failEarly();
    return;
  }

  codecContext = avcodec_alloc_context3(codec);
  if (!codecContext) {
    failEarly();
    return;
  }

  if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
    failEarly();
    return;
  }

  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    failEarly();
    return;
  }

  int currentWidth = codecContext->width;
  int currentHeight = codecContext->height;
  AVPixelFormat currentPixFmt = codecContext->pix_fmt;
  const AVPixelFormat targetFormat = AV_PIX_FMT_RGB32;

  // Créer le contexte swscale initial
  auto createSwsContext = [&](int width, int height, AVPixelFormat pixFmt) -> bool {
    if (swsContext) {
      sws_freeContext(swsContext);
      swsContext = nullptr;
    }
    
    if (width <= 0 || height <= 0) {
      return false;
    }
    
    swsContext = sws_getContext(width,
                                height,
                                pixFmt,
                                width,
                                height,
                                targetFormat,
                                SWS_FAST_BILINEAR,
                                nullptr,
                                nullptr,
                                nullptr);
    return swsContext != nullptr;
  };

  if (!createSwsContext(currentWidth, currentHeight, currentPixFmt)) {
    failEarly();
    return;
  }

  int bufferSize = av_image_get_buffer_size(targetFormat, currentWidth, currentHeight, 1);
  std::vector<uint8_t> buffer(static_cast<size_t>(bufferSize));
  
  // Initialiser le buffer de sortie
  av_image_fill_arrays(convertedFrame->data,
                       convertedFrame->linesize,
                       buffer.data(),
                       targetFormat,
                       currentWidth,
                       currentHeight,
                       1);

  // Calculer le délai entre frames basé sur le framerate du stream
  AVRational frameRate = formatContext->streams[videoStreamIndex]->avg_frame_rate;
  double fps = (frameRate.num > 0 && frameRate.den > 0) ? 
               static_cast<double>(frameRate.num) / frameRate.den : 30.0;
  // Limiter à un range raisonnable
  if (fps < 1.0) fps = 30.0;
  if (fps > 120.0) fps = 60.0;
  
  const auto frameDelay = std::chrono::microseconds(static_cast<int64_t>(1000000.0 / fps));
  auto lastFrameTime = std::chrono::steady_clock::now();

  while (!m_stopRequested) {
    if (av_read_frame(formatContext, packet) < 0) {
      // Pour les streams HLS live, on ne sort pas de la boucle sur erreur de lecture
      // On attend un peu et on réessaie
      if (isHlsStream) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      
      // Pour les fichiers locaux, on flush le décodeur et on sort
      avcodec_send_packet(codecContext, nullptr);
      while (avcodec_receive_frame(codecContext, decodedFrame) == 0) {
        if (swsContext && currentWidth > 0 && currentHeight > 0) {
          sws_scale(swsContext,
                    decodedFrame->data,
                    decodedFrame->linesize,
                    0,
                    currentHeight,
                    convertedFrame->data,
                    convertedFrame->linesize);

          QImage frameImage(convertedFrame->data[0],
                            currentWidth,
                            currentHeight,
                            convertedFrame->linesize[0],
                            QImage::Format_RGB32);

          deliverFrame(QVideoFrame(frameImage.copy()));
        }
      }
      break;
    }

    if (packet->stream_index != videoStreamIndex) {
      av_packet_unref(packet);
      continue;
    }

    if (avcodec_send_packet(codecContext, packet) < 0) {
      av_packet_unref(packet);
      continue;
    }

    while (avcodec_receive_frame(codecContext, decodedFrame) == 0) {
      // Vérifier si la résolution ou le format a changé
      if (decodedFrame->width != currentWidth || 
          decodedFrame->height != currentHeight ||
          static_cast<AVPixelFormat>(decodedFrame->format) != currentPixFmt) {
        
        currentWidth = decodedFrame->width;
        currentHeight = decodedFrame->height;
        currentPixFmt = static_cast<AVPixelFormat>(decodedFrame->format);
        
        if (currentWidth <= 0 || currentHeight <= 0) {
          continue;  // Frame invalide, skip
        }
        
        // Recréer le contexte swscale
        if (!createSwsContext(currentWidth, currentHeight, currentPixFmt)) {
          continue;  // Impossible de créer le contexte, skip cette frame
        }
        
        // Réallouer le buffer
        bufferSize = av_image_get_buffer_size(targetFormat, currentWidth, currentHeight, 1);
        buffer.resize(static_cast<size_t>(bufferSize));
      }
      
      // Skip si dimensions invalides
      if (currentWidth <= 0 || currentHeight <= 0 || !swsContext) {
        continue;
      }

      // Contrôle du framerate - attendre si nécessaire
      auto now = std::chrono::steady_clock::now();
      auto elapsed = now - lastFrameTime;
      if (elapsed < frameDelay) {
        std::this_thread::sleep_for(frameDelay - elapsed);
      }
      lastFrameTime = std::chrono::steady_clock::now();

      // Configurer le buffer de sortie
      av_image_fill_arrays(convertedFrame->data,
                           convertedFrame->linesize,
                           buffer.data(),
                           targetFormat,
                           currentWidth,
                           currentHeight,
                           1);

      sws_scale(swsContext,
                decodedFrame->data,
                decodedFrame->linesize,
                0,
                currentHeight,
                convertedFrame->data,
                convertedFrame->linesize);

      QImage frameImage(convertedFrame->data[0],
                        currentWidth,
                        currentHeight,
                        convertedFrame->linesize[0],
                        QImage::Format_RGB32);

      QVideoFrame videoFrame(frameImage.copy());
      deliverFrame(videoFrame);
    }

    av_packet_unref(packet);
  }

  cleanup();
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

