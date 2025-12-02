#!/usr/bin/env bash
set -e

BUILD_DIR="${1:-/Users/user/Projects/BluePlayer/build}"
APP_PATH="${2:-$BUILD_DIR/src/BluePlayer.app}"
OUTPUT_PATH="${3:-$HOME/Desktop/BluePlayer_HomeView.png}"

echo "Building BluePlayer..."
cmake -S /Users/user/Projects/BluePlayer -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DIR" -j4

if [ ! -d "$APP_PATH" ]; then
    echo "Error: Application not found at $APP_PATH"
    exit 1
fi

echo "Launching BluePlayer..."
open "$APP_PATH"

echo "Waiting for app to start..."
sleep 3

echo "Capturing screenshot..."
echo "Note: If interactive capture fails, the app window should be visible."
echo "You can manually capture it using: screencapture -x -W $OUTPUT_PATH"

# Try interactive window capture first
screencapture -x -W "$OUTPUT_PATH" 2>/dev/null || {
    echo "Interactive capture not available, trying full screen capture..."
    screencapture -x "$OUTPUT_PATH" 2>/dev/null || {
        echo "Warning: Automatic screenshot capture failed."
        echo "Please manually capture the BluePlayer window using:"
        echo "  screencapture -x -W $OUTPUT_PATH"
        echo ""
        echo "Or use Cmd+Shift+4 and click on the BluePlayer window."
    }
}

if [ -f "$OUTPUT_PATH" ]; then
    echo "Screenshot saved to: $OUTPUT_PATH"
    open "$OUTPUT_PATH" 2>/dev/null || true
fi

