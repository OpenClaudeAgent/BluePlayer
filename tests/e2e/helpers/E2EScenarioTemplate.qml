import QtQuick

/**
 * E2EScenarioTemplate - Standard wrapper for E2E test scenarios
 * 
 * Encapsulates the boilerplate:
 * - Root Item with standard dimensions
 * - Loader for main.qml
 * - Ready signal when app is loaded
 * 
 * Usage:
 *   E2EScenarioTemplate {
 *       E2ETestCase {
 *           name: "MyTest"
 *           when: windowShown && appReady
 *           
 *           function initTestCase() {
 *               mainWindow = app  // Use 'app' alias
 *           }
 *           
 *           function test_something() { ... }
 *       }
 *   }
 */
Item {
    id: root
    
    // Standard E2E test dimensions
    width: 1280
    height: 720
    
    // Expose the loaded app to children
    readonly property alias app: appLoader.item
    
    // Signal when app is ready
    readonly property bool appReady: appLoader.status === Loader.Ready && appLoader.item !== null
    
    // Expose the loader status for 'when' conditions
    readonly property alias loaderStatus: appLoader.status
    
    // QML path from environment (set by C++ setup)
    readonly property string qmlPath: typeof E2E_QML_PATH !== "undefined" ? E2E_QML_PATH : ""
    
    Loader {
        id: appLoader
        anchors.fill: parent
        source: root.qmlPath.length > 0 ? "file://" + root.qmlPath + "/main.qml" : ""
        asynchronous: false
        
        onStatusChanged: {
            if (status === Loader.Error) {
                console.error("E2EScenarioTemplate: Failed to load main.qml from " + source)
            } else if (status === Loader.Ready) {
                console.log("E2EScenarioTemplate: App loaded successfully")
            }
        }
    }
    
    // Pass appReady to children via context
    onAppReadyChanged: {
        if (appReady) {
            console.log("E2EScenarioTemplate: App ready, mainWindow available")
        }
    }
}
