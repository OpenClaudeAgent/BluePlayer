#!/bin/bash

# Complete investigation script that runs app and curl tests

set -e

echo "=== BluePlayer Twitch GraphQL Investigation ==="
echo ""

# Step 1: Check environment
echo "Step 1: Checking environment..."
if [ -z "$TWITCH_CLIENT_ID" ]; then
    if [ -f .env ]; then
        export $(cat .env | grep -v '^#' | xargs)
    fi
fi

if [ -z "$TWITCH_CLIENT_ID" ]; then
    echo "ERROR: TWITCH_CLIENT_ID not found"
    exit 1
fi

echo "✓ Client-ID: $TWITCH_CLIENT_ID"
echo ""

# Step 2: Run app and capture logs
echo "Step 2: Running app to capture GraphQL request..."
echo "NOTE: You need to click on a stream in the app UI to trigger GraphQL request"
echo "App will run for 20 seconds..."
echo ""

BLUEPLAYER_LOG_NETWORK=debug BLUEPLAYER_LOG_TWITCH=debug make run > /tmp/blueplayer_full.log 2>&1 &
APP_PID=$!

sleep 20
kill $APP_PID 2>/dev/null || true
wait $APP_PID 2>/dev/null || true

echo "✓ App logs captured"
echo ""

# Step 3: Extract GraphQL errors
echo "Step 3: Analyzing GraphQL errors..."
if grep -q "Client-ID header is invalid" /tmp/blueplayer_full.log; then
    echo "✓ Found 'Client-ID header is invalid' error"
    echo ""
    echo "Error details:"
    grep -A 5 "Client-ID header is invalid" /tmp/blueplayer_full.log | head -10
    echo ""
elif grep -q "getPlaybackAccessToken" /tmp/blueplayer_full.log; then
    echo "✓ Found GraphQL request in logs"
    echo ""
    echo "Request details:"
    grep -A 3 "getPlaybackAccessToken" /tmp/blueplayer_full.log | head -10
    echo ""
else
    echo "⚠ No GraphQL request found in logs"
    echo "  This means no stream was clicked during the test"
    echo "  Please click on a stream manually to trigger the request"
    echo ""
fi

# Step 4: Show header investigation results
echo "Step 4: Header investigation results..."
echo ""
echo "Headers being set (from logs):"
grep "RAW HEADER BYTES INVESTIGATION" -A 5 /tmp/blueplayer_full.log | grep "Client-ID" | head -3
echo ""
echo "Headers actually sent (from rawHeaderList):"
grep "Final request headers" /tmp/blueplayer_full.log | grep "client-id" | head -1
echo ""

# Step 5: Instructions for curl test
echo "Step 5: To run curl test manually:"
echo ""
echo "1. Get your OAuth token (from app's secure storage or browser dev tools)"
echo "2. Run: export TWITCH_ACCESS_TOKEN='your_token_here'"
echo "3. Run: ./test/twitch_graphql_test.sh kamet0"
echo ""
echo "This will test if Twitch GraphQL accepts lowercase 'client-id' header"
echo ""

echo "=== Investigation Complete ==="
echo "Full logs saved to: /tmp/blueplayer_full.log"




