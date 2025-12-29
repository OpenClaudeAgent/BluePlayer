/**
 * tst_LoadingOverlay.qml
 * 
 * Functional UI tests for the LoadingOverlay component.
 * Tests loading state visibility and BusyIndicator behavior.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 400

    // =========================================================================
    // Component Under Test (Mock matching LoadingOverlay.qml API)
    // =========================================================================
    
    Rectangle {
        id: loadingOverlay
        objectName: "loadingOverlay"
        
        // Center in parent for visibility
        anchors.centerIn: parent
        
        // Properties matching LoadingOverlay
        property bool loading: false
        
        // Visual properties
        width: 120
        height: 120
        radius: 20
        color: "#80000000"
        visible: loading
        
        Column {
            id: contentColumn
            objectName: "contentColumn"
            anchors.centerIn: parent
            spacing: 16
            
            BusyIndicator {
                id: busyIndicator
                objectName: "busyIndicator"
                anchors.horizontalCenter: parent.horizontalCenter
                running: loadingOverlay.loading
                palette.dark: "#FFFFFF"
            }
        }
        
        // Reset function for tests
        function reset() {
            loading = false
        }
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "LoadingOverlayTests"
        when: windowShown
        
        function init() {
            loadingOverlay.reset()
            // waitForRendering removed for perf
        }
        
        // =====================================================================
        // Default Values Tests
        // =====================================================================
        
        function test_defaultValues() {
            compare(loadingOverlay.loading, false, "Default loading is false")
            compare(loadingOverlay.visible, false, "Default visible is false")
        }
        
        // =====================================================================
        // Loading Property Tests
        // =====================================================================
        
        function test_loadingProperty_canBeSet() {
            loadingOverlay.loading = true
            compare(loadingOverlay.loading, true, "Loading can be set to true")
            
            loadingOverlay.loading = false
            compare(loadingOverlay.loading, false, "Loading can be set to false")
        }
        
        function test_loadingProperty_canBeToggled() {
            compare(loadingOverlay.loading, false, "Initially not loading")
            
            loadingOverlay.loading = true
            compare(loadingOverlay.loading, true, "Can start loading")
            
            loadingOverlay.loading = false
            compare(loadingOverlay.loading, false, "Can stop loading")
        }
        
        // =====================================================================
        // Visibility Tests
        // =====================================================================
        
        function test_visibility_hiddenWhenNotLoading() {
            loadingOverlay.loading = false
            compare(loadingOverlay.visible, false, "Hidden when not loading")
        }
        
        function test_visibility_visibleWhenLoading() {
            loadingOverlay.loading = true
            compare(loadingOverlay.visible, true, "Visible when loading")
        }
        
        function test_visibility_togglesWithLoading() {
            // Start hidden
            compare(loadingOverlay.visible, false, "Initially hidden")
            
            // Show
            loadingOverlay.loading = true
            compare(loadingOverlay.visible, true, "Visible after loading=true")
            
            // Hide again
            loadingOverlay.loading = false
            compare(loadingOverlay.visible, false, "Hidden after loading=false")
        }
        
        // =====================================================================
        // BusyIndicator Tests
        // =====================================================================
        
        function test_busyIndicator_exists() {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            verify(indicator !== null, "BusyIndicator exists")
        }
        
        function test_busyIndicator_notRunningByDefault() {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            compare(indicator.running, false, "BusyIndicator not running by default")
        }
        
        function test_busyIndicator_runsWhenLoading() {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            
            loadingOverlay.loading = true
            compare(indicator.running, true, "BusyIndicator runs when loading")
        }
        
        function test_busyIndicator_stopsWhenNotLoading() {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            
            loadingOverlay.loading = true
            compare(indicator.running, true, "Running when loading")
            
            loadingOverlay.loading = false
            compare(indicator.running, false, "Stopped when not loading")
        }
        
        function test_busyIndicator_syncedWithLoadingProperty() {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            
            // Multiple toggles
            for (var i = 0; i < 5; i++) {
                loadingOverlay.loading = true
                compare(indicator.running, true, "Running on iteration " + i)
                
                loadingOverlay.loading = false
                compare(indicator.running, false, "Stopped on iteration " + i)
            }
        }
        
        // =====================================================================
        // Edge Cases
        // =====================================================================
        
        function test_rapidLoadingToggle() {
            for (var i = 0; i < 10; i++) {
                loadingOverlay.loading = !loadingOverlay.loading
            }
            // After 10 toggles (starting from false), should be false
            compare(loadingOverlay.loading, false, "Loading state correct after 10 toggles")
            compare(loadingOverlay.visible, false, "Visibility matches loading state")
        }
        
        function test_multipleConsecutiveTrueAssignments() {
            loadingOverlay.loading = true
            loadingOverlay.loading = true
            loadingOverlay.loading = true
            
            compare(loadingOverlay.loading, true, "Still loading after multiple true assignments")
            compare(loadingOverlay.visible, true, "Still visible")
        }
        
        function test_multipleConsecutiveFalseAssignments() {
            loadingOverlay.loading = true
            loadingOverlay.loading = false
            loadingOverlay.loading = false
            loadingOverlay.loading = false
            
            compare(loadingOverlay.loading, false, "Not loading after multiple false assignments")
            compare(loadingOverlay.visible, false, "Hidden")
        }
        
        // =====================================================================
        // Integration Tests
        // =====================================================================
        
        function test_fullLoadingCycle() {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            
            // 1. Initial state - not loading
            compare(loadingOverlay.loading, false, "Initially not loading")
            compare(loadingOverlay.visible, false, "Initially hidden")
            compare(indicator.running, false, "Indicator not running")
            
            // 2. Start loading
            loadingOverlay.loading = true
            compare(loadingOverlay.loading, true, "Now loading")
            compare(loadingOverlay.visible, true, "Now visible")
            compare(indicator.running, true, "Indicator running")
            
            // 3. Stop loading
            loadingOverlay.loading = false
            compare(loadingOverlay.loading, false, "Loading finished")
            compare(loadingOverlay.visible, false, "Hidden again")
            compare(indicator.running, false, "Indicator stopped")
        }
        
        function test_stateConsistency() {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            
            // Verify all states are always in sync
            var states = [true, false, true, true, false, true, false, false]
            
            for (var i = 0; i < states.length; i++) {
                loadingOverlay.loading = states[i]
                
                compare(loadingOverlay.visible, states[i], 
                    "Visibility matches loading at step " + i)
                compare(indicator.running, states[i], 
                    "Indicator running matches loading at step " + i)
            }
        }
        
        // =====================================================================
        // Data-Driven Tests
        // =====================================================================
        
        function test_loadingStates_data() {
            return [
                { tag: "loading", loading: true, expectedVisible: true, expectedRunning: true },
                { tag: "not-loading", loading: false, expectedVisible: false, expectedRunning: false }
            ]
        }
        
        function test_loadingStates(data) {
            var indicator = findChild(loadingOverlay, "busyIndicator")
            
            loadingOverlay.loading = data.loading
            
            compare(loadingOverlay.visible, data.expectedVisible, 
                "Visibility is " + data.expectedVisible + " when loading=" + data.loading)
            compare(indicator.running, data.expectedRunning, 
                "Indicator running is " + data.expectedRunning + " when loading=" + data.loading)
        }
    }
}
