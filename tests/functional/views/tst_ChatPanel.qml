/**
 * tst_ChatPanel.qml
 * 
 * Functional UI tests for the ChatPanel component.
 * Tests connection status, message display, close signal,
 * send message behavior, and scroll functionality.
 * 
 * Refactored to use:
 * - createTemporaryObject for test isolation
 * - cleanup() for proper teardown
 * - waitForRendering instead of wait() for visual sync
 * - tryCompare for async property checks
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root

    // Detect offscreen mode - mouse events crash in offscreen
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"
    width: 600
    height: 700

    // =========================================================================
    // Mock Theme Constants
    // =========================================================================
    
    QtObject {
        id: blueTheme
        readonly property color surface: "#1C1C1E"
        readonly property color surfaceHeader: "#2C2C2E"
        readonly property color primaryText: "#FFFFFF"
        readonly property color mutedText: "#8E8E93"
        readonly property color accent: "#9147FF"
        readonly property color divider: "#3A3A3C"
        readonly property color statusConnected: "#30D158"
        readonly property color statusConnecting: "#FF9F0A"
        readonly property color statusError: "#FF453A"
        readonly property color statusDisconnected: "#8E8E93"
        readonly property string fontFamily: "Inter"
        readonly property int animHoverDuration: 1  // Reduced for tests
        readonly property int animContentFadeDuration: 1  // Reduced for tests
    }

    // =========================================================================
    // Mock ChatClient
    // =========================================================================
    
    QtObject {
        id: mockChatClient
        objectName: "mockChatClient"
        
        property bool connected: false
        property string connectionStatus: "disconnected"
        property string currentChannel: ""
        
        signal messageReceived(var message)
        signal errorOccurred(string error)
        
        function connectToChannel(channelName) {
            currentChannel = channelName
            connectionStatus = "connecting"
        }
        
        function disconnect() {
            connected = false
            connectionStatus = "disconnected"
            currentChannel = ""
        }
        
        function sendMessage(msg) {
            messageReceived({
                username: "testUser",
                displayName: "Test User",
                message: msg,
                color: "#FF0000",
                badges: []
            })
        }
        
        function simulateConnected() {
            connected = true
            connectionStatus = "connected"
        }
        
        function simulateError(errorMsg) {
            connectionStatus = "error"
            errorOccurred(errorMsg)
        }
        
        function simulateMessage(username, message) {
            messageReceived({
                username: username,
                displayName: username,
                message: message,
                color: "#9147FF",
                badges: []
            })
        }
        
        function reset() {
            connected = false
            connectionStatus = "disconnected"
            currentChannel = ""
        }
    }

    // =========================================================================
    // Mock ChatMessage Component
    // =========================================================================
    
    Component {
        id: chatMessageComponent
        Rectangle {
            property var messageData
            height: 30
            color: "transparent"
            
            Text {
                anchors.fill: parent
                anchors.margins: 4
                text: messageData ? (messageData.displayName + ": " + messageData.message) : ""
                color: blueTheme.primaryText
                font.pixelSize: 12
            }
        }
    }

    // =========================================================================
    // Component Under Test
    // =========================================================================
    
    Component {
        id: chatPanelComponent
        
        Rectangle {
            id: chatPanel
            objectName: "chatPanel"
            width: currentWidth
            height: 600
            
            property var chatClient: null
            property string channelName: ""
            property bool isConnected: chatClient ? chatClient.connected : false
            property string connectionStatus: chatClient ? chatClient.connectionStatus : "disconnected"
            property int maxMessages: 500
            property bool autoScroll: true
            
            signal closeRequested()
            
            color: blueTheme.surface
            border.color: blueTheme.divider
            border.width: 1
            
            property int minimumWidth: 250
            property int maximumWidth: 500
            property int currentWidth: 320
            
            // Resize handle
            MouseArea {
                id: resizeHandle
                objectName: "resizeHandle"
                width: 8
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                cursorShape: Qt.SplitHCursor
                z: 10
                hoverEnabled: true
                
                property real startX: 0
                property real startWidth: 0
                
                onPressed: function(mouse) {
                    startX = mapToGlobal(mouse.x, 0).x
                    startWidth = chatPanel.currentWidth
                }
                
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var currentX = mapToGlobal(mouse.x, 0).x
                        var delta = startX - currentX
                        var newWidth = Math.max(chatPanel.minimumWidth, 
                                       Math.min(chatPanel.maximumWidth, startWidth + delta))
                        chatPanel.currentWidth = newWidth
                    }
                }
                
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    
                    Rectangle {
                        id: resizeIndicator
                        objectName: "resizeIndicator"
                        anchors.centerIn: parent
                        width: 3
                        height: 40
                        radius: 1.5
                        color: resizeHandle.containsMouse || resizeHandle.pressed ? "#FFFFFF" : "#48484A"
                    }
                }
            }
            
            // Message model
            ListModel {
                id: messageModel
                objectName: "messageModel"
            }
            
            // Handle incoming messages
            Connections {
                target: chatPanel.chatClient
                enabled: chatPanel.chatClient !== null
                
                function onMessageReceived(message) {
                    messageModel.append(message)
                    while (messageModel.count > chatPanel.maxMessages) {
                        messageModel.remove(0)
                    }
                    if (chatPanel.autoScroll) {
                        messageListView.positionViewAtEnd()
                    }
                }
                
                function onConnectedChanged() {
                    if (chatPanel.chatClient && chatPanel.chatClient.connected) {
                        statusText.text = "Connected"
                    }
                }
                
                function onErrorOccurred(error) {
                    statusText.text = "Error: " + error
                }
            }
            
            onChannelNameChanged: {
                if (channelName && chatClient) {
                    messageModel.clear()
                    chatClient.connectToChannel(channelName)
                }
            }
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                
                // Header
                Rectangle {
                    id: header
                    objectName: "header"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    color: blueTheme.surfaceHeader
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 8
                        spacing: 8
                        
                        // Connection status indicator
                        Rectangle {
                            id: statusIndicator
                            objectName: "statusIndicator"
                            width: 8
                            height: 8
                            radius: 4
                            color: {
                                switch (chatPanel.connectionStatus) {
                                    case "connected": return blueTheme.statusConnected
                                    case "connecting": return blueTheme.statusConnecting
                                    case "error": return blueTheme.statusError
                                    default: return blueTheme.statusDisconnected
                                }
                            }
                        }
                        
                        // Channel name
                        Text {
                            id: channelNameText
                            objectName: "channelNameText"
                            Layout.fillWidth: true
                            text: chatPanel.channelName ? "#" + chatPanel.channelName : "Chat"
                            color: blueTheme.primaryText
                            font.pixelSize: 14
                            font.bold: true
                            font.family: blueTheme.fontFamily
                            elide: Text.ElideRight
                        }
                        
                        // Close button
                        Rectangle {
                            id: closeButton
                            objectName: "closeButton"
                            width: 28
                            height: 28
                            radius: 14
                            color: closeMouseArea.containsMouse ? "#48484A" : "transparent"
                            
                            Text {
                                anchors.centerIn: parent
                                text: "\u2715"
                                color: blueTheme.mutedText
                                font.pixelSize: 14
                            }
                            
                            MouseArea {
                                id: closeMouseArea
                                objectName: "closeMouseArea"
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: chatPanel.closeRequested()
                            }
                        }
                    }
                }
                
                // Separator
                Rectangle {
                    id: separator
                    objectName: "separator"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: blueTheme.divider
                }
                
                // Status bar
                Rectangle {
                    id: statusBar
                    objectName: "statusBar"
                    Layout.fillWidth: true
                    Layout.preferredHeight: visible ? 32 : 0
                    visible: chatPanel.connectionStatus !== "connected"
                    color: blueTheme.surfaceHeader
                    
                    Text {
                        id: statusText
                        objectName: "statusText"
                        anchors.centerIn: parent
                        text: {
                            switch (chatPanel.connectionStatus) {
                                case "connecting": return "Connecting..."
                                case "error": return "Connection error"
                                default: return "Disconnected"
                            }
                        }
                        color: blueTheme.mutedText
                        font.pixelSize: 12
                        font.family: blueTheme.fontFamily
                    }
                }
                
                // Message list
                ListView {
                    id: messageListView
                    objectName: "messageListView"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 4
                    model: messageModel
                    
                    delegate: chatMessageComponent
                    
                    // Empty state
                    Text {
                        id: emptyStateText
                        objectName: "emptyStateText"
                        anchors.centerIn: parent
                        visible: messageModel.count === 0 && chatPanel.connectionStatus === "connected"
                        text: "Waiting for messages..."
                        color: "#6E6E73"
                        font.pixelSize: 13
                        font.family: blueTheme.fontFamily
                    }
                    
                    // Scroll to bottom button
                    Rectangle {
                        id: scrollToBottomButton
                        objectName: "scrollToBottomButton"
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottomMargin: 8
                        width: 120
                        height: 28
                        radius: 14
                        color: "#CC2C2C2E"
                        border.color: "#48484A"
                        border.width: 1
                        visible: !chatPanel.autoScroll && messageModel.count > 0
                        
                        Text {
                            anchors.centerIn: parent
                            text: "\u2193 New messages"
                            color: blueTheme.primaryText
                            font.pixelSize: 11
                        }
                        
                        MouseArea {
                            id: scrollButtonMouseArea
                            objectName: "scrollButtonMouseArea"
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                chatPanel.autoScroll = true
                                messageListView.positionViewAtEnd()
                            }
                        }
                    }
                }
                
                // Message input area
                Rectangle {
                    id: inputArea
                    objectName: "inputArea"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: blueTheme.surfaceHeader
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8
                        
                        TextField {
                            id: messageInput
                            objectName: "messageInput"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            placeholderText: activeFocus ? "" : "Send a message..."
                            placeholderTextColor: "#6E6E73"
                            color: blueTheme.primaryText
                            font.pixelSize: 13
                            font.family: blueTheme.fontFamily
                            
                            background: Rectangle {
                                color: blueTheme.surface
                                radius: 8
                                border.color: messageInput.activeFocus ? blueTheme.accent : "#48484A"
                                border.width: 1
                            }
                            
                            onAccepted: chatPanel.sendCurrentMessage()
                            Keys.onEscapePressed: focus = false
                        }
                        
                        Rectangle {
                            id: sendButton
                            objectName: "sendButton"
                            Layout.preferredWidth: 34
                            Layout.preferredHeight: 34
                            radius: 8
                            color: sendMouseArea.containsMouse ? blueTheme.accent : "#6441A4"
                            opacity: messageInput.text.trim().length > 0 ? 1.0 : 0.5
                            
                            Text {
                                anchors.centerIn: parent
                                text: "\u27A4"
                                color: blueTheme.primaryText
                                font.pixelSize: 16
                            }
                            
                            MouseArea {
                                id: sendMouseArea
                                objectName: "sendMouseArea"
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: chatPanel.sendCurrentMessage()
                            }
                        }
                    }
                }
            }
            
            function sendCurrentMessage() {
                var msg = messageInput.text.trim()
                if (msg.length > 0 && chatClient) {
                    chatClient.sendMessage(msg)
                    messageInput.text = ""
                }
            }
            
            function scrollToBottom() {
                messageListView.positionViewAtEnd()
            }
            
            function clear() {
                messageModel.clear()
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var chatPanel: null

    // =========================================================================
    // Signal Spies (created dynamically in init to avoid offscreen issues)
    // =========================================================================
    
    property var closeRequestedSpy: null
    property var messageReceivedSpy: null
    
    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ChatPanelTests"
        when: windowShown

        function init() {
            chatPanel = createTemporaryObject(chatPanelComponent, root)
            verify(chatPanel !== null, "ChatPanel should be created")
            
            // Create spies dynamically to avoid offscreen mode issues
            closeRequestedSpy = createTemporaryObject(signalSpyComponent, root, {target: chatPanel, signalName: "closeRequested"})
            messageReceivedSpy = createTemporaryObject(signalSpyComponent, root, {target: mockChatClient, signalName: "messageReceived"})
            
            mockChatClient.reset()
            mouseMove(root, 1, 1)
            waitForRendering(chatPanel)
        }

        function cleanup() {
            chatPanel = null
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_channelNameEmpty() {
            compare(chatPanel.channelName, "", "Default channelName should be empty")
        }
        
        function test_defaultState_chatClientNull() {
            compare(chatPanel.chatClient, null, "Default chatClient should be null")
        }
        
        function test_defaultState_notConnected() {
            compare(chatPanel.isConnected, false, "Should not be connected by default")
        }
        
        function test_defaultState_connectionStatusDisconnected() {
            compare(chatPanel.connectionStatus, "disconnected", "Default status should be disconnected")
        }
        
        function test_defaultState_autoScrollEnabled() {
            compare(chatPanel.autoScroll, true, "autoScroll should be true by default")
        }
        
        function test_defaultState_maxMessages() {
            compare(chatPanel.maxMessages, 500, "Default maxMessages should be 500")
        }

        // =====================================================================
        // TEST: Header Display
        // =====================================================================
        
        function test_header_exists() {
            var header = findChild(chatPanel, "header")
            verify(header !== null, "Header should exist")
        }
        
        function test_header_defaultChannelName() {
            var nameText = findChild(chatPanel, "channelNameText")
            compare(nameText.text, "Chat", "Default header should show 'Chat'")
        }
        
        function test_header_channelNameWithHash() {
            // Arrange
            chatPanel.channelName = "streamer123"
            waitForRendering(chatPanel)
            
            // Assert
            var nameText = findChild(chatPanel, "channelNameText")
            compare(nameText.text, "#streamer123", "Channel name should be prefixed with #")
        }

        // =====================================================================
        // TEST: Connection Status
        // =====================================================================
        
        function test_statusIndicator_exists() {
            var indicator = findChild(chatPanel, "statusIndicator")
            verify(indicator !== null, "Status indicator should exist")
        }
        
        // =====================================================================
        // TEST: Status Bar Visibility
        // =====================================================================
        
        function test_statusBar_visibleWhenDisconnected() {
            var statusBar = findChild(chatPanel, "statusBar")
            verify(statusBar.visible, "Status bar should be visible when disconnected")
        }
        
        function test_statusBar_hiddenWhenConnected() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            mockChatClient.connectionStatus = "connected"
            waitForRendering(chatPanel)
            
            // Assert
            var statusBar = findChild(chatPanel, "statusBar")
            verify(!statusBar.visible, "Status bar should be hidden when connected")
        }
        
        function test_statusBar_showsDisconnectedText() {
            var statusText = findChild(chatPanel, "statusText")
            compare(statusText.text, "Disconnected", "Should show Disconnected text")
        }
        
        function test_statusBar_showsConnectingText() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            mockChatClient.connectionStatus = "connecting"
            waitForRendering(chatPanel)
            
            // Assert
            var statusText = findChild(chatPanel, "statusText")
            compare(statusText.text, "Connecting...", "Should show Connecting text")
        }

        // =====================================================================
        // TEST: Close Button
        // =====================================================================
        
        function test_closeButton_exists() {
            var closeBtn = findChild(chatPanel, "closeButton")
            verify(closeBtn !== null, "Close button should exist")
        }
        
        function test_closeButton_click_emitsSignal() {
            // Arrange
            var closeMouseArea = findChild(chatPanel, "closeMouseArea")
            verify(closeMouseArea !== null, "Close mouse area should exist")
            
            // Act
            mouseClick(closeMouseArea)
            
            // Assert
            compare(closeRequestedSpy.count, 1, "closeRequested should be emitted once")
        }
        
        function test_closeButton_multipleClicks() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var closeMouseArea = findChild(chatPanel, "closeMouseArea")
            
            // Act
            mouseClick(closeMouseArea)
            mouseClick(closeMouseArea)
            mouseClick(closeMouseArea)
            
            // Assert
            compare(closeRequestedSpy.count, 3, "Should emit signal for each click")
        }

        // =====================================================================
        // TEST: Message List
        // =====================================================================
        
        function test_messageList_exists() {
            var listView = findChild(chatPanel, "messageListView")
            verify(listView !== null, "Message list view should exist")
        }
        
        function test_messageList_emptyByDefault() {
            var listView = findChild(chatPanel, "messageListView")
            compare(listView.count, 0, "Message list should be empty by default")
        }
        
        function test_messageList_receivesMessages() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            waitForRendering(chatPanel)
            
            // Act
            mockChatClient.simulateMessage("testUser", "Hello world!")
            waitForRendering(chatPanel)
            
            // Assert
            var listView = findChild(chatPanel, "messageListView")
            tryCompare(listView, "count", 1, 100, "Message list should have 1 message")
        }
        
        function test_messageList_multipleMessages() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            waitForRendering(chatPanel)
            
            // Act
            mockChatClient.simulateMessage("user1", "Message 1")
            mockChatClient.simulateMessage("user2", "Message 2")
            mockChatClient.simulateMessage("user3", "Message 3")
            waitForRendering(chatPanel)
            
            // Assert
            var listView = findChild(chatPanel, "messageListView")
            tryCompare(listView, "count", 3, 100, "Message list should have 3 messages")
        }
        
        function test_messageList_respectsMaxMessages() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            chatPanel.maxMessages = 5
            waitForRendering(chatPanel)
            
            // Act - send more than maxMessages
            for (var i = 0; i < 8; i++) {
                mockChatClient.simulateMessage("user" + i, "Message " + i)
            }
            waitForRendering(chatPanel)
            
            // Assert
            var listView = findChild(chatPanel, "messageListView")
            tryCompare(listView, "count", 5, 100, "Message count should not exceed maxMessages")
        }
        
        function test_messageList_clearFunction() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            mockChatClient.simulateMessage("user", "Test message")
            waitForRendering(chatPanel)
            var listView = findChild(chatPanel, "messageListView")
            tryCompare(listView, "count", 1, 100, "Should have a message first")
            
            // Act
            chatPanel.clear()
            waitForRendering(chatPanel)
            
            // Assert
            compare(listView.count, 0, "clear() should remove all messages")
        }

        // =====================================================================
        // TEST: Empty State
        // =====================================================================
        
        function test_emptyState_hiddenWhenDisconnected() {
            var emptyText = findChild(chatPanel, "emptyStateText")
            verify(!emptyText.visible, "Empty state should be hidden when disconnected")
        }
        
        function test_emptyState_visibleWhenConnectedNoMessages() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            mockChatClient.connectionStatus = "connected"
            waitForRendering(chatPanel)
            
            // Assert
            var emptyText = findChild(chatPanel, "emptyStateText")
            verify(emptyText.visible, "Empty state should be visible when connected with no messages")
        }
        
        function test_emptyState_text() {
            var emptyText = findChild(chatPanel, "emptyStateText")
            compare(emptyText.text, "Waiting for messages...", "Should show waiting message")
        }

        // =====================================================================
        // TEST: Input Area
        // =====================================================================
        
        function test_inputArea_exists() {
            var inputArea = findChild(chatPanel, "inputArea")
            verify(inputArea !== null, "Input area should exist")
        }
        
        function test_messageInput_exists() {
            var input = findChild(chatPanel, "messageInput")
            verify(input !== null, "Message input should exist")
        }
        
        function test_messageInput_placeholder() {
            var input = findChild(chatPanel, "messageInput")
            compare(input.placeholderText, "Send a message...", "Should show placeholder text")
        }
        
        function test_sendButton_exists() {
            var sendBtn = findChild(chatPanel, "sendButton")
            verify(sendBtn !== null, "Send button should exist")
        }
        
        function test_sendButton_lowOpacityWhenEmpty() {
            var sendBtn = findChild(chatPanel, "sendButton")
            var input = findChild(chatPanel, "messageInput")
            input.text = ""
            waitForRendering(chatPanel)
            
            compare(sendBtn.opacity, 0.5, "Send button should have low opacity when input empty")
        }
        
        function test_sendButton_fullOpacityWithText() {
            var sendBtn = findChild(chatPanel, "sendButton")
            var input = findChild(chatPanel, "messageInput")
            input.text = "Hello"
            waitForRendering(chatPanel)
            
            compare(sendBtn.opacity, 1.0, "Send button should have full opacity with text")
        }
        
        function test_sendButton_click_sendsMessage() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            var input = findChild(chatPanel, "messageInput")
            var sendMouseArea = findChild(chatPanel, "sendMouseArea")
            input.text = "Test message"
            waitForRendering(chatPanel)
            
            // Act
            mouseClick(sendMouseArea)
            waitForRendering(chatPanel)
            
            // Assert
            compare(input.text, "", "Input should be cleared after sending")
            compare(messageReceivedSpy.count, 1, "Message should be sent")
        }
        
        function test_sendButton_noSendWhenEmpty() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            var input = findChild(chatPanel, "messageInput")
            var sendMouseArea = findChild(chatPanel, "sendMouseArea")
            input.text = ""
            waitForRendering(chatPanel)
            
            // Act
            mouseClick(sendMouseArea)
            waitForRendering(chatPanel)
            
            // Assert
            compare(messageReceivedSpy.count, 0, "Should not send empty message")
        }

        // =====================================================================
        // TEST: Resize Handle
        // =====================================================================
        
        function test_resizeHandle_exists() {
            var handle = findChild(chatPanel, "resizeHandle")
            verify(handle !== null, "Resize handle should exist")
        }
        
        function test_resizeHandle_cursorShape() {
            var handle = findChild(chatPanel, "resizeHandle")
            compare(handle.cursorShape, Qt.SplitHCursor, "Should show horizontal split cursor")
        }

        // =====================================================================
        // TEST: Channel Connection
        // =====================================================================
        
        function test_channelChange_connectsToChannel() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            waitForRendering(chatPanel)
            
            // Act
            chatPanel.channelName = "newchannel"
            waitForRendering(chatPanel)
            
            // Assert
            compare(mockChatClient.currentChannel, "newchannel", "Should connect to new channel")
            compare(mockChatClient.connectionStatus, "connecting", "Status should be connecting")
        }
        
        function test_channelChange_clearsMessages() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            mockChatClient.simulateMessage("user", "Old message")
            waitForRendering(chatPanel)
            var listView = findChild(chatPanel, "messageListView")
            tryCompare(listView, "count", 1, 100, "Should have a message first")
            
            // Act
            chatPanel.channelName = "newchannel"
            waitForRendering(chatPanel)
            
            // Assert
            compare(listView.count, 0, "Messages should be cleared on channel change")
        }

        // =====================================================================
        // TEST: Scroll to Bottom Button
        // =====================================================================
        
        function test_scrollToBottomButton_hiddenByDefault() {
            var btn = findChild(chatPanel, "scrollToBottomButton")
            verify(!btn.visible, "Scroll button should be hidden by default (autoScroll=true)")
        }
        
        function test_scrollToBottomButton_hiddenWhenNoMessages() {
            // Arrange
            chatPanel.autoScroll = false
            waitForRendering(chatPanel)
            
            // Assert
            var btn = findChild(chatPanel, "scrollToBottomButton")
            verify(!btn.visible, "Scroll button should be hidden when no messages")
        }
        
        function test_scrollToBottomButton_visibleConditions() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            chatPanel.autoScroll = false
            mockChatClient.simulateMessage("user", "Test message")
            waitForRendering(chatPanel)
            
            // Assert
            var btn = findChild(chatPanel, "scrollToBottomButton")
            verify(btn.visible, "Scroll button should be visible when autoScroll=false and has messages")
        }
        
        function test_scrollToBottomButton_click_enablesAutoScroll() {
            // Arrange
            chatPanel.chatClient = mockChatClient
            chatPanel.autoScroll = false
            mockChatClient.simulateMessage("user", "Test message")
            waitForRendering(chatPanel)
            
            // Act
            var scrollMouseArea = findChild(chatPanel, "scrollButtonMouseArea")
            mouseClick(scrollMouseArea)
            waitForRendering(chatPanel)
            
            // Assert
            compare(chatPanel.autoScroll, true, "autoScroll should be re-enabled")
        }
    }
}
