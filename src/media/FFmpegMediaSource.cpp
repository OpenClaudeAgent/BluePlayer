#include "media/FFmpegMediaSource.hpp"

#include "media/FFmpegBridge.hpp"

#include <QImage>
#include <QMetaObject>
#include <QVideoFrame>
#include <QVideoSink>
#include <Qt>

#include <atomic>
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

  auto cleanup = [&]() {
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

  if (avformat_open_input(&formatContext, path.toUtf8().constData(), nullptr, nullptr) != 0) {
    failEarly();
    return;
  }

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

  const int targetWidth = codecContext->width;
  const int targetHeight = codecContext->height;
  const AVPixelFormat targetFormat = AV_PIX_FMT_RGB32;

  swsContext = sws_getContext(codecContext->width,
                              codecContext->height,
                              codecContext->pix_fmt,
                              targetWidth,
                              targetHeight,
                              targetFormat,
                              SWS_BILINEAR,
                              nullptr,
                              nullptr,
                              nullptr);

  if (!swsContext) {
    failEarly();
    return;
  }

  const int bufferSize = av_image_get_buffer_size(targetFormat, targetWidth, targetHeight, 1);
  std::vector<uint8_t> buffer(static_cast<size_t>(bufferSize));
  av_image_fill_arrays(convertedFrame->data,
                       convertedFrame->linesize,
                       buffer.data(),
                       targetFormat,
                       targetWidth,
                       targetHeight,
                       1);

  while (!m_stopRequested) {
    if (av_read_frame(formatContext, packet) < 0) {
      avcodec_send_packet(codecContext, nullptr);
      while (avcodec_receive_frame(codecContext, decodedFrame) == 0) {
        sws_scale(swsContext,
                  decodedFrame->data,
                  decodedFrame->linesize,
                  0,
                  codecContext->height,
                  convertedFrame->data,
                  convertedFrame->linesize);

        QImage frameImage(convertedFrame->data[0],
                          targetWidth,
                          targetHeight,
                          convertedFrame->linesize[0],
                          QImage::Format_RGB32);

        deliverFrame(QVideoFrame(frameImage.copy()));
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
      sws_scale(swsContext,
                decodedFrame->data,
                decodedFrame->linesize,
                0,
                codecContext->height,
                convertedFrame->data,
                convertedFrame->linesize);

      // Créer QImage sans copie en utilisant les données directement
      // Note: QImage prend possession des données seulement si on utilise QImage::fromData
      // Ici on utilise un wrapper qui ne copie pas
      QImage frameImage(convertedFrame->data[0],
                        targetWidth,
                        targetHeight,
                        convertedFrame->linesize[0],
                        QImage::Format_RGB32);

      // Créer QVideoFrame avec référence partagée au lieu de copie
      // QVideoFrame fait une copie shallow si possible
      QVideoFrame videoFrame(frameImage);
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

