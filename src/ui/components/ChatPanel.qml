import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../themes/BlueTheme.js" as BlueTheme

/**
 * ChatPanel - Twitch chat panel with message list
 */
Rectangle {
    id: chatPanel

    property var chatClient: null
    property string channelName: ""
    property bool isConnected: chatClient ? chatClient.connected : false
    property string connectionStatus: chatClient ? chatClient.connectionStatus : "disconnected"
    property int maxMessages: 500
    property bool autoScroll: true

    signal closeRequested()

    color: "#1C1C1E"
    border.color: "#3A3A3C"
    border.width: 1

    // Minimum and maximum width
    property int minimumWidth: 250
    property int maximumWidth: 500
    property int currentWidth: 320

    // Resize handle on the left edge
    MouseArea {
        id: resizeHandle
        width: 8
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        cursorShape: Qt.SplitHCursor
        z: 10  // Above other content
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
                var newWidth = Math.max(minimumWidth, Math.min(maximumWidth, startWidth + delta))
                chatPanel.currentWidth = newWidth
            }
        }

        // Visual feedback - only the center indicator changes color, not the background
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            
            Rectangle {
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
    }

    // Handle incoming messages
    Connections {
        target: chatClient
        enabled: chatClient !== null

        function onMessageReceived(message) {
            // Add message to model
            messageModel.append(message)

            // Remove old messages if over limit
            while (messageModel.count > maxMessages) {
                messageModel.remove(0)
            }

            // Auto-scroll to bottom if enabled
            if (autoScroll) {
                scrollToBottom()
            }
        }

        function onConnectedChanged() {
            if (chatClient && chatClient.connected) {
                statusText.text = qsTr("Connected")
            }
        }

        function onErrorOccurred(error) {
            statusText.text = qsTr("Error: %1").arg(error)
        }
    }

    // Connect when channel changes
    onChannelNameChanged: {
        if (channelName && chatClient) {
            messageModel.clear()
            chatClient.connectToChannel(channelName)
        }
    }

    Component.onDestruction: {
        if (chatClient) {
            chatClient.disconnect()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            color: "#2C2C2E"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 8
                spacing: 8

                // Connection status indicator
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: {
                        switch (connectionStatus) {
                            case "connected": return "#30D158"  // Green
                            case "connecting": return "#FF9F0A" // Orange
                            case "error": return "#FF453A"      // Red
                            default: return "#8E8E93"           // Gray
                        }
                    }

                    SequentialAnimation on opacity {
                        running: connectionStatus === "connecting"
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 500 }
                        NumberAnimation { to: 1.0; duration: 500 }
                    }
                }

                // Channel name
                Text {
                    Layout.fillWidth: true
                    text: channelName ? "#" + channelName : qsTr("Chat")
                    color: "#FFFFFF"
                    font.pixelSize: 14
                    font.bold: true
                    font.family: BlueTheme.fontFamily
                    elide: Text.ElideRight
                }

                // Close button
                Rectangle {
                    width: 28
                    height: 28
                    radius: 14
                    color: closeMouseArea.containsMouse ? "#48484A" : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "\u2715"
                        color: "#8E8E93"
                        font.pixelSize: 14
                    }

                    MouseArea {
                        id: closeMouseArea
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
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#3A3A3C"
        }

        // Status bar (when not connected)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: visible ? 32 : 0
            visible: connectionStatus !== "connected"
            color: "#2C2C2E"

            Text {
                id: statusText
                anchors.centerIn: parent
                text: {
                    switch (connectionStatus) {
                        case "connecting": return qsTr("Connecting...")
                        case "error": return qsTr("Connection error")
                        default: return qsTr("Disconnected")
                    }
                }
                color: "#8E8E93"
                font.pixelSize: 12
                font.family: BlueTheme.fontFamily
            }

            Behavior on Layout.preferredHeight {
                NumberAnimation { duration: 200 }
            }
        }

        // Message list
        ListView {
            id: messageListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: messageModel

            // Virtualization - only render visible items
            cacheBuffer: 200

            // Click to remove focus from input
            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                onPressed: function(mouse) {
                    messageInput.focus = false
                    mouse.accepted = false
                }
            }

            // Detect when user scrolls up
            onContentYChanged: {
                if (!atYEnd && moving) {
                    autoScroll = false
                }
            }

            // Resume auto-scroll when at bottom
            onAtYEndChanged: {
                if (atYEnd) {
                    autoScroll = true
                }
            }

            delegate: ChatMessage {
                width: messageListView.width
                messageData: model
            }

            // Scroll to bottom button
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 8
                width: scrollButtonRow.width + 16
                height: 28
                radius: 14
                color: "#CC2C2C2E"
                border.color: "#48484A"
                border.width: 1
                visible: !autoScroll && messageModel.count > 0
                opacity: visible ? 1.0 : 0.0

                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }

                Row {
                    id: scrollButtonRow
                    anchors.centerIn: parent
                    spacing: 4

                    Text {
                        text: "\u2193"
                        color: "#FFFFFF"
                        font.pixelSize: 12
                    }
                    Text {
                        text: qsTr("New messages")
                        color: "#FFFFFF"
                        font.pixelSize: 11
                        font.family: BlueTheme.fontFamily
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        autoScroll = true
                        scrollToBottom()
                    }
                }
            }

            // Empty state
            Text {
                anchors.centerIn: parent
                visible: messageModel.count === 0 && connectionStatus === "connected"
                text: qsTr("Waiting for messages...")
                color: "#6E6E73"
                font.pixelSize: 13
                font.family: BlueTheme.fontFamily
            }
        }

        // Message input area - always visible when connected
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            color: "#2C2C2E"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                TextField {
                    id: messageInput
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    placeholderText: activeFocus ? "" : qsTr("Send a message...")
                    placeholderTextColor: "#6E6E73"
                    color: "#FFFFFF"
                    font.pixelSize: 13
                    font.family: BlueTheme.fontFamily
                    
                    background: Rectangle {
                        color: "#1C1C1E"
                        radius: 8
                        border.color: messageInput.activeFocus ? "#9147FF" : "#48484A"
                        border.width: 1
                    }

                    onAccepted: {
                        sendCurrentMessage()
                    }

                    Keys.onEscapePressed: {
                        focus = false
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    radius: 8
                    color: sendMouseArea.containsMouse ? "#9147FF" : "#6441A4"
                    opacity: messageInput.text.trim().length > 0 ? 1.0 : 0.5

                    Text {
                        anchors.centerIn: parent
                        text: "\u27A4"  // Arrow
                        color: "#FFFFFF"
                        font.pixelSize: 16
                    }

                    MouseArea {
                        id: sendMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: sendCurrentMessage()
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

    // Helper function
    function scrollToBottom() {
        messageListView.positionViewAtEnd()
    }

    function clear() {
        messageModel.clear()
    }
}
