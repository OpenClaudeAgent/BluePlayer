#!/bin/bash

# Test script to verify Twitch GraphQL API Client-ID header requirements
# This script tests if the Client-ID header works when sent manually with exact case

set -e

# Load environment variables
if [ -f .env ]; then
    export $(cat .env | grep -v '^#' | xargs)
fi

# Check required variables
if [ -z "$TWITCH_CLIENT_ID" ]; then
    echo "ERROR: TWITCH_CLIENT_ID environment variable is not set"
    exit 1
fi

if [ -z "$TWITCH_ACCESS_TOKEN" ]; then
    echo "WARNING: TWITCH_ACCESS_TOKEN environment variable is not set"
    echo "Attempting to extract from app logs..."
    # Try to extract token from recent app run
    TOKEN_FROM_LOG=$(grep -o "Bearer [a-zA-Z0-9_-]*" /tmp/blueplayer_output.log 2>/dev/null | head -1 | cut -d' ' -f2)
    if [ -n "$TOKEN_FROM_LOG" ]; then
        export TWITCH_ACCESS_TOKEN="$TOKEN_FROM_LOG"
        echo "Found token in logs (first 10 chars): ${TOKEN_FROM_LOG:0:10}..."
    else
        echo "ERROR: Could not find token. Please set TWITCH_ACCESS_TOKEN manually"
        echo "Or run the app and click on a stream to generate logs with the token"
        exit 1
    fi
fi

STREAMER_LOGIN="${1:-kamet0}"  # Default to kamet0 if not provided

echo "=== Twitch GraphQL API Test ==="
echo "Client-ID: $TWITCH_CLIENT_ID"
echo "Streamer Login: $STREAMER_LOGIN"
echo ""

# GraphQL query for PlaybackAccessToken
GRAPHQL_QUERY=$(cat <<EOF
{
  "query": "query PlaybackAccessToken(\$login: String!) { streamPlaybackAccessToken(channelName: \$login, params: {platform: \"web\", playerBackend: \"mediaplayer\", playerType: \"site\"}) { value signature } }",
  "variables": {
    "login": "$STREAMER_LOGIN"
  }
}
EOF
)

echo "=== Test 1: Client-ID with exact case (Client-ID) ==="
RESPONSE1=$(curl -s -w "\nHTTP_CODE:%{http_code}" \
  -X POST \
  -H "Content-Type: application/json" \
  -H "Client-ID: $TWITCH_CLIENT_ID" \
  -H "Authorization: Bearer $TWITCH_ACCESS_TOKEN" \
  -d "$GRAPHQL_QUERY" \
  "https://gql.twitch.tv/gql")

HTTP_CODE1=$(echo "$RESPONSE1" | grep "HTTP_CODE" | cut -d: -f2)
BODY1=$(echo "$RESPONSE1" | sed '/HTTP_CODE/d')

echo "HTTP Status: $HTTP_CODE1"
echo "Response: $BODY1"
echo ""

if [ "$HTTP_CODE1" = "200" ]; then
    echo "✓ SUCCESS: Client-ID with exact case works!"
    exit 0
fi

echo "=== Test 2: Client-ID in lowercase (client-id) ==="
RESPONSE2=$(curl -s -w "\nHTTP_CODE:%{http_code}" \
  -X POST \
  -H "Content-Type: application/json" \
  -H "client-id: $TWITCH_CLIENT_ID" \
  -H "Authorization: Bearer $TWITCH_ACCESS_TOKEN" \
  -d "$GRAPHQL_QUERY" \
  "https://gql.twitch.tv/gql")

HTTP_CODE2=$(echo "$RESPONSE2" | grep "HTTP_CODE" | cut -d: -f2)
BODY2=$(echo "$RESPONSE2" | sed '/HTTP_CODE/d')

echo "HTTP Status: $HTTP_CODE2"
echo "Response: $BODY2"
echo ""

echo "=== Test 3: No Client-ID header (should fail) ==="
RESPONSE3=$(curl -s -w "\nHTTP_CODE:%{http_code}" \
  -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TWITCH_ACCESS_TOKEN" \
  -d "$GRAPHQL_QUERY" \
  "https://gql.twitch.tv/gql")

HTTP_CODE3=$(echo "$RESPONSE3" | grep "HTTP_CODE" | cut -d: -f2)
BODY3=$(echo "$RESPONSE3" | sed '/HTTP_CODE/d')

echo "HTTP Status: $HTTP_CODE3"
echo "Response: $BODY3"
echo ""

echo "=== Summary ==="
echo "Test 1 (Client-ID): HTTP $HTTP_CODE1"
echo "Test 2 (client-id): HTTP $HTTP_CODE2"
echo "Test 3 (no header): HTTP $HTTP_CODE3"

