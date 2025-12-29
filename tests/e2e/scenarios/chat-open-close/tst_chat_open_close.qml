import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Chat Open/Close (B.1)
 * 
 * Tests the chat panel in player view:
 * - Open/close chat panel
 * - Receive messages from MockIrcServer
 * - Send messages
 * 
 * Context: AuthenticatedSetup
 */
E2EScenarioTemplate {
    id: root

    E2ETestCase {
        id: testCase
        name: "E2E_ChatOpenClose"
        when: windowShown && root.appReady

        // Track received message count for verification
        property int initialMessageCount: 0

        // Uses default initTestCase() and cleanupTestCase() from E2ETestCase base

        function test_01_navigate_to_player() {
            console.log("Testing: Navigate to player")
            
            // Wait for home to fully initialize (API calls, rendering, etc.)
            wait(E2EConstants.timeoutMedium)
            
            var streamCard = findChildByPrefix(mainWindow, E2EConstants.streamCardPrefix)
            verify(streamCard !== null, "Stream card should exist")
            
            console.log("  Clicking stream card: " + streamCard.objectName)
            
            // Use clickAndWait helper with navigation condition
            var navigated = clickAndWait(streamCard, function() {
                return mainWindow.currentView === "player"
            }, E2EConstants.timeoutLong)
            
            verify(navigated, "Should navigate to player view")
            console.log("OK Player view opened")
            takeScreenshot()
        }

        function test_02_chat_button_exists() {
            console.log("Testing: Chat toggle button exists")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            // Wait for controls to be ready
            tryVerify(function() {
                var btn = findChild(mainWindow, E2EConstants.chatToggleButton)
                return btn !== null && btn.visible
            }, 3000, "Chat toggle button should be visible")
            
            var chatButton = findChild(mainWindow, E2EConstants.chatToggleButton)
            verify(chatButton !== null, "Chat toggle button should exist")
            verify(chatButton.visible, "Chat toggle button should be visible")
            
            console.log("OK Chat toggle button found")
            takeScreenshot()
        }

        function test_03_open_chat_panel() {
            console.log("Testing: Open chat panel")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var chatButton = findChild(mainWindow, E2EConstants.chatToggleButton)
            verify(chatButton !== null, "Chat toggle button should exist")
            
            // Get player view to check chatVisible state
            var playerView = findChild(mainWindow, "playerView")
            verify(playerView !== null, "Player view should exist")
            
            console.log("  Initial chatVisible:", playerView.chatVisible)
            
            // Click to open chat
            mouseClick(chatButton)
            
            // Wait for chat panel to appear
            tryVerify(function() {
                return playerView.chatVisible === true
            }, 2000, "Chat should become visible")
            
            // Verify chat panel element exists
            tryVerify(function() {
                var panel = findChild(mainWindow, E2EConstants.chatPanel)
                return panel !== null && panel.visible
            }, 2000, "Chat panel should be visible")
            
            console.log("OK Chat panel opened")
            takeScreenshot()
        }

        function test_04_chat_connects() {
            console.log("Testing: Chat connects to MockIrcServer")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, "playerView")
            if (!playerView || !playerView.chatVisible) {
                skip("Chat is not open")
                return
            }
            
            // Wait for MockIrcServer to receive connection
            tryVerify(function() {
                return mockIrcServer.hasConnectedClient()
            }, 5000, "MockIrcServer should have a connected client")
            
            var joinedChannel = mockIrcServer.joinedChannel()
            console.log("  Client joined channel:", joinedChannel)
            verify(joinedChannel.length > 0, "Should have joined a channel")
            
            console.log("OK Chat connected to MockIrcServer")
            takeScreenshot()
        }

        function test_05_receive_chat_messages() {
            console.log("Testing: Receive chat messages")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            if (!mockIrcServer.hasConnectedClient()) {
                skip("Chat not connected")
                return
            }
            
            var messageList = findChild(mainWindow, E2EConstants.chatMessageList)
            verify(messageList !== null, "Message list should exist")
            
            initialMessageCount = messageList.count
            console.log("  Initial message count:", initialMessageCount)
            
            // Send test messages from MockIrcServer
            console.log("  Sending test messages from MockIrcServer...")
            mockIrcServer.sendChatMessage("TestUser1", "Hello from E2E test!")
            mockIrcServer.sendChatMessage("Moderator", "Welcome to the chat", "#00FF00", "moderator/1")
            mockIrcServer.sendChatMessage("Subscriber", "Great stream!", "#FF6B6B", "subscriber/12")
            
            // Wait for messages to appear in the list
            tryVerify(function() {
                return messageList.count >= initialMessageCount + 3
            }, 3000, "Should receive 3 new messages")
            
            console.log("  Final message count:", messageList.count)
            console.log("OK Received", messageList.count - initialMessageCount, "messages")
            takeScreenshot()
        }

        function test_06_send_chat_message() {
            console.log("Testing: Send chat message")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            if (!mockIrcServer.hasConnectedClient()) {
                skip("Chat not connected")
                return
            }
            
            var messageInput = findChild(mainWindow, E2EConstants.chatMessageInput)
            verify(messageInput !== null, "Message input should exist")
            
            var messageList = findChild(mainWindow, E2EConstants.chatMessageList)
            verify(messageList !== null, "Message list should exist")
            
            // Clear any previous client messages
            mockIrcServer.clearClientMessages()
            
            // Enable echo so the message appears in UI (like real Twitch IRC)
            mockIrcServer.setEchoMessages(true, "E2ETestUser", "#FF6B6B")
            
            // Record initial message count
            var initialCount = messageList.count
            console.log("  Initial message count:", initialCount)
            
            // In offscreen mode, focus doesn't work properly
            // So we set the text programmatically
            var testMessage = "Hello from E2E test sender!"
            messageInput.text = testMessage
            
            console.log("  Set message:", messageInput.text)
            verify(messageInput.text === testMessage, "Input should contain the message")
            
            // Trigger send via accepted signal (simulates pressing Enter)
            messageInput.accepted()
            
            // Wait for message to be sent to MockIrcServer
            tryVerify(function() {
                return mockIrcServer.clientMessageCount() > 0
            }, 2000, "MockIrcServer should receive the message")
            
            var sentMessages = mockIrcServer.clientMessages()
            console.log("  Messages received by server:", sentMessages.length)
            verify(sentMessages.length > 0, "At least one message should be sent")
            verify(sentMessages[0] === testMessage, "Sent message should match")
            
            // Input should be cleared after sending
            verify(messageInput.text === "", "Input should be cleared after sending")
            
            // Wait for echoed message to appear in UI
            tryVerify(function() {
                return messageList.count > initialCount
            }, 2000, "Message should appear in chat UI")
            
            console.log("  Final message count:", messageList.count)
            verify(messageList.count > initialCount, "Message should be visible in chat list")
            
            console.log("OK Message sent and visible in UI")
            takeScreenshot()
        }

        function test_07_close_chat_panel() {
            console.log("Testing: Close chat panel")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, "playerView")
            verify(playerView !== null, "Player view should exist")
            
            if (!playerView.chatVisible) {
                skip("Chat is already closed")
                return
            }
            
            var chatButton = findChild(mainWindow, E2EConstants.chatToggleButton)
            verify(chatButton !== null, "Chat toggle button should exist")
            
            // Click to close chat
            mouseClick(chatButton)
            
            // Wait for chat to close
            tryVerify(function() {
                return playerView.chatVisible === false
            }, 2000, "Chat should become hidden")
            
            console.log("OK Chat panel closed")
            takeScreenshot()
        }

        function test_08_reopen_chat_reconnects() {
            console.log("Testing: Reopen chat reconnects")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var chatButton = findChild(mainWindow, E2EConstants.chatToggleButton)
            var playerView = findChild(mainWindow, "playerView")
            
            verify(chatButton !== null, "Chat toggle button should exist")
            verify(playerView !== null, "Player view should exist")
            
            // Reopen chat
            mouseClick(chatButton)
            
            tryVerify(function() {
                return playerView.chatVisible === true
            }, 2000, "Chat should reopen")
            
            // Wait for reconnection
            tryVerify(function() {
                return mockIrcServer.hasConnectedClient()
            }, 5000, "Should reconnect to MockIrcServer")
            
            console.log("OK Chat reconnected successfully")
            takeScreenshot()
        }
    }
}
