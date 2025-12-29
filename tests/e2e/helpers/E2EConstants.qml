pragma Singleton
import QtQuick

/**
 * E2E Testing Constants - QML mirror of E2EConstants.hpp
 *
 * Usage in tests:
 *   import BluePlayer.E2E 1.0
 *   E2EConstants.mainWindow
 *   E2EConstants.timeoutMedium
 */
QtObject {
    // =========================================================================
    // QML ObjectNames
    // =========================================================================

    // Main components
    readonly property string mainWindow: "mainWindow"
    readonly property string playerView: "playerView"
    readonly property string homeView: "homeView"
    readonly property string loginRoot: "loginRoot"

    // Buttons
    readonly property string loginButton: "loginButton"
    readonly property string playButton: "playButton"
    readonly property string pauseButton: "pauseButton"
    readonly property string fullscreenButton: "fullscreenButton"
    readonly property string volumeButton: "volumeButton"
    readonly property string chatButton: "chatButton"
    readonly property string chatToggleButton: "chatToggleButton"
    readonly property string qualityButton: "qualityButton"
    readonly property string pipButton: "pipButton"

    // Cards and lists
    readonly property string streamCardPrefix: "streamCard_"
    readonly property string vodCardPrefix: "vodCard_"
    readonly property string categoryCardPrefix: "categoryCard_"
    readonly property string searchResultPrefix: "searchResult_"

    // Panels
    readonly property string preferencesPanel: "preferencesPanel"
    readonly property string cacheManagerPanel: "cacheManagerPanel"
    readonly property string chatPanel: "chatPanel"

    // Quality selector
    readonly property string qualityControl: "qualityControl"
    readonly property string qualityPopup: "qualityPopup"
    readonly property string qualityOptionPrefix: "qualityOption_"
    readonly property string qualityButtonText: "qualityButtonText"
    readonly property string qualityToast: "qualityToast"

    // Chat components
    readonly property string chatMessageList: "chatMessageList"
    readonly property string chatMessageInput: "chatMessageInput"

    // Search components
    readonly property string searchField: "searchField"
    readonly property string searchResultsPopup: "searchResultsPopup"

    // Controls
    readonly property string seekBar: "seekBar"
    readonly property string volumeSlider: "volumeSlider"
    readonly property string searchBar: "searchBar"

    // =========================================================================
    // Timeouts (milliseconds)
    // =========================================================================

    // Short operations (UI animations, hover effects)
    readonly property int timeoutShort: 500

    // Medium operations (API responses, view transitions)
    readonly property int timeoutMedium: 1500

    // Long operations (stream loading, heavy processing)
    readonly property int timeoutLong: 5000

    // Video playback verification
    readonly property int timeoutPlayback: 10000

    // Application startup
    readonly property int timeoutAppReady: 3000

    // Network retry
    readonly property int timeoutNetworkRetry: 2000

    // =========================================================================
    // Test Configuration
    // =========================================================================

    readonly property string testUserLogin: "testuser"
    readonly property string testUserId: "99999"
    readonly property string testClientId: "e2e_test_client_id"

    // =========================================================================
    // Helper functions
    // =========================================================================

    /**
     * Build a stream card objectName from index.
     * @param index The stream index
     * @return objectName like "streamCard_0"
     */
    function streamCardName(index: int): string {
        return streamCardPrefix + index
    }

    /**
     * Build a VOD card objectName from index.
     * @param index The VOD index
     * @return objectName like "vodCard_0"
     */
    function vodCardName(index: int): string {
        return vodCardPrefix + index
    }

    /**
     * Build a search result objectName from index.
     * @param index The search result index
     * @return objectName like "searchResult_0"
     */
    function searchResultName(index: int): string {
        return searchResultPrefix + index
    }

    /**
     * Build a quality option objectName from index.
     * @param index The quality option index
     * @return objectName like "qualityOption_0"
     */
    function qualityOptionName(index: int): string {
        return qualityOptionPrefix + index
    }
}
