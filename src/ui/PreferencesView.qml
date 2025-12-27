import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "themes/BlueTheme.js" as BlueTheme
import "components"

Item {
  id: preferencesRoot
  signal closeRequested()
  
  // Accès au cache manager
  function getCacheManager() {
    return typeof cacheManager !== "undefined" ? cacheManager : null
  }

  Rectangle {
    anchors.fill: parent
    color: BlueTheme.windowBackground
    opacity: 0.98
  }

  ScrollView {
    anchors.fill: parent
    contentWidth: availableWidth
    ColumnLayout {
      id: prefsLayout
      width: parent.width
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.margins: BlueTheme.spacingLarge
      spacing: BlueTheme.spacingLarge

      Label {
        text: qsTr("Préférences")
        font.pixelSize: 24
        font.bold: true
        color: BlueTheme.primaryText
      }

      // Section Authentification Twitch
      RowLayout {
        Layout.fillWidth: true
        Item { Layout.fillWidth: true }
        BlueCard {
          Layout.preferredWidth: 500
          ColumnLayout {
            width: parent.width
            spacing: BlueTheme.spacingMedium

            Label {
              text: qsTr("Authentification Twitch")
              font.pixelSize: 16
              font.bold: true
              color: BlueTheme.primaryText
            }

            // Accès au service Twitch (disponible globalement)
            function getTwitchService() {
              return typeof twitchService !== "undefined" ? twitchService : null
            }

            Text {
              text: {
                var service = getTwitchService()
                if (service && service.authenticated) {
                  var userName = service.userName || ""
                  return userName ? 
                    qsTr("Vous êtes connecté à Twitch en tant que %1.").arg(userName) : 
                    qsTr("Vous êtes connecté à Twitch.")
                } else {
                  return qsTr("Vous n'êtes pas connecté à Twitch.")
                }
              }
              color: {
                var service = getTwitchService()
                return service && service.authenticated ? 
                  BlueTheme.statusPositive : 
                  BlueTheme.secondaryText
              }
              font.pixelSize: 14
              wrapMode: Text.WordWrap
              Layout.fillWidth: true
            }

            RowLayout {
              Layout.fillWidth: true
              Item { Layout.fillWidth: true }
              BlueButton {
                text: {
                  var service = getTwitchService()
                  return service && service.authenticated ? 
                    qsTr("Se déconnecter") : 
                    qsTr("Se connecter à Twitch")
                }
                onClicked: {
                  var service = getTwitchService()
                  if (service) {
                    if (service.authenticated) {
                      console.log("[PreferencesView] Logging out...")
                      service.logout()
                      // Rediriger vers l'accueil après déconnexion
                      preferencesRoot.closeRequested()
                    } else {
                      console.log("[PreferencesView] Logging in...")
                      service.login()
                    }
                  } else {
                    console.log("[PreferencesView] ERROR: twitchService is not available")
                  }
                }
                filled: {
                  var service = getTwitchService()
                  return service && service.authenticated ? false : true
                }
              }
            }
            
            // Espace en bas pour le padding
            Item {
              Layout.preferredHeight: BlueTheme.spacingMedium
            }

            // Connexion pour fermer automatiquement les préférences après déconnexion
            Connections {
              target: (function() {
                return typeof twitchService !== "undefined" ? twitchService : null
              })()
              enabled: typeof twitchService !== "undefined" && twitchService !== null
              function onAuthenticatedChanged(authenticated) {
                if (!authenticated) {
                  console.log("[PreferencesView] User logged out, closing preferences")
                  preferencesRoot.closeRequested()
                }
              }
            }
          }
        }
        Item { Layout.fillWidth: true }
      }

      // Section Cache
      RowLayout {
        Layout.fillWidth: true
        Item { Layout.fillWidth: true }
        BlueCard {
          Layout.preferredWidth: 500
          ColumnLayout {
            width: parent.width
            spacing: BlueTheme.spacingMedium

            Label {
              text: qsTr("Cache des replays")
              font.pixelSize: 16
              font.bold: true
              color: BlueTheme.primaryText
            }

            Text {
              text: {
                var cm = getCacheManager()
                if (cm) {
                  return qsTr("%1 replays enregistres - %2 utilises")
                    .arg(cm.vodCount)
                    .arg(cm.formattedTotalSize())
                }
                return qsTr("Cache non disponible")
              }
              color: BlueTheme.secondaryText
              font.pixelSize: 14
              wrapMode: Text.WordWrap
              Layout.fillWidth: true
            }

            // Barre de progression
            Rectangle {
              Layout.fillWidth: true
              Layout.preferredHeight: 6
              radius: 3
              color: BlueTheme.divider

              Rectangle {
                width: {
                  var cm = getCacheManager()
                  if (cm) {
                    return parent.width * Math.min(1, cm.cacheUsagePercent() / 100)
                  }
                  return 0
                }
                height: parent.height
                radius: 3
                color: {
                  var cm = getCacheManager()
                  if (cm) {
                    var usage = cm.cacheUsagePercent()
                    return usage > 90 ? BlueTheme.statusNegative :
                           usage > 70 ? BlueTheme.statusWarning : BlueTheme.accent
                  }
                  return BlueTheme.accent
                }
              }
            }

            // Taille maximale avec input numérique
            RowLayout {
              Layout.fillWidth: true
              spacing: 12

              Text {
                text: qsTr("Taille maximale")
                font.pixelSize: 14
                color: BlueTheme.secondaryText
                Layout.alignment: Qt.AlignVCenter
              }

              Item { Layout.fillWidth: true }

              // Contrôle numérique avec stepper intégré
              Rectangle {
                width: 140
                height: 32
                radius: 8
                color: BlueTheme.surface
                border.color: BlueTheme.divider
                border.width: 1
                Layout.alignment: Qt.AlignVCenter

                RowLayout {
                  anchors.fill: parent
                  anchors.margins: 2
                  spacing: 0

                  // Bouton -
                  Rectangle {
                    Layout.preferredWidth: 32
                    Layout.fillHeight: true
                    radius: 6
                    color: minusBtnArea.containsMouse ? BlueTheme.divider : "transparent"

                    Text {
                      anchors.centerIn: parent
                      text: "-"
                      font.pixelSize: 16
                      font.bold: true
                      color: BlueTheme.primaryText
                    }

                    MouseArea {
                      id: minusBtnArea
                      anchors.fill: parent
                      hoverEnabled: true
                      cursorShape: Qt.PointingHandCursor
                      onClicked: {
                        var cm = getCacheManager()
                        if (cm) {
                          var currentGB = cm.maxCacheSize / (1024 * 1024 * 1024)
                          var newGB = Math.max(1, currentGB - 10)
                          cm.setMaxCacheSize(newGB * 1024 * 1024 * 1024)
                        }
                      }
                    }
                  }

                  // Valeur
                  Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    TextInput {
                      id: cacheSizeInput
                      anchors.centerIn: parent
                      width: parent.width
                      horizontalAlignment: Text.AlignHCenter
                      font.family: BlueTheme.fontFamily
                      font.pixelSize: 14
                      font.bold: true
                      color: BlueTheme.primaryText
                      text: {
                        var cm = getCacheManager()
                        if (cm) {
                          return Math.round(cm.maxCacheSize / (1024 * 1024 * 1024)).toString()
                        }
                        return "10"
                      }
                      validator: IntValidator { bottom: 1; top: 9999 }
                      selectByMouse: true
                      onEditingFinished: {
                        var cm = getCacheManager()
                        if (cm) {
                          var newGB = parseInt(text) || 10
                          newGB = Math.max(1, newGB)
                          cm.setMaxCacheSize(newGB * 1024 * 1024 * 1024)
                        }
                      }
                    }
                  }

                  // Bouton +
                  Rectangle {
                    Layout.preferredWidth: 32
                    Layout.fillHeight: true
                    radius: 6
                    color: plusBtnArea.containsMouse ? BlueTheme.divider : "transparent"

                    Text {
                      anchors.centerIn: parent
                      text: "+"
                      font.pixelSize: 16
                      font.bold: true
                      color: BlueTheme.primaryText
                    }

                    MouseArea {
                      id: plusBtnArea
                      anchors.fill: parent
                      hoverEnabled: true
                      cursorShape: Qt.PointingHandCursor
                      onClicked: {
                        var cm = getCacheManager()
                        if (cm) {
                          var currentGB = cm.maxCacheSize / (1024 * 1024 * 1024)
                          var newGB = currentGB + 10
                          cm.setMaxCacheSize(newGB * 1024 * 1024 * 1024)
                        }
                      }
                    }
                  }
                }
              }

              Text {
                text: "GB"
                font.pixelSize: 14
                color: BlueTheme.secondaryText
                Layout.alignment: Qt.AlignVCenter
              }
            }

            // Bouton vider le cache
            RowLayout {
              Layout.fillWidth: true
              Layout.topMargin: BlueTheme.spacingSmall
              Item { Layout.fillWidth: true }
              
              Rectangle {
                width: clearCacheText.width + 2 * BlueTheme.spacingMedium
                height: clearCacheText.height + 2 * BlueTheme.spacingMedium
                radius: 14
                color: clearCacheArea.containsMouse ? Qt.rgba(BlueTheme.statusNegative.r, BlueTheme.statusNegative.g, BlueTheme.statusNegative.b, 0.1) : "transparent"
                border.color: BlueTheme.statusNegative
                border.width: 1
                visible: {
                  var cm = getCacheManager()
                  return cm && cm.vodCount > 0
                }
                
                Behavior on color {
                  ColorAnimation { duration: 150 }
                }

                Text {
                  id: clearCacheText
                  anchors.centerIn: parent
                  text: qsTr("Vider le cache")
                  font.family: BlueTheme.fontFamily
                  font.pixelSize: 13
                  color: BlueTheme.statusNegative
                }

                MouseArea {
                  id: clearCacheArea
                  anchors.fill: parent
                  hoverEnabled: true
                  cursorShape: Qt.PointingHandCursor
                  onClicked: clearCacheDialog.open()
                }
              }
            }
            
            Item {
              Layout.preferredHeight: BlueTheme.spacingSmall
            }
          }
        }
        Item { Layout.fillWidth: true }
      }

      RowLayout {
        Layout.fillWidth: true
        Item { Layout.fillWidth: true }
        BlueButton {
          text: qsTr("Fermer et revenir à l'accueil")
          onClicked: closeRequested()
        }
        Item { Layout.fillWidth: true }
      }
    }
  }
  
  // Dialog pour vider le cache
  Popup {
    id: clearCacheDialog
    modal: true
    anchors.centerIn: parent
    width: 320
    padding: 24

    background: Rectangle {
      radius: 16
      color: BlueTheme.surface
      border.color: BlueTheme.divider
      border.width: 1
    }

    ColumnLayout {
      anchors.fill: parent
      spacing: 16

      Text {
        text: qsTr("Vider tout le cache ?")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 16
        font.bold: true
        color: BlueTheme.primaryText
      }

      Text {
        text: {
          var cm = getCacheManager()
          if (cm) {
            return qsTr("%1 replays seront supprimes.\nEspace libere: %2")
              .arg(cm.vodCount)
              .arg(cm.formattedTotalSize())
          }
          return ""
        }
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: BlueTheme.secondaryText
      }

      RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Item { Layout.fillWidth: true }

        Rectangle {
          width: cancelClearText.width + 24
          height: 36
          radius: 18
          color: "transparent"
          border.color: BlueTheme.divider
          border.width: 1

          Text {
            id: cancelClearText
            anchors.centerIn: parent
            text: qsTr("Annuler")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: BlueTheme.primaryText
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: clearCacheDialog.close()
          }
        }

        Rectangle {
          width: confirmClearText.width + 24
          height: 36
          radius: 18
          color: BlueTheme.statusNegative

          Text {
            id: confirmClearText
            anchors.centerIn: parent
            text: qsTr("Vider")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: "#FFFFFF"
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              var cm = getCacheManager()
              if (cm) {
                cm.clearAllVods()
              }
              clearCacheDialog.close()
            }
          }
        }
      }
    }
  }
}

