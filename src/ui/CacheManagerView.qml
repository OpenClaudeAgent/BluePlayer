import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import BluePlayer.UI 1.0

import "themes/BlueTheme.js" as BlueTheme
import "components"

Item {
  id: root

  signal backRequested()
  signal playVodRequested(string vodId, string filePath, var metadata)

  property var cacheManager: null

  CacheManagerViewModel {
    id: viewModel
    Component.onCompleted: {
      if (root.cacheManager) {
        viewModel.initialize(root.cacheManager)
      }
    }
  }

  onCacheManagerChanged: {
    if (cacheManager) {
      viewModel.initialize(cacheManager)
    }
  }

  // Fond avec gradient identique au reste de l'app
  Rectangle {
    anchors.fill: parent
    gradient: Gradient {
      GradientStop { position: 0; color: BlueTheme.gradientStart }
      GradientStop { position: 1; color: BlueTheme.gradientEnd }
    }
  }

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: BlueTheme.spacingLarge
    spacing: BlueTheme.spacingMedium

    // Header compact
    RowLayout {
      Layout.fillWidth: true
      Layout.rightMargin: 100  // Espace pour les icônes globales (↺ et ⚙️)
      spacing: BlueTheme.spacingMedium

      // Bouton retour
      Rectangle {
        Layout.preferredWidth: 36
        Layout.preferredHeight: 36
        radius: 18
        color: backButtonArea.containsMouse ? BlueTheme.surfaceSoft : "transparent"

        Text {
          anchors.centerIn: parent
          text: "\u2190"
          font.pixelSize: 18
          color: BlueTheme.primaryText
        }

        MouseArea {
          id: backButtonArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: root.backRequested()
        }
      }

      // Titre
      Text {
        text: qsTr("Mes Replays")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 22
        font.bold: true
        color: BlueTheme.primaryText
      }

      // Stats compactes
      Text {
        text: qsTr("%1 videos  ·  %2 / %3")
                .arg(viewModel.vodCount)
                .arg(viewModel.totalSizeFormatted)
                .arg(viewModel.maxSizeFormatted)
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: BlueTheme.mutedText
      }

      Item { Layout.fillWidth: true }
    }

    // Barre de progression du cache (compacte)
    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: 6
      radius: 3
      color: BlueTheme.divider

      Rectangle {
        width: parent.width * Math.min(1, viewModel.usagePercent / 100)
        height: parent.height
        radius: 3
        color: viewModel.usagePercent > 90 ? BlueTheme.statusNegative :
               viewModel.usagePercent > 70 ? BlueTheme.statusWarning : BlueTheme.accent

        Behavior on width {
          NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }
      }
    }

    // Barre de tri/filtre/sélection
    RowLayout {
      Layout.fillWidth: true
      spacing: BlueTheme.spacingMedium

      Text {
        text: qsTr("Trier:")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 12
        color: BlueTheme.mutedText
      }

      // Boutons de tri style pill
      Repeater {
        model: [
          { text: qsTr("Date"), value: "recordedAt" },
          { text: qsTr("Taille"), value: "fileSize" },
          { text: qsTr("Duree"), value: "duration" }
        ]

        Rectangle {
          width: sortBtnText.width + 16
          height: 28
          radius: 14
          color: viewModel.sortField === modelData.value ? BlueTheme.surfaceSoft : "transparent"
          border.color: viewModel.sortField === modelData.value ? BlueTheme.divider : "transparent"
          border.width: 1

          Text {
            id: sortBtnText
            anchors.centerIn: parent
            text: modelData.text
            font.family: BlueTheme.fontFamily
            font.pixelSize: 12
            color: viewModel.sortField === modelData.value ? BlueTheme.primaryText : BlueTheme.secondaryText
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: viewModel.sortField = modelData.value
          }
        }
      }

      // Toggle ordre
      Rectangle {
        width: 28
        height: 28
        radius: 14
        color: sortOrderArea.containsMouse ? BlueTheme.surfaceSoft : "transparent"

        Text {
          anchors.centerIn: parent
          text: viewModel.sortAscending ? "\u2191" : "\u2193"
          font.pixelSize: 14
          color: BlueTheme.secondaryText
        }

        MouseArea {
          id: sortOrderArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: viewModel.sortAscending = !viewModel.sortAscending
        }
      }

      // Séparateur
      Rectangle {
        width: 1
        height: 20
        color: BlueTheme.mutedText
        opacity: 0.4
      }

      // Mode sélection
      Rectangle {
        width: selModeText.width + 16
        height: 28
        radius: 14
        color: viewModel.selectionMode ? BlueTheme.accent : (selModeArea.containsMouse ? BlueTheme.surfaceSoft : "transparent")
        border.color: viewModel.selectionMode ? BlueTheme.accent : "transparent"
        border.width: 1

        Text {
          id: selModeText
          anchors.centerIn: parent
          text: viewModel.selectionMode ? qsTr("Annuler") : qsTr("Selectionner")
          font.family: BlueTheme.fontFamily
          font.pixelSize: 12
          color: viewModel.selectionMode ? BlueTheme.windowBackground : BlueTheme.secondaryText
        }

        MouseArea {
          id: selModeArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: viewModel.selectionMode = !viewModel.selectionMode
        }
      }

      // Supprimer sélection
      Rectangle {
        visible: viewModel.selectionMode && viewModel.selectedCount > 0
        width: delSelText.width + 16
        height: 28
        radius: 14
        color: BlueTheme.statusNegative

        Text {
          id: delSelText
          anchors.centerIn: parent
          text: qsTr("Supprimer (%1)").arg(viewModel.selectedCount)
          font.family: BlueTheme.fontFamily
          font.pixelSize: 12
          color: "#FFFFFF"
        }

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor
          onClicked: deleteSelectedDialog.open()
        }
      }

      Item { Layout.fillWidth: true }

      // Filtre streamer (si plusieurs)
      Text {
        visible: viewModel.getStreamerList().length > 1
        text: qsTr("Streamer:")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 12
        color: BlueTheme.mutedText
      }

      ComboBox {
        id: streamerFilter
        visible: viewModel.getStreamerList().length > 1
        Layout.preferredWidth: 140
        model: {
          var list = [{ text: qsTr("Tous"), value: "" }]
          var streamers = viewModel.getStreamerList()
          for (var i = 0; i < streamers.length; i++) {
            list.push({ text: streamers[i], value: streamers[i] })
          }
          return list
        }
        textRole: "text"
        valueRole: "value"
        currentIndex: 0
        onCurrentValueChanged: viewModel.filterStreamer = currentValue

        background: Rectangle {
          radius: 14
          color: BlueTheme.surface
          border.color: BlueTheme.divider
          border.width: 1
        }

        contentItem: Text {
          leftPadding: 12
          text: streamerFilter.displayText
          font.family: BlueTheme.fontFamily
          font.pixelSize: 12
          color: BlueTheme.primaryText
          verticalAlignment: Text.AlignVCenter
        }
      }
    }

    // Liste des VOD
    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

      GridView {
        id: vodGridView
        anchors.fill: parent
        cellWidth: 280
        cellHeight: 180
        model: viewModel.vodList

        delegate: Item {
          width: vodGridView.cellWidth
          height: vodGridView.cellHeight

          Rectangle {
            id: cardRect
            anchors.fill: parent
            anchors.margins: 8
            radius: 12
            // Force re-evaluation when selectedCount changes
            property bool isCardSelected: viewModel.selectedCount >= 0 && viewModel.isSelected(modelData.id)
            color: delegateMouseArea.containsMouse ? BlueTheme.surfaceSoft : BlueTheme.surface
            border.color: isCardSelected ? BlueTheme.accent : "transparent"
            border.width: isCardSelected ? 2 : 0

            ColumnLayout {
              anchors.fill: parent
              anchors.margins: 12
              spacing: 8

              // Thumbnail area
              Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 90
                radius: 8
                color: BlueTheme.divider
                clip: true

                Image {
                  anchors.fill: parent
                  source: modelData.thumbnailPath || ""
                  fillMode: Image.PreserveAspectCrop
                  visible: modelData.thumbnailPath && modelData.thumbnailPath.length > 0
                }

                // Placeholder
                Text {
                  anchors.centerIn: parent
                  text: "\uD83C\uDFA5"
                  font.pixelSize: 28
                  color: BlueTheme.mutedText
                  visible: !modelData.thumbnailPath || modelData.thumbnailPath.length === 0
                }

                // Badge durée
                Rectangle {
                  anchors.right: parent.right
                  anchors.bottom: parent.bottom
                  anchors.margins: 6
                  width: durText.width + 8
                  height: 18
                  radius: 4
                  color: "#CC000000"

                  Text {
                    id: durText
                    anchors.centerIn: parent
                    text: modelData.durationFormatted || ""
                    font.family: BlueTheme.fontFamily
                    font.pixelSize: 10
                    font.bold: true
                    color: "#FFFFFF"
                  }
                }

                // Checkbox sélection
                Rectangle {
                  id: selectionCheckbox
                  visible: viewModel.selectionMode
                  anchors.left: parent.left
                  anchors.top: parent.top
                  anchors.margins: 6
                  width: 22
                  height: 22
                  radius: 11
                  // Force re-evaluation when selectedCount changes
                  property bool isChecked: viewModel.selectedCount >= 0 && viewModel.isSelected(modelData.id)
                  color: isChecked ? BlueTheme.accent : BlueTheme.surface
                  border.color: isChecked ? BlueTheme.accent : BlueTheme.divider
                  border.width: 2

                  Text {
                    anchors.centerIn: parent
                    text: "\u2713"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#FFFFFF"
                    visible: selectionCheckbox.isChecked
                  }
                }

                // Play overlay on hover
                Rectangle {
                  anchors.centerIn: parent
                  width: 40
                  height: 40
                  radius: 20
                  color: "#CC000000"
                  visible: delegateMouseArea.containsMouse && !viewModel.selectionMode
                  opacity: delegateMouseArea.containsMouse ? 1 : 0

                  Text {
                    anchors.centerIn: parent
                    text: "\u25B6"
                    font.pixelSize: 16
                    color: "#FFFFFF"
                  }

                  Behavior on opacity {
                    NumberAnimation { duration: 150 }
                  }
                }

                // Barre de progression visionnage
                Rectangle {
                  visible: modelData.watchPosition > 0
                  anchors.left: parent.left
                  anchors.right: parent.right
                  anchors.bottom: parent.bottom
                  height: 3
                  color: "#80000000"

                  Rectangle {
                    width: parent.width * (modelData.watchPosition / modelData.duration)
                    height: parent.height
                    color: BlueTheme.accent
                  }
                }
              }

              // Info
              ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                  text: modelData.streamerName || qsTr("Inconnu")
                  font.family: BlueTheme.fontFamily
                  font.pixelSize: 13
                  font.bold: true
                  color: BlueTheme.primaryText
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                }

                Text {
                  text: modelData.streamTitle || qsTr("Sans titre")
                  font.family: BlueTheme.fontFamily
                  font.pixelSize: 11
                  color: BlueTheme.secondaryText
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 8

                  Text {
                    text: modelData.recordedAtFormatted || ""
                    font.family: BlueTheme.fontFamily
                    font.pixelSize: 10
                    color: BlueTheme.mutedText
                  }

                  Text {
                    text: "·"
                    font.pixelSize: 10
                    color: BlueTheme.mutedText
                  }

                  Text {
                    text: modelData.fileSizeFormatted || ""
                    font.family: BlueTheme.fontFamily
                    font.pixelSize: 10
                    color: BlueTheme.mutedText
                  }
                }
              }
            }

            MouseArea {
              id: delegateMouseArea
              anchors.fill: parent
              hoverEnabled: true
              cursorShape: Qt.PointingHandCursor
              onClicked: {
                if (viewModel.selectionMode) {
                  viewModel.toggleSelection(modelData.id, !viewModel.isSelected(modelData.id))
                } else {
                  root.playVodRequested(modelData.id, modelData.filePath, modelData)
                }
              }
            }

            Behavior on color {
              ColorAnimation { duration: 150 }
            }
          }
        }

        // État vide
        Item {
          anchors.centerIn: parent
          width: 300
          height: 200
          visible: viewModel.vodCount === 0

          ColumnLayout {
            anchors.centerIn: parent
            spacing: 16

            Text {
              Layout.alignment: Qt.AlignHCenter
              text: "\uD83D\uDCF9"
              font.pixelSize: 48
              opacity: 0.5
            }

            Text {
              Layout.alignment: Qt.AlignHCenter
              text: qsTr("Aucun replay")
              font.family: BlueTheme.fontFamily
              font.pixelSize: 16
              font.bold: true
              color: BlueTheme.secondaryText
            }

            Text {
              Layout.alignment: Qt.AlignHCenter
              text: qsTr("Les streams regardes apparaitront ici")
              font.family: BlueTheme.fontFamily
              font.pixelSize: 13
              color: BlueTheme.mutedText
              horizontalAlignment: Text.AlignHCenter
            }
          }
        }
      }
    }
  }

  // Dialog suppression sélection
  Popup {
    id: deleteSelectedDialog
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
        text: qsTr("Supprimer %1 replays ?").arg(viewModel.selectedCount)
        font.family: BlueTheme.fontFamily
        font.pixelSize: 16
        font.bold: true
        color: BlueTheme.primaryText
      }

      Text {
        text: qsTr("Cette action est irreversible.")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: BlueTheme.secondaryText
      }

      RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Item { Layout.fillWidth: true }

        Rectangle {
          width: cancelText.width + 24
          height: 36
          radius: 18
          color: "transparent"
          border.color: BlueTheme.divider
          border.width: 1

          Text {
            id: cancelText
            anchors.centerIn: parent
            text: qsTr("Annuler")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: BlueTheme.primaryText
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: deleteSelectedDialog.close()
          }
        }

        Rectangle {
          width: confirmDelText.width + 24
          height: 36
          radius: 18
          color: BlueTheme.statusNegative

          Text {
            id: confirmDelText
            anchors.centerIn: parent
            text: qsTr("Supprimer")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: "#FFFFFF"
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              viewModel.deleteSelected()
              viewModel.selectionMode = false
              deleteSelectedDialog.close()
            }
          }
        }
      }
    }
  }

  // Dialog vider tout
  Popup {
    id: clearAllDialog
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
        text: qsTr("%1 replays seront supprimes.\nEspace libere: %2")
                .arg(viewModel.vodCount)
                .arg(viewModel.totalSizeFormatted)
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: BlueTheme.secondaryText
      }

      RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Item { Layout.fillWidth: true }

        Rectangle {
          width: cancelAllText.width + 24
          height: 36
          radius: 18
          color: "transparent"
          border.color: BlueTheme.divider
          border.width: 1

          Text {
            id: cancelAllText
            anchors.centerIn: parent
            text: qsTr("Annuler")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: BlueTheme.primaryText
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: clearAllDialog.close()
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
              viewModel.clearAll()
              clearAllDialog.close()
            }
          }
        }
      }
    }
  }
}
