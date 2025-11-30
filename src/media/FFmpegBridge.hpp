#pragma once

#include <QString>

namespace blueplayer::media {

class FFmpegBridge final {
public:
  static void ensureInitialized();
  static QString versionSummary();

private:
  static void initializeInternal();
};

}  // namespace blueplayer::media

