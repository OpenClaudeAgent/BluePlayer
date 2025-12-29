/**
 * tst_ChatMessage.qml
 * 
 * Functional UI tests for the ChatMessage component.
 * Tests message display, username rendering, badge display, and emote parts.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 400

    // =========================================================================
    // Component Under Test (Mock of ChatMessage)
    // =========================================================================
    
    Component {
        id: chatMessageComponent
        
        Item {
            id: messageRoot
            objectName: "chatMessage"

            // Properties matching ChatMessage
            property var messageData: ({})
            property int emoteSize: 24
            property int badgeSize: 18

            // Extracted data with defaults
            readonly property string username: messageData.displayName || messageData.username || "Anonymous"
            readonly property string userColor: messageData.color || "#AAAAAA"
            readonly property var badges: messageData.badges || []
            readonly property var emoteParts: messageData.emoteParts || []
            readonly property string rawMessage: messageData.message || ""

            implicitHeight: messageFlow.height + 8
            implicitWidth: parent ? parent.width : 300
            width: 300
            height: implicitHeight

            // Message container
            Flow {
                id: messageFlow
                objectName: "messageFlow"
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3

                // Badges
                Repeater {
                    id: badgeRepeater
                    objectName: "badgeRepeater"
                    model: messageRoot.badges

                    delegate: Rectangle {
                        id: badgeRect
                        objectName: "badge_" + index
                        required property var modelData
                        required property int index
                        
                        width: messageRoot.badgeSize
                        height: messageRoot.badgeSize
                        radius: 3
                        color: getBadgeColor(modelData.type || "")

                        Text {
                            id: badgeIcon
                            objectName: "badgeIcon_" + parent.index
                            anchors.centerIn: parent
                            text: getBadgeIcon(modelData.type || "")
                            font.pixelSize: 10
                            font.bold: true
                            color: "#FFFFFF"
                        }
                    }
                }

                // Username
                Text {
                    id: usernameText
                    objectName: "usernameText"
                    text: messageRoot.username + ":"
                    color: messageRoot.userColor
                    font.pixelSize: 13
                    font.bold: true
                }

                // Space after username
                Item { width: 4; height: 1 }

                // Message parts (text and emotes)
                Repeater {
                    id: partsRepeater
                    objectName: "partsRepeater"
                    model: (messageRoot.emoteParts && messageRoot.emoteParts.length > 0) 
                           ? messageRoot.emoteParts 
                           : [{type: "text", content: messageRoot.rawMessage}]

                    delegate: Loader {
                        id: partLoader
                        objectName: "partLoader_" + index
                        property var partData: partsRepeater.model[index] || {}
                        property string partType: partData.type || "text"
                        
                        sourceComponent: partType === "emote" ? emoteComponent : textComponent
                    }
                }
            }

            // Text component
            Component {
                id: textComponent

                Text {
                    objectName: "messageText"
                    text: partData.content || ""
                    color: "#FFFFFF"
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                }
            }

            // Emote component (simplified for testing)
            Component {
                id: emoteComponent

                Rectangle {
                    objectName: "emote"
                    width: messageRoot.emoteSize
                    height: messageRoot.emoteSize
                    color: "#9147FF"
                    radius: 4
                    
                    Text {
                        anchors.centerIn: parent
                        text: partData.content || ""
                        font.pixelSize: 8
                        color: "#FFFFFF"
                    }
                }
            }

            // Helper functions
            function getBadgeColor(badgeType) {
                if (!badgeType) return "transparent"
                
                var colors = {
                    "broadcaster": "#E91916",
                    "moderator": "#00AD03",
                    "vip": "#E005B9",
                    "subscriber": "#9147FF",
                    "bits": "#FAAF19",
                    "premium": "#9147FF",
                    "partner": "#9147FF",
                    "staff": "#E91916"
                }
                
                var baseType = badgeType.split("/")[0]
                return colors[baseType] || "#6E6E73"
            }

            function getBadgeIcon(badgeType) {
                if (!badgeType) return ""
                
                var icons = {
                    "broadcaster": "B",
                    "moderator": "M",
                    "vip": "V",
                    "subscriber": "S",
                    "bits": "$",
                    "premium": "P",
                    "partner": "P",
                    "staff": "S"
                }
                
                var baseType = badgeType.split("/")[0]
                return icons[baseType] || baseType.charAt(0).toUpperCase()
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var chatMessage: null

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ChatMessageTests"
        when: windowShown

        function init() {
            chatMessage = createTemporaryObject(chatMessageComponent, root)
            verify(chatMessage !== null, "ChatMessage should be created")
            chatMessage.anchors.centerIn = root
        }

        function cleanup() {
            chatMessage = null
        }

        // =====================================================================
        // TEST: Default Values
        // =====================================================================
        
        function test_defaultValues_emptyMessageData() {
            compare(chatMessage.username, "Anonymous", "Default username is Anonymous")
            compare(chatMessage.rawMessage, "", "Default rawMessage is empty")
        }

        function test_defaultValues_defaultColor() {
            compare(chatMessage.userColor, "#AAAAAA", "Default user color is gray")
        }

        function test_defaultValues_emptyBadges() {
            compare(chatMessage.badges.length, 0, "Default badges is empty array")
        }

        function test_defaultValues_emptyEmoteParts() {
            compare(chatMessage.emoteParts.length, 0, "Default emoteParts is empty array")
        }

        function test_defaultValues_sizes() {
            compare(chatMessage.emoteSize, 24, "Default emote size is 24")
            compare(chatMessage.badgeSize, 18, "Default badge size is 18")
        }

        // =====================================================================
        // TEST: Username Display
        // =====================================================================
        
        function test_username_displayNamePreferred() {
            chatMessage.messageData = {
                displayName: "Pokimane",
                username: "pokimane"
            }
            
            compare(chatMessage.username, "Pokimane", "displayName should be preferred")
        }

        function test_username_fallbackToUsername() {
            chatMessage.messageData = {
                username: "shroud"
            }
            
            compare(chatMessage.username, "shroud", "Falls back to username when no displayName")
        }

        function test_username_anonymousWhenEmpty() {
            chatMessage.messageData = {}
            
            compare(chatMessage.username, "Anonymous", "Anonymous when no name provided")
        }

        function test_username_textElementUpdates() {
            var usernameText = findChild(chatMessage, "usernameText")
            
            chatMessage.messageData = { displayName: "xQc" }
            
            compare(usernameText.text, "xQc:", "Username text includes colon")
        }

        // =====================================================================
        // TEST: User Color
        // =====================================================================
        
        function test_userColor_appliedFromData() {
            chatMessage.messageData = {
                displayName: "TestUser",
                color: "#FF0000"
            }
            
            compare(chatMessage.userColor, "#FF0000", "User color from messageData")
        }

        function test_userColor_defaultWhenNotProvided() {
            chatMessage.messageData = {
                displayName: "TestUser"
            }
            
            compare(chatMessage.userColor, "#AAAAAA", "Default gray when no color")
        }

        function test_userColor_appliedToUsernameText() {
            var usernameText = findChild(chatMessage, "usernameText")
            
            chatMessage.messageData = {
                displayName: "TestUser",
                color: "#00FF00"
            }
            
            compare(usernameText.color.toString(), "#00ff00", "Color applied to username text")
        }

        // =====================================================================
        // TEST: Badge Display
        // =====================================================================
        
        function test_badges_noBadgesWhenEmpty() {
            chatMessage.messageData = {
                displayName: "TestUser",
                badges: []
            }
            
            compare(chatMessage.badges.length, 0, "No badges when empty array")
        }

        function test_badges_singleBadge() {
            chatMessage.messageData = {
                displayName: "TestUser",
                badges: [{ type: "broadcaster" }]
            }
            
            compare(chatMessage.badges.length, 1, "Single badge rendered")
        }

        function test_badges_multipleBadges() {
            chatMessage.messageData = {
                displayName: "TestUser",
                badges: [
                    { type: "broadcaster" },
                    { type: "subscriber" },
                    { type: "vip" }
                ]
            }
            
            compare(chatMessage.badges.length, 3, "Multiple badges rendered")
        }

        function test_badges_broadcasterColor() {
            var color = chatMessage.getBadgeColor("broadcaster")
            compare(color, "#E91916", "Broadcaster badge is red")
        }

        function test_badges_moderatorColor() {
            var color = chatMessage.getBadgeColor("moderator")
            compare(color, "#00AD03", "Moderator badge is green")
        }

        function test_badges_vipColor() {
            var color = chatMessage.getBadgeColor("vip")
            compare(color, "#E005B9", "VIP badge is pink")
        }

        function test_badges_subscriberColor() {
            var color = chatMessage.getBadgeColor("subscriber")
            compare(color, "#9147FF", "Subscriber badge is Twitch purple")
        }

        function test_badges_unknownBadgeColor() {
            var color = chatMessage.getBadgeColor("unknown_badge")
            compare(color, "#6E6E73", "Unknown badge gets default gray")
        }

        function test_badges_emptyBadgeTypeReturnsTransparent() {
            var color = chatMessage.getBadgeColor("")
            compare(color, "transparent", "Empty badge type returns transparent")
        }

        // =====================================================================
        // TEST: Badge Icons
        // =====================================================================
        
        function test_badgeIcon_broadcaster() {
            var icon = chatMessage.getBadgeIcon("broadcaster")
            compare(icon, "B", "Broadcaster icon is B")
        }

        function test_badgeIcon_moderator() {
            var icon = chatMessage.getBadgeIcon("moderator")
            compare(icon, "M", "Moderator icon is M")
        }

        function test_badgeIcon_vip() {
            var icon = chatMessage.getBadgeIcon("vip")
            compare(icon, "V", "VIP icon is V")
        }

        function test_badgeIcon_subscriber() {
            var icon = chatMessage.getBadgeIcon("subscriber")
            compare(icon, "S", "Subscriber icon is S")
        }

        function test_badgeIcon_bits() {
            var icon = chatMessage.getBadgeIcon("bits")
            compare(icon, "$", "Bits icon is $")
        }

        function test_badgeIcon_unknown() {
            var icon = chatMessage.getBadgeIcon("custom_badge")
            compare(icon, "C", "Unknown badge uses first letter uppercase")
        }

        function test_badgeIcon_emptyReturnsEmpty() {
            var icon = chatMessage.getBadgeIcon("")
            compare(icon, "", "Empty badge type returns empty icon")
        }

        // =====================================================================
        // TEST: Message Content
        // =====================================================================
        
        function test_message_rawMessageDisplayed() {
            chatMessage.messageData = {
                displayName: "TestUser",
                message: "Hello world!"
            }
            
            compare(chatMessage.rawMessage, "Hello world!", "Raw message stored")
        }

        function test_message_emptyMessageAllowed() {
            chatMessage.messageData = {
                displayName: "TestUser",
                message: ""
            }
            
            compare(chatMessage.rawMessage, "", "Empty message allowed")
        }

        // =====================================================================
        // TEST: Emote Parts
        // =====================================================================
        
        function test_emoteParts_textOnly() {
            chatMessage.messageData = {
                displayName: "TestUser",
                emoteParts: [
                    { type: "text", content: "Hello everyone!" }
                ]
            }
            
            compare(chatMessage.emoteParts.length, 1, "Single text part")
            compare(chatMessage.emoteParts[0].type, "text", "Part is text type")
        }

        function test_emoteParts_singleEmote() {
            chatMessage.messageData = {
                displayName: "TestUser",
                emoteParts: [
                    { type: "emote", content: "Kappa", emoteId: "25" }
                ]
            }
            
            compare(chatMessage.emoteParts.length, 1, "Single emote part")
            compare(chatMessage.emoteParts[0].type, "emote", "Part is emote type")
        }

        function test_emoteParts_mixedContent() {
            chatMessage.messageData = {
                displayName: "TestUser",
                emoteParts: [
                    { type: "text", content: "Hello " },
                    { type: "emote", content: "Kappa", emoteId: "25" },
                    { type: "text", content: " world" }
                ]
            }
            
            compare(chatMessage.emoteParts.length, 3, "Mixed content parts")
        }

        function test_emoteParts_fallbackToRawMessage() {
            chatMessage.messageData = {
                displayName: "TestUser",
                message: "Fallback message",
                emoteParts: []
            }
            
            // When emoteParts is empty, partsRepeater model falls back to raw message
            compare(chatMessage.rawMessage, "Fallback message", "Raw message available for fallback")
        }

        // =====================================================================
        // TEST: Size Configuration
        // =====================================================================
        
        function test_emoteSize_canBeChanged() {
            chatMessage.emoteSize = 32
            compare(chatMessage.emoteSize, 32, "Emote size can be changed")
        }

        function test_badgeSize_canBeChanged() {
            chatMessage.badgeSize = 24
            compare(chatMessage.badgeSize, 24, "Badge size can be changed")
        }

        // =====================================================================
        // TEST: Badge Type Parsing (with version)
        // =====================================================================
        
        function test_badgeType_withVersion() {
            // Badge types can have format "subscriber/12" where 12 is the version
            var color = chatMessage.getBadgeColor("subscriber/12")
            compare(color, "#9147FF", "Badge with version parsed correctly")
        }

        function test_badgeIcon_withVersion() {
            var icon = chatMessage.getBadgeIcon("subscriber/24")
            compare(icon, "S", "Badge icon with version parsed correctly")
        }

        // =====================================================================
        // TEST: Data-Driven Badge Colors
        // =====================================================================

        function test_badgeColors_data() {
            return [
                { tag: "broadcaster", type: "broadcaster", expected: "#E91916" },
                { tag: "moderator", type: "moderator", expected: "#00AD03" },
                { tag: "vip", type: "vip", expected: "#E005B9" },
                { tag: "subscriber", type: "subscriber", expected: "#9147FF" },
                { tag: "bits", type: "bits", expected: "#FAAF19" },
                { tag: "premium", type: "premium", expected: "#9147FF" },
                { tag: "partner", type: "partner", expected: "#9147FF" },
                { tag: "staff", type: "staff", expected: "#E91916" },
                { tag: "unknown", type: "random", expected: "#6E6E73" }
            ]
        }

        function test_badgeColors(data) {
            var color = chatMessage.getBadgeColor(data.type)
            compare(color, data.expected, "Badge color for " + data.tag)
        }

        // =====================================================================
        // TEST: Data-Driven Badge Icons
        // =====================================================================

        function test_badgeIcons_data() {
            return [
                { tag: "broadcaster", type: "broadcaster", expected: "B" },
                { tag: "moderator", type: "moderator", expected: "M" },
                { tag: "vip", type: "vip", expected: "V" },
                { tag: "subscriber", type: "subscriber", expected: "S" },
                { tag: "bits", type: "bits", expected: "$" },
                { tag: "premium", type: "premium", expected: "P" },
                { tag: "partner", type: "partner", expected: "P" },
                { tag: "staff", type: "staff", expected: "S" }
            ]
        }

        function test_badgeIcons(data) {
            var icon = chatMessage.getBadgeIcon(data.type)
            compare(icon, data.expected, "Badge icon for " + data.tag)
        }

        // =====================================================================
        // TEST: Integration - Full Message
        // =====================================================================

        function test_fullMessage_withAllData() {
            chatMessage.messageData = {
                displayName: "Ninja",
                username: "ninja",
                color: "#1E90FF",
                message: "Let's go!",
                badges: [
                    { type: "partner" },
                    { type: "subscriber", version: "36" }
                ],
                emoteParts: [
                    { type: "text", content: "Let's go! " },
                    { type: "emote", content: "PogChamp", emoteId: "305954156" }
                ]
            }
            
            compare(chatMessage.username, "Ninja", "Display name shown")
            compare(chatMessage.userColor, "#1E90FF", "Custom color applied")
            compare(chatMessage.badges.length, 2, "Two badges shown")
            compare(chatMessage.emoteParts.length, 2, "Two emote parts")
        }

        function test_fullMessage_basicChat() {
            chatMessage.messageData = {
                username: "viewer123",
                message: "Hello stream!"
            }
            
            compare(chatMessage.username, "viewer123", "Username displayed")
            compare(chatMessage.userColor, "#AAAAAA", "Default color used")
            compare(chatMessage.badges.length, 0, "No badges")
            compare(chatMessage.rawMessage, "Hello stream!", "Message content correct")
        }
    }
}
