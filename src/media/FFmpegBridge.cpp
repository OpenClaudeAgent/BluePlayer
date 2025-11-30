#include "media/FFmpegBridge.hpp"

#include <mutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#include <QLatin1String>

namespace {

std::once_flag g_ffmpegInitFlag;

inline QString versionFrom(unsigned version) {
  return QStringLiteral("%1.%2.%3")
      .arg(AV_VERSION_MAJOR(version))
      .arg(AV_VERSION_MINOR(version))
      .arg(AV_VERSION_MICRO(version));
}

}  // namespace

namespace blueplayer::media {

void FFmpegBridge::ensureInitialized() {
  std::call_once(g_ffmpegInitFlag, [] { initializeInternal(); });
}

QString FFmpegBridge::versionSummary() {
  ensureInitialized();
  return QStringLiteral("FFmpeg %1 | avcodec %2 | avformat %3 | avutil %4")
      .arg(QLatin1String(av_version_info()))
      .arg(versionFrom(avcodec_version()))
      .arg(versionFrom(avformat_version()))
      .arg(versionFrom(avutil_version()));
}

void FFmpegBridge::initializeInternal() {
  av_log_set_level(AV_LOG_WARNING);
  avformat_network_init();
  // avcodec_register_all() est dépréciée dans les versions récentes mais reste inoffensive si définie.
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 10, 100)
  avcodec_register_all();
#endif
}

}  // namespace blueplayer::media

