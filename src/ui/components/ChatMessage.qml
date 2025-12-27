import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * ChatMessage - Single chat message with badges and emotes
 */
Item {
    id: root

    property var messageData: ({})
    property int emoteSize: 24
    property int badgeSize: 18

    // Extract data with defaults
    readonly property string username: messageData.displayName || messageData.username || "Anonymous"
    readonly property string userColor: messageData.color || "#AAAAAA"
    readonly property var badges: messageData.badges || []
    readonly property var emoteParts: messageData.emoteParts || []
    readonly property string rawMessage: messageData.message || ""

    implicitHeight: messageFlow.height + 8
    implicitWidth: parent ? parent.width : 300

    // Message container
    Flow {
        id: messageFlow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        spacing: 3

        // Badges - styled rectangles with tooltips
        Repeater {
            id: badgeRepeater
            model: root.badges

            delegate: Rectangle {
                required property var modelData
                required property int index
                
                width: badgeSize
                height: badgeSize
                radius: 3
                color: getBadgeColor(modelData.type || "")

                Text {
                    anchors.centerIn: parent
                    text: getBadgeIcon(modelData.type || "")
                    font.pixelSize: 10
                    font.bold: true
                    color: "#FFFFFF"
                }

                MouseArea {
                    id: badgeMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                }

                ToolTip.visible: badgeMouseArea.containsMouse
                ToolTip.text: getBadgeDescription(modelData.type || "", modelData.version || "")
                ToolTip.delay: 300
            }
        }

        // Username
        Text {
            text: root.username + ":"
            color: root.userColor
            font.pixelSize: 13
            font.bold: true
            font.family: "SF Pro Text, -apple-system, Helvetica Neue"
        }

        // Space after username
        Item { width: 4; height: 1 }

        // Message parts (text and emotes)
        Repeater {
            id: partsRepeater
            model: root.emoteParts.length > 0 ? root.emoteParts : [{type: "text", content: root.rawMessage}]

            delegate: Loader {
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
            text: partData.content || ""
            color: "#FFFFFF"
            font.pixelSize: 13
            font.family: "SF Pro Text, -apple-system, Helvetica Neue"
            wrapMode: Text.Wrap
        }
    }

    // Emote component
    Component {
        id: emoteComponent

        Image {
            width: emoteSize
            height: emoteSize
            sourceSize: Qt.size(emoteSize * 2, emoteSize * 2)
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            cache: true
            source: getEmoteUrl(partData.emoteId)

            // Fallback to text if emote fails to load
            Text {
                anchors.centerIn: parent
                visible: parent.status === Image.Error
                text: partData.content || ""
                color: "#FFFFFF"
                font.pixelSize: 13
            }
        }
    }

    // Helper functions
    function getEmoteUrl(emoteId) {
        if (!emoteId) return ""
        // Twitch emote CDN - 1.0 = 28px, 2.0 = 56px, 3.0 = 112px
        return "https://static-cdn.jtvnw.net/emoticons/v2/" + emoteId + "/default/dark/2.0"
    }

    function getBadgeColor(badgeType) {
        if (!badgeType) return "transparent"
        
        var colors = {
            "broadcaster": "#E91916",   // Red
            "moderator": "#00AD03",     // Green  
            "vip": "#E005B9",           // Pink/Magenta
            "subscriber": "#9147FF",    // Twitch purple
            "sub-gifter": "#9147FF",    // Twitch purple
            "sub-gift-leader": "#9147FF",
            "bits": "#FAAF19",          // Gold
            "bits-leader": "#FAAF19",
            "premium": "#9147FF",       // Twitch purple
            "turbo": "#9147FF",         // Twitch purple
            "partner": "#9147FF",       // Twitch purple
            "verified": "#9147FF",
            "founder": "#9147FF",
            "staff": "#E91916",         // Red
            "admin": "#FAAF19",         // Gold/Orange
            "global_mod": "#00AD03",    // Green
            "glhf-pledge": "#30D158",   // Green
            "hype-train": "#9147FF",
            "predictions": "#3498DB",   // Blue
            "moments": "#9147FF",
            "artist-badge": "#E005B9",
            "no_audio": "#FF6B6B",
            "no_video": "#FF6B6B"
        }
        
        // Check for partial matches (e.g., "subscriber/12" -> "subscriber")
        var baseType = badgeType.split("/")[0]
        return colors[baseType] || "#6E6E73"  // Default gray for unknown badges
    }

    function getBadgeIcon(badgeType) {
        if (!badgeType) return ""
        
        var icons = {
            "broadcaster": "B",
            "moderator": "M",
            "vip": "V",
            "subscriber": "S",
            "sub-gifter": "G",
            "sub-gift-leader": "G",
            "bits": "$",
            "bits-leader": "$",
            "premium": "P",
            "turbo": "T",
            "partner": "P",
            "verified": "V",
            "founder": "F",
            "staff": "S",
            "admin": "A",
            "global_mod": "G",
            "glhf-pledge": "G",
            "hype-train": "H",
            "predictions": "?",
            "moments": "M",
            "artist-badge": "A",
            "no_audio": "!",
            "no_video": "!"
        }
        
        var baseType = badgeType.split("/")[0]
        return icons[baseType] || baseType.charAt(0).toUpperCase()
    }

    function getBadgeDescription(badgeType, version) {
        if (!badgeType) return ""
        
        var descriptions = {
            "broadcaster": qsTr("Broadcaster - Channel owner"),
            "moderator": qsTr("Moderator - Can moderate chat"),
            "vip": qsTr("VIP - Special community member"),
            "subscriber": qsTr("Subscriber") + (version ? " (" + version + " " + qsTr("months") + ")" : ""),
            "sub-gifter": qsTr("Sub Gifter - Has gifted subscriptions"),
            "sub-gift-leader": qsTr("Sub Gift Leader"),
            "bits": qsTr("Bits - Has cheered with bits"),
            "bits-leader": qsTr("Bits Leader"),
            "premium": qsTr("Twitch Prime"),
            "turbo": qsTr("Twitch Turbo"),
            "partner": qsTr("Twitch Partner"),
            "verified": qsTr("Verified"),
            "founder": qsTr("Founder - Early subscriber"),
            "staff": qsTr("Twitch Staff"),
            "admin": qsTr("Twitch Admin"),
            "global_mod": qsTr("Global Moderator"),
            "glhf-pledge": qsTr("GLHF Pledge"),
            "hype-train": qsTr("Hype Train Contributor"),
            "predictions": qsTr("Predictions Participant"),
            "moments": qsTr("Moments Badge"),
            "artist-badge": qsTr("Artist")
        }
        
        var baseType = badgeType.split("/")[0]
        return descriptions[baseType] || badgeType
    }
}
