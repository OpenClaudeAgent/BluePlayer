/**
 * @file main.cpp
 * @brief E2E Test Runner for BluePlayer
 * 
 * This is the entry point for Qt Quick Tests.
 * It uses QUICK_TEST_MAIN_WITH_SETUP to:
 * 1. Start mock servers (via Setup class)
 * 2. Load QML test files (tst_*.qml)
 * 3. Execute tests with visible UI
 * 4. Clean up mock servers
 * 
 * Run with: ./e2e_tests
 * 
 * Test files are discovered automatically from the source directory.
 * Each tst_*.qml file should contain TestCase components.
 */

#include <QtQuickTest/quicktest.h>
#include "Setup.hpp"

// QUICK_TEST_MAIN_WITH_SETUP(name, SetupClass)
// - "BluePlayerE2E" is the test suite name
// - Setup is our class that starts mock servers
QUICK_TEST_MAIN_WITH_SETUP(BluePlayerE2E, Setup)
