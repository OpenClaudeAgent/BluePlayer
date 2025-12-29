#ifndef BLUEPLAYER_TEST_E2E_CONSTANTS_HPP
#define BLUEPLAYER_TEST_E2E_CONSTANTS_HPP

#include <QString>
#include <QtTypes>

namespace blueplayer::test::e2e::constants {

// ============================================================================
// QML ObjectNames
// ============================================================================

// Main components
constexpr const char* MAIN_WINDOW = "mainWindow";
constexpr const char* PLAYER_VIEW = "playerView";
constexpr const char* HOME_VIEW = "homeView";
constexpr const char* LOGIN_ROOT = "loginRoot";

// Buttons
constexpr const char* LOGIN_BUTTON = "loginButton";
constexpr const char* PLAY_BUTTON = "playButton";
constexpr const char* PAUSE_BUTTON = "pauseButton";
constexpr const char* FULLSCREEN_BUTTON = "fullscreenButton";
constexpr const char* VOLUME_BUTTON = "volumeButton";
constexpr const char* CHAT_BUTTON = "chatButton";
constexpr const char* QUALITY_BUTTON = "qualityButton";
constexpr const char* PIP_BUTTON = "pipButton";

// Cards and lists
constexpr const char* STREAM_CARD_PREFIX = "streamCard_";
constexpr const char* VOD_CARD_PREFIX = "vodCard_";
constexpr const char* CATEGORY_CARD_PREFIX = "categoryCard_";

// Panels
constexpr const char* PREFERENCES_PANEL = "preferencesPanel";
constexpr const char* CACHE_MANAGER_PANEL = "cacheManagerPanel";
constexpr const char* CHAT_PANEL = "chatPanel";

// Controls
constexpr const char* SEEK_BAR = "seekBar";
constexpr const char* VOLUME_SLIDER = "volumeSlider";
constexpr const char* SEARCH_BAR = "searchBar";

// ============================================================================
// Timeouts (milliseconds)
// ============================================================================

namespace timeout {
    // Short operations (UI animations, hover effects)
    constexpr int SHORT = 500;
    
    // Medium operations (API responses, view transitions)
    constexpr int MEDIUM = 1500;
    
    // Long operations (stream loading, heavy processing)
    constexpr int LONG = 5000;
    
    // Video playback verification
    constexpr int PLAYBACK = 10000;
    
    // Application startup
    constexpr int APP_READY = 3000;
    
    // Network retry
    constexpr int NETWORK_RETRY = 2000;
} // namespace timeout

// ============================================================================
// Ports
// ============================================================================

namespace ports {
    // Use 0 for auto-assigned port (recommended)
    constexpr quint16 AUTO = 0;
    
    // Default fallback ports (if auto fails)
    constexpr quint16 DEFAULT_TWITCH = 18080;
    constexpr quint16 DEFAULT_HLS = 18081;
} // namespace ports

// ============================================================================
// Test Configuration
// ============================================================================

namespace config {
    // Default test user
    constexpr const char* TEST_USER_LOGIN = "testuser";
    constexpr const char* TEST_USER_ID = "99999";
    constexpr const char* TEST_CLIENT_ID = "e2e_test_client_id";
    
    // HLS settings
    constexpr int DEFAULT_SEGMENT_COUNT = 5;
    constexpr int DEFAULT_SEGMENT_DURATION = 2;
    
    // Chat settings
    constexpr int MAX_CHAT_MESSAGES = 500;
} // namespace config

// ============================================================================
// Helper functions
// ============================================================================

/**
 * @brief Build a stream card objectName from index.
 * @param index The stream index
 * @return objectName like "streamCard_0"
 */
inline QString streamCardName(int index)
{
    return QString(STREAM_CARD_PREFIX) + QString::number(index);
}

/**
 * @brief Build a VOD card objectName from index.
 * @param index The VOD index
 * @return objectName like "vodCard_0"
 */
inline QString vodCardName(int index)
{
    return QString(VOD_CARD_PREFIX) + QString::number(index);
}

} // namespace blueplayer::test::e2e::constants

#endif // BLUEPLAYER_TEST_E2E_CONSTANTS_HPP
