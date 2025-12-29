/**
 * tst_PanelHeader.qml
 * 
 * Functional UI tests for the PanelHeader component.
 * Tests title display, close button visibility, backClicked signal, and default state.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 500
    height: 300

    // Detect offscreen mode - mouse events may not work
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Component Under Test (Mock of PanelHeader)
    // =========================================================================
    
    Component {
        id: panelHeaderComponent
        
        Item {
            id: panelHeader
            objectName: "panelHeader"
            
            // Theme constants (inline for testing)
            readonly property color themeSurface: "#1e1e2e"
            readonly property color themePrimaryText: "#cdd6f4"
            readonly property color themeDivider: "#45475a"
            readonly property color themeAccent: "#3b82f6"
            readonly property int themeSpacingLarge: 24
            readonly property int themeSpacingMedium: 16
            readonly property string themeFontFamily: "Inter"
            
            // ─────────────────────────────────────────────────────────────────
            // Public Properties
            // ─────────────────────────────────────────────────────────────────
            
            property string title: ""
            property bool showBackButton: true
            property color backgroundColor: Qt.rgba(themeSurface.r, themeSurface.g, themeSurface.b, 0.6)
            
            // ─────────────────────────────────────────────────────────────────
            // Signals
            // ─────────────────────────────────────────────────────────────────
            
            signal backClicked()
            
            // ─────────────────────────────────────────────────────────────────
            // Dimensions
            // ─────────────────────────────────────────────────────────────────
            
            implicitHeight: 64
            implicitWidth: parent ? parent.width : 400
            width: implicitWidth
            height: implicitHeight
            
            // ─────────────────────────────────────────────────────────────────
            // Background
            // ─────────────────────────────────────────────────────────────────
            
            Rectangle {
                id: headerBackground
                objectName: "headerBackground"
                anchors.fill: parent
                color: panelHeader.backgroundColor
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Title (aligned left)
            // ─────────────────────────────────────────────────────────────────
            
            Text {
                id: titleText
                objectName: "titleText"
                
                anchors {
                    left: parent.left
                    leftMargin: panelHeader.themeSpacingLarge
                    verticalCenter: parent.verticalCenter
                }
                
                text: panelHeader.title
                font.pixelSize: 20
                font.weight: Font.DemiBold
                font.family: panelHeader.themeFontFamily
                color: panelHeader.themePrimaryText
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Close Button (X) - Simplified mock of CircleButton
            // ─────────────────────────────────────────────────────────────────
            
            Rectangle {
                id: closeButton
                objectName: "closeButton"
                visible: panelHeader.showBackButton
                
                anchors {
                    right: parent.right
                    rightMargin: panelHeader.themeSpacingMedium
                    verticalCenter: parent.verticalCenter
                }
                
                width: 32
                height: 32
                radius: width / 2
                color: closeMouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
                
                readonly property bool hovered: closeMouseArea.containsMouse
                
                Text {
                    id: closeIcon
                    objectName: "closeIcon"
                    anchors.centerIn: parent
                    text: "\u2715"  // X symbol
                    font.pixelSize: 16
                    color: panelHeader.themePrimaryText
                }
                
                MouseArea {
                    id: closeMouseArea
                    objectName: "closeMouseArea"
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: panelHeader.backClicked()
                }
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Bottom Divider
            // ─────────────────────────────────────────────────────────────────
            
            Rectangle {
                id: bottomDivider
                objectName: "bottomDivider"
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                height: 1
                color: panelHeader.themeDivider
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var header: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: backClickedSpy; signalName: "backClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "PanelHeaderTests"
        when: windowShown

        function init() {
            header = createTemporaryObject(panelHeaderComponent, root)
            verify(header !== null, "PanelHeader should be created")
            header.anchors.centerIn = root
            backClickedSpy.target = header
            backClickedSpy.clear()
            // Move mouse away to reset hover state
            mouseMove(root, 1, 1)
        }

        function cleanup() {
            header = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_dimensions() {
            compare(header.implicitHeight, 64, "Default implicitHeight should be 64")
            verify(header.implicitWidth > 0, "implicitWidth should be positive")
        }
        
        function test_defaultState_emptyTitle() {
            compare(header.title, "", "Default title should be empty string")
        }
        
        function test_defaultState_showBackButtonTrue() {
            compare(header.showBackButton, true, "Default showBackButton should be true")
        }
        
        function test_defaultState_backgroundColorDefined() {
            verify(header.backgroundColor !== undefined, "backgroundColor should be defined")
            // Check it has some transparency (alpha < 1)
            compare(header.backgroundColor.a, 0.6, "backgroundColor should have 0.6 alpha")
        }

        // =====================================================================
        // TEST: Title Display
        // =====================================================================
        
        function test_title_displaysText() {
            var titleText = findChild(header, "titleText")
            verify(titleText !== null, "titleText should exist")
            
            header.title = "Preferences"
            
            compare(titleText.text, "Preferences", "Title text should match header.title")
        }
        
        function test_title_updatesWhenChanged() {
            var titleText = findChild(header, "titleText")
            
            header.title = "First Title"
            compare(titleText.text, "First Title")
            
            header.title = "Second Title"
            compare(titleText.text, "Second Title", "Title should update when property changes")
        }
        
        function test_title_emptyStringDisplayed() {
            var titleText = findChild(header, "titleText")
            
            header.title = ""
            
            compare(titleText.text, "", "Empty title should be displayed")
        }
        
        function test_title_hasCorrectFontSize() {
            var titleText = findChild(header, "titleText")
            
            compare(titleText.font.pixelSize, 20, "Title font size should be 20")
        }
        
        function test_title_isBold() {
            var titleText = findChild(header, "titleText")
            
            compare(titleText.font.weight, Font.DemiBold, "Title should use DemiBold weight")
        }
        
        function test_title_isLeftAligned() {
            var titleText = findChild(header, "titleText")
            
            // Title should be on the left side of the header
            verify(titleText.x < header.width / 2, "Title should be on the left side")
            compare(titleText.x, header.themeSpacingLarge, "Title left margin should match themeSpacingLarge")
        }
        
        function test_title_isVerticallyCentered() {
            var titleText = findChild(header, "titleText")
            
            // Check vertical centering (with some tolerance for font metrics)
            var expectedY = (header.height - titleText.height) / 2
            verify(Math.abs(titleText.y - expectedY) < 2, "Title should be vertically centered")
        }

        // =====================================================================
        // TEST: Close Button Visibility
        // =====================================================================
        
        function test_closeButton_visibleByDefault() {
            var closeButton = findChild(header, "closeButton")
            verify(closeButton !== null, "closeButton should exist")
            
            compare(closeButton.visible, true, "Close button should be visible by default")
        }
        
        function test_closeButton_hiddenWhenShowBackButtonFalse() {
            var closeButton = findChild(header, "closeButton")
            
            header.showBackButton = false
            
            compare(closeButton.visible, false, "Close button should be hidden when showBackButton is false")
        }
        
        function test_closeButton_togglesVisibility() {
            var closeButton = findChild(header, "closeButton")
            
            header.showBackButton = false
            compare(closeButton.visible, false)
            
            header.showBackButton = true
            compare(closeButton.visible, true, "Close button visibility should toggle with showBackButton")
        }
        
        function test_closeButton_isRightAligned() {
            var closeButton = findChild(header, "closeButton")
            
            // Button should be on the right side
            verify(closeButton.x > header.width / 2, "Close button should be on the right side")
            
            // Check right margin
            var expectedRight = header.width - header.themeSpacingMedium - closeButton.width
            compare(closeButton.x, expectedRight, "Close button should have correct right margin")
        }
        
        function test_closeButton_isVerticallyCentered() {
            var closeButton = findChild(header, "closeButton")
            
            var expectedY = (header.height - closeButton.height) / 2
            verify(Math.abs(closeButton.y - expectedY) < 1, "Close button should be vertically centered")
        }
        
        function test_closeButton_hasRoundShape() {
            var closeButton = findChild(header, "closeButton")
            
            compare(closeButton.radius, closeButton.width / 2, "Close button should be circular")
        }
        
        function test_closeButton_hasXIcon() {
            var closeIcon = findChild(header, "closeIcon")
            verify(closeIcon !== null, "Close icon should exist")
            
            compare(closeIcon.text, "\u2715", "Close button should display X symbol")
        }

        // =====================================================================
        // TEST: Close Button Click Signal
        // =====================================================================
        
        function test_closeButton_click_emitsBackClickedSignal() {
            var closeMouseArea = findChild(header, "closeMouseArea")
            verify(closeMouseArea !== null, "closeMouseArea should exist")
            
            mouseClick(closeMouseArea)
            
            compare(backClickedSpy.count, 1, "backClicked should be emitted once")
        }
        
        function test_closeButton_multipleClicks_emitMultipleSignals() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            
            var closeMouseArea = findChild(header, "closeMouseArea")
            
            mouseClick(closeMouseArea)
            mouseClick(closeMouseArea)
            mouseClick(closeMouseArea)
            
            compare(backClickedSpy.count, 3, "backClicked should be emitted for each click")
        }
        
        function test_closeButton_hidden_noSignalEmitted() {
            var closeMouseArea = findChild(header, "closeMouseArea")
            
            header.showBackButton = false
            
            // Even if we try to click, button is not visible
            mouseClick(closeMouseArea)
            
            compare(backClickedSpy.count, 0, "backClicked should not emit when button is hidden")
        }

        // =====================================================================
        // TEST: Close Button Hover State
        // =====================================================================
        
        function test_closeButton_hover_changesState() {
            var closeButton = findChild(header, "closeButton")
            var closeMouseArea = findChild(header, "closeMouseArea")
            
            compare(closeButton.hovered, false, "Initially not hovered")
            
            mouseMove(closeMouseArea, closeMouseArea.width / 2, closeMouseArea.height / 2)
            
            tryCompare(closeButton, "hovered", true, 100, "Should be hovered after mouseMove")
        }
        
        function test_closeButton_hoverExit_restoresState() {
            var closeButton = findChild(header, "closeButton")
            var closeMouseArea = findChild(header, "closeMouseArea")
            
            mouseMove(closeMouseArea, closeMouseArea.width / 2, closeMouseArea.height / 2)
            tryCompare(closeButton, "hovered", true, 100)
            
            mouseMove(root, 1, 1)
            
            tryCompare(closeButton, "hovered", false, 100, "Should not be hovered after mouse leaves")
        }

        // =====================================================================
        // TEST: Background
        // =====================================================================
        
        function test_background_exists() {
            var background = findChild(header, "headerBackground")
            verify(background !== null, "headerBackground should exist")
        }
        
        function test_background_fillsParent() {
            var background = findChild(header, "headerBackground")
            
            compare(background.width, header.width, "Background width should match header")
            compare(background.height, header.height, "Background height should match header")
        }
        
        function test_background_colorMatchesProperty() {
            var background = findChild(header, "headerBackground")
            
            compare(background.color, header.backgroundColor, "Background color should match backgroundColor property")
        }
        
        function test_background_customColor() {
            var background = findChild(header, "headerBackground")
            var customColor = Qt.rgba(1, 0, 0, 0.5)  // Semi-transparent red
            
            header.backgroundColor = customColor
            
            compare(background.color, customColor, "Background should use custom color")
        }

        // =====================================================================
        // TEST: Bottom Divider
        // =====================================================================
        
        function test_divider_exists() {
            var divider = findChild(header, "bottomDivider")
            verify(divider !== null, "bottomDivider should exist")
        }
        
        function test_divider_isAtBottom() {
            var divider = findChild(header, "bottomDivider")
            
            var expectedY = header.height - divider.height
            compare(divider.y, expectedY, "Divider should be at the bottom")
        }
        
        function test_divider_hasCorrectHeight() {
            var divider = findChild(header, "bottomDivider")
            
            compare(divider.height, 1, "Divider height should be 1px")
        }
        
        function test_divider_spansFullWidth() {
            var divider = findChild(header, "bottomDivider")
            
            compare(divider.width, header.width, "Divider should span full width")
            compare(divider.x, 0, "Divider should start at x=0")
        }
        
        function test_divider_hasThemeColor() {
            var divider = findChild(header, "bottomDivider")
            
            compare(divider.color, header.themeDivider, "Divider should use themeDivider color")
        }

        // =====================================================================
        // TEST: Theme Constants
        // =====================================================================
        
        function test_themeConstants_areDefined() {
            verify(header.themeSurface !== undefined, "themeSurface should be defined")
            verify(header.themePrimaryText !== undefined, "themePrimaryText should be defined")
            verify(header.themeDivider !== undefined, "themeDivider should be defined")
            verify(header.themeSpacingLarge > 0, "themeSpacingLarge should be positive")
            verify(header.themeSpacingMedium > 0, "themeSpacingMedium should be positive")
        }

        // =====================================================================
        // TEST: Layout Integrity
        // =====================================================================
        
        function test_layout_titleAndButtonDontOverlap() {
            var titleText = findChild(header, "titleText")
            var closeButton = findChild(header, "closeButton")
            
            header.title = "Very Long Title That Could Potentially Overlap"
            
            var titleRight = titleText.x + titleText.width
            var buttonLeft = closeButton.x
            
            verify(titleRight < buttonLeft, "Title and close button should not overlap")
        }
    }
}
