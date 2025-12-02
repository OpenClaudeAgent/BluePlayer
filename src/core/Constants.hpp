#pragma once

#include <QtGlobal>

namespace blueplayer::core::constants {

// Twitch API
namespace twitch {
  constexpr const char* kAuthorizeEndpoint = "https://id.twitch.tv/oauth2/authorize";
  constexpr const char* kTokenEndpoint = "https://id.twitch.tv/oauth2/token";
  constexpr const char* kHelixBaseUrl = "https://api.twitch.tv/helix";
  constexpr quint16 kDefaultRedirectPort = 8443;
  constexpr int kDefaultStreamLimit = 100;
  constexpr int kDefaultStreamListLimit = 12;
  constexpr const char* kDefaultScope = "user:read:email user:read:follows";
  constexpr int kThumbnailWidth = 320;
  constexpr int kThumbnailHeight = 180;
}

// Media
namespace media {
  constexpr int kRefreshStreamsDelayMs = 500;
}

// UI
namespace ui {
  constexpr int kSearchDebounceDelayMs = 400;
  constexpr int kPlaceholderCardsCount = 20;
}

// Network
namespace network {
  constexpr int kDefaultCacheSize = 50 * 1024 * 1024;  // 50 MB
  constexpr int kDefaultCacheTTLSeconds = 300;  // 5 minutes
}

}  // namespace blueplayer::core::constants

