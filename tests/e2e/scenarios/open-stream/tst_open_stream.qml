import QtQuick
import QtTest

/**
 * E2E Scenario: Open Stream
 * Tests the flow of opening a live stream from the home view.
 */
Item {
    id: root
    width: 1280
    height: 720

    Loader {
        id: appLoader
        anchors.fill: parent
        source: "file://" + E2E_QML_PATH + "/main.qml"
        asynchronous: false
    }

    TestCase {
        id: testCase
        name: "E2E_OpenStream"
        when: windowShown && appLoader.status === Loader.Ready

        property var mainWindow: null

        function initTestCase() {
            mainWindow = appLoader.item
            verify(mainWindow !== null, "Main window should load")
        }

        function test_01_home_view_displayed() {
            wait(1500)
            compare(mainWindow.currentView, "home", "Should show home view when authenticated")
        }

        function test_02_streams_loaded() {
            wait(1500)
            var streamCard = findChildByPrefix(mainWindow, "streamCard_")
            verify(streamCard !== null, "Stream cards should be displayed")
        }

        function test_03_click_stream_opens_player() {
            wait(1500)
            
            var streamCard = findChildByPrefix(mainWindow, "streamCard_")
            verify(streamCard !== null, "Stream card should exist")
            verify(streamCard.visible, "Stream card should be visible")
            
            mouseClick(streamCard)
            wait(1500)
            
            compare(mainWindow.currentView, "player", "Should navigate to player view")
            
            var playerView = findChild(mainWindow, "playerView")
            verify(playerView !== null, "PlayerView should exist")
            
            wait(1500)
            
            verify(!playerView.showError, "Player should not show error")
            compare(playerView.streamerLogin, "teststreamer", "Player should have correct streamer")
            verify(playerView.hlsUrl.length > 0, "Player should have HLS URL")
        }

        function findChild(parent, objectName) {
            if (!parent) return null
            if (parent.objectName === objectName) return parent
            if (parent.children) {
                for (var i = 0; i < parent.children.length; i++) {
                    var found = findChild(parent.children[i], objectName)
                    if (found) return found
                }
            }
            if (parent.contentItem) {
                var found = findChild(parent.contentItem, objectName)
                if (found) return found
            }
            return null
        }

        function findChildByPrefix(parent, prefix) {
            if (!parent) return null
            if (parent.objectName && parent.objectName.indexOf(prefix) === 0) return parent
            if (parent.children) {
                for (var i = 0; i < parent.children.length; i++) {
                    var found = findChildByPrefix(parent.children[i], prefix)
                    if (found) return found
                }
            }
            if (parent.contentItem) {
                var found = findChildByPrefix(parent.contentItem, prefix)
                if (found) return found
            }
            return null
        }
    }
}
