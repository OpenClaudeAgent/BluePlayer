import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import BluePlayer.UI 1.0

import "themes/BlueTheme.js" as BlueTheme
import "components"

Item {
  id: root

  // Theme access
  readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null

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
      GradientStop { position: 0; color: tm ? tm.gradientStart : BlueTheme.gradientStart }
      GradientStop { position: 1; color: tm ? tm.gradientEnd : BlueTheme.gradientEnd }
    }
  }

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 0
    spacing: 0

    // Header unifié - Utilise PanelHeader (pas de rightMargin hack nécessaire)
    PanelHeader {
      Layout.fillWidth: true
      title: qsTr("My Replays")
      onBackClicked: root.backRequested()
    }

    // Stats compactes sous le header
    RowLayout {
      Layout.fillWidth: true
      Layout.leftMargin: BlueTheme.spacingLarge
      Layout.rightMargin: BlueTheme.spacingLarge
      Layout.topMargin: BlueTheme.spacingMedium
      spacing: BlueTheme.spacingMedium

      Text {
        text: qsTr("%1 videos  ·  %2 / %3")
                .arg(viewModel.vodCount)
                .arg(viewModel.totalSizeFormatted)
                .arg(viewModel.maxSizeFormatted)
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: tm ? tm.mutedText : BlueTheme.mutedText
      }

      Item { Layout.fillWidth: true }
    }

    // Barre de progression du cache (compacte)
    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: 6
      Layout.leftMargin: BlueTheme.spacingLarge
      Layout.rightMargin: BlueTheme.spacingLarge
      Layout.topMargin: BlueTheme.spacingSmall
      radius: 3
      color: tm ? tm.divider : BlueTheme.divider

      Rectangle {
        width: parent.width * Math.min(1, viewModel.usagePercent / 100)
        height: parent.height
        radius: 3
        color: viewModel.usagePercent > 90 ? (tm ? tm.statusNegative : BlueTheme.statusNegative) :
               viewModel.usagePercent > 70 ? (tm ? tm.statusWarning : BlueTheme.statusWarning) : (tm ? tm.accent : BlueTheme.accent)

        Behavior on width {
          NumberAnimation { duration: BlueTheme.animPanelDuration; easing.type: Easing.OutCubic }
        }
      }
    }

    // Barre de tri/filtre/sélection
    RowLayout {
      Layout.fillWidth: true
      Layout.leftMargin: BlueTheme.spacingLarge
      Layout.rightMargin: BlueTheme.spacingLarge
      Layout.topMargin: BlueTheme.spacingMedium
      spacing: BlueTheme.spacingMedium

      Text {
        text: qsTr("Sort:")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 12
        color: tm ? tm.mutedText : BlueTheme.mutedText
      }

      // Boutons de tri style pill
      Repeater {
        model: [
          { text: qsTr("Date"), value: "recordedAt" },
          { text: qsTr("Last watched"), value: "lastPlayedAt" },
          { text: qsTr("Size"), value: "fileSize" },
          { text: qsTr("Duration"), value: "duration" }
        ]

        Rectangle {
          width: sortBtnText.width + 16
          height: 28
          radius: 14
          color: viewModel.sortField === modelData.value ? (tm ? tm.surfaceSoft : BlueTheme.surfaceSoft) : "transparent"
          border.color: viewModel.sortField === modelData.value ? (tm ? tm.divider : BlueTheme.divider) : "transparent"
          border.width: 1

          Text {
            id: sortBtnText
            anchors.centerIn: parent
            text: modelData.text
            font.family: BlueTheme.fontFamily
            font.pixelSize: 12
            color: viewModel.sortField === modelData.value ? (tm ? tm.primaryText : BlueTheme.primaryText) : (tm ? tm.secondaryText : BlueTheme.secondaryText)
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              console.info("[Cache] Sort changed:", modelData.value)
              viewModel.sortField = modelData.value
            }
          }
        }
      }

      // Toggle ordre
      Rectangle {
        width: 28
        height: 28
        radius: 14
        color: sortOrderArea.containsMouse ? (tm ? tm.surfaceSoft : BlueTheme.surfaceSoft) : "transparent"

        Text {
          anchors.centerIn: parent
          text: viewModel.sortAscending ? "\u2191" : "\u2193"
          font.pixelSize: 14
          color: tm ? tm.secondaryText : BlueTheme.secondaryText
        }

        MouseArea {
          id: sortOrderArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: {
            var newOrder = !viewModel.sortAscending
            console.info("[Cache] Sort order:", newOrder ? "ascending" : "descending")
            viewModel.sortAscending = newOrder
          }
        }
      }

      // Séparateur
      Rectangle {
        width: 1
        height: 20
        color: tm ? tm.mutedText : BlueTheme.mutedText
        opacity: 0.4
      }

      // Mode sélection
      Rectangle {
        width: selModeText.width + 16
        height: 28
        radius: 14
        color: viewModel.selectionMode ? (tm ? tm.accent : BlueTheme.accent) : (selModeArea.containsMouse ? (tm ? tm.surfaceSoft : BlueTheme.surfaceSoft) : "transparent")
        border.color: viewModel.selectionMode ? (tm ? tm.accent : BlueTheme.accent) : "transparent"
        border.width: 1

        Text {
          id: selModeText
          anchors.centerIn: parent
          text: viewModel.selectionMode ? qsTr("Cancel") : qsTr("Select")
          font.family: BlueTheme.fontFamily
          font.pixelSize: 12
          color: viewModel.selectionMode ? (tm ? tm.windowBackground : BlueTheme.windowBackground) : (tm ? tm.secondaryText : BlueTheme.secondaryText)
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
        color: tm ? tm.statusNegative : BlueTheme.statusNegative

        Text {
          id: delSelText
          anchors.centerIn: parent
          text: qsTr("Delete (%1)").arg(viewModel.selectedCount)
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
        color: tm ? tm.mutedText : BlueTheme.mutedText
      }

      ComboBox {
        id: streamerFilter
        visible: viewModel.getStreamerList().length > 1
        Layout.preferredWidth: 140
        model: {
          var list = [{ text: qsTr("All"), value: "" }]
          var streamers = viewModel.getStreamerList()
          for (var i = 0; i < streamers.length; i++) {
            list.push({ text: streamers[i], value: streamers[i] })
          }
          return list
        }
        textRole: "text"
        valueRole: "value"
        currentIndex: 0
        onCurrentValueChanged: {
          if (currentValue) {
            console.info("[Cache] Filter by streamer:", currentValue)
          } else {
            console.info("[Cache] Filter cleared")
          }
          viewModel.filterStreamer = currentValue
        }

        background: Rectangle {
          radius: 14
          color: tm ? tm.surface : BlueTheme.surface
          border.color: tm ? tm.divider : BlueTheme.divider
          border.width: 1
        }

        contentItem: Text {
          leftPadding: 12
          text: streamerFilter.displayText
          font.family: BlueTheme.fontFamily
          font.pixelSize: 12
          color: tm ? tm.primaryText : BlueTheme.primaryText
          verticalAlignment: Text.AlignVCenter
        }
      }
    }

    // Liste des VOD
    // Liste des VOD avec Flow pour hauteur dynamique
    Flickable {
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.leftMargin: BlueTheme.spacingLarge
      Layout.rightMargin: BlueTheme.spacingLarge
      Layout.topMargin: BlueTheme.spacingMedium
      clip: true
      contentWidth: width
      contentHeight: vodFlowWrapper.height
      boundsBehavior: Flickable.StopAtBounds
      
      ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
      }

      // Wrapper pour permettre l'effet scale sans crop
      Item {
        id: vodFlowWrapper
        width: parent.width
        height: vodFlowLayout.implicitHeight + BlueTheme.spacingSmall * 2

        Flow {
          id: vodFlowLayout
          x: BlueTheme.spacingSmall
          y: BlueTheme.spacingSmall
          width: parent.width - BlueTheme.spacingSmall * 2
          spacing: BlueTheme.spacingMedium

        Repeater {
          model: viewModel.vodList

          Rectangle {
            id: cardRect
            width: 200
            implicitHeight: cardContent.implicitHeight + BlueTheme.spacingMedium * 2
            radius: 12
            
            property bool isCardSelected: viewModel.selectedCount >= 0 && viewModel.isSelected(modelData.id)
            color: delegateMouseArea.containsMouse ? (tm ? tm.cardHighlight : BlueTheme.cardHighlight) : (tm ? tm.surface : BlueTheme.surface)
            border.color: isCardSelected ? (tm ? tm.accent : BlueTheme.accent) : (tm ? tm.divider : BlueTheme.divider)
            border.width: isCardSelected ? 2 : 1
            scale: delegateMouseArea.containsMouse ? BlueTheme.scaleHover : 1.0

            Behavior on color {
              ColorAnimation { duration: BlueTheme.animCardDuration; easing.type: Easing.OutCubic }
            }
            Behavior on scale {
              NumberAnimation { duration: BlueTheme.animCardDuration; easing.type: Easing.OutCubic }
            }

            ColumnLayout {
              id: cardContent
              anchors.left: parent.left
              anchors.right: parent.right
              anchors.top: parent.top
              anchors.leftMargin: BlueTheme.spacingMedium
              anchors.rightMargin: BlueTheme.spacingMedium
              anchors.topMargin: BlueTheme.spacingMedium
              spacing: BlueTheme.spacingSmall

              // Thumbnail area - ratio 16:9
              Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: (cardRect.width - BlueTheme.spacingMedium * 2) * 9 / 16
                radius: 8
                color: tm ? tm.divider : BlueTheme.divider
                clip: true

                Image {
                  anchors.fill: parent
                  source: modelData.thumbnailPath || ""
                  fillMode: Image.PreserveAspectCrop
                  asynchronous: true
                  cache: true
                  visible: modelData.thumbnailPath && modelData.thumbnailPath.length > 0
                }

                // Placeholder
                Text {
                  anchors.centerIn: parent
                  text: "\uD83C\uDFA5"
                  font.pixelSize: 28
                  color: tm ? tm.mutedText : BlueTheme.mutedText
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

                // Badge qualité
                Rectangle {
                  visible: modelData.quality && modelData.quality.length > 0
                  anchors.right: parent.right
                  anchors.top: parent.top
                  anchors.margins: 6
                  width: qualityBadgeText.width + 10
                  height: 18
                  radius: 4
                  color: "#CC000000"

                  Text {
                    id: qualityBadgeText
                    anchors.centerIn: parent
                    text: BlueTheme.formatQuality(modelData.quality)
                    font.family: BlueTheme.fontFamily
                    font.pixelSize: 10
                    font.bold: true
                    color: "#FFFFFF"
                  }
                }

                // Checkbox sélection
                Rectangle {
                  visible: viewModel.selectionMode
                  anchors.left: parent.left
                  anchors.top: parent.top
                  anchors.margins: 6
                  width: 22
                  height: 22
                  radius: 11
                  property bool isChecked: viewModel.selectedCount >= 0 && viewModel.isSelected(modelData.id)
                  color: isChecked ? (tm ? tm.accent : BlueTheme.accent) : (tm ? tm.surface : BlueTheme.surface)
                  border.color: isChecked ? (tm ? tm.accent : BlueTheme.accent) : (tm ? tm.divider : BlueTheme.divider)
                  border.width: 2

                  Text {
                    anchors.centerIn: parent
                    text: "\u2713"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#FFFFFF"
                    visible: parent.isChecked
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
                    NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
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
                    width: parent.width * Math.min(1, modelData.watchPosition / Math.max(1, modelData.duration))
                    height: parent.height
                    color: tm ? tm.accent : BlueTheme.accent
                  }
                }
              }

              // Info section
              ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                  text: modelData.streamerName || qsTr("Unknown")
                  font.family: BlueTheme.fontFamily
                  font.pixelSize: 13
                  font.bold: true
                  color: tm ? tm.primaryText : BlueTheme.primaryText
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                }

                Text {
                  text: modelData.streamTitle || qsTr("Untitled")
                  font.family: BlueTheme.fontFamily
                  font.pixelSize: 11
                  color: tm ? tm.secondaryText : BlueTheme.secondaryText
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                }

                RowLayout {
                  Layout.fillWidth: true
                  spacing: 6

                  Text {
                    text: modelData.recordedAtFormatted || ""
                    font.family: BlueTheme.fontFamily
                    font.pixelSize: 10
                    color: tm ? tm.mutedText : BlueTheme.mutedText
                  }

                  Text {
                    text: "·"
                    font.pixelSize: 10
                    color: tm ? tm.mutedText : BlueTheme.mutedText
                  }

                  Text {
                    text: modelData.fileSizeFormatted || ""
                    font.family: BlueTheme.fontFamily
                    font.pixelSize: 10
                    color: tm ? tm.mutedText : BlueTheme.mutedText
                  }
                }
                
                // "Vu il y a X jours"
                Text {
                  visible: modelData.lastWatchedFormatted && modelData.lastWatchedFormatted.length > 0
                  text: modelData.lastWatchedFormatted || ""
                  font.family: BlueTheme.fontFamily
                  font.pixelSize: 10
                  font.italic: true
                  color: modelData.isCompleted ? (tm ? tm.accent : BlueTheme.accent) : (tm ? tm.secondaryText : BlueTheme.secondaryText)
                  Layout.fillWidth: true
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
                  console.info("[Cache] VOD played:", modelData.streamerName)
                  root.playVodRequested(modelData.id, modelData.filePath, modelData)
                }
              }
            }

            Behavior on color {
              ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
          }
        }
      }

      // État vide (dans le Flow)
      Item {
        width: vodFlowLayout.width
        height: 200
        visible: viewModel.vodCount === 0

        ColumnLayout {
          anchors.centerIn: parent
          spacing: BlueTheme.spacingMedium

          Text {
            Layout.alignment: Qt.AlignHCenter
            text: "\uD83D\uDCF9"
            font.pixelSize: 48
            opacity: 0.5
          }

          Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("No replays")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 16
            font.bold: true
            color: tm ? tm.secondaryText : BlueTheme.secondaryText
          }

          Text {
          Layout.alignment: Qt.AlignHCenter
          text: qsTr("Watched streams will appear here")
          font.family: BlueTheme.fontFamily
          font.pixelSize: 13
          color: tm ? tm.mutedText : BlueTheme.mutedText
          horizontalAlignment: Text.AlignHCenter
        }
      }
    }
  }  // Flow
  }  // Item wrapper
}  // Flickable

  // Dialog suppression sélection
  Popup {
    id: deleteSelectedDialog
    modal: true
    anchors.centerIn: parent
    width: 320
    padding: 24

    background: Rectangle {
      radius: 16
      color: tm ? tm.surface : BlueTheme.surface
      border.color: tm ? tm.divider : BlueTheme.divider
      border.width: 1
    }

    ColumnLayout {
      anchors.fill: parent
      spacing: 16

      Text {
        text: qsTr("Delete %1 replays?").arg(viewModel.selectedCount)
        font.family: BlueTheme.fontFamily
        font.pixelSize: 16
        font.bold: true
        color: tm ? tm.primaryText : BlueTheme.primaryText
      }

      Text {
        text: qsTr("This action cannot be undone.")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: tm ? tm.secondaryText : BlueTheme.secondaryText
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
          border.color: tm ? tm.divider : BlueTheme.divider
          border.width: 1

          Text {
            id: cancelText
            anchors.centerIn: parent
            text: qsTr("Cancel")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: tm ? tm.primaryText : BlueTheme.primaryText
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
          color: tm ? tm.statusNegative : BlueTheme.statusNegative

          Text {
            id: confirmDelText
            anchors.centerIn: parent
            text: qsTr("Delete")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: "#FFFFFF"
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              console.info("[Cache] VODs deleted:", viewModel.selectedCount)
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
      color: tm ? tm.surface : BlueTheme.surface
      border.color: tm ? tm.divider : BlueTheme.divider
      border.width: 1
    }

    ColumnLayout {
      anchors.fill: parent
      spacing: 16

      Text {
        text: qsTr("Clear all cache?")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 16
        font.bold: true
        color: tm ? tm.primaryText : BlueTheme.primaryText
      }

      Text {
        text: qsTr("%1 replays will be deleted.\nSpace freed: %2")
                .arg(viewModel.vodCount)
                .arg(viewModel.totalSizeFormatted)
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: tm ? tm.secondaryText : BlueTheme.secondaryText
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
          border.color: tm ? tm.divider : BlueTheme.divider
          border.width: 1

          Text {
            id: cancelAllText
            anchors.centerIn: parent
            text: qsTr("Cancel")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: tm ? tm.primaryText : BlueTheme.primaryText
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
          color: tm ? tm.statusNegative : BlueTheme.statusNegative

          Text {
            id: confirmClearText
            anchors.centerIn: parent
            text: qsTr("Clear")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            color: "#FFFFFF"
          }

          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              console.info("[Cache] All VODs cleared:", viewModel.vodCount, "videos")
              viewModel.clearAll()
              clearAllDialog.close()
            }
          }
        }
      }
    }
  }
}
