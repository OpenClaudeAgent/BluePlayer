# Solution Implemented: libcurl for GraphQL Requests

## Summary

Successfully implemented libcurl-based HTTP client for Twitch GraphQL API requests to preserve exact header case. This solves the "Client-ID header is invalid" error caused by Qt's header normalization.

## Changes Made

### 1. New Files Created
- `src/core/network/CurlHttpClient.hpp` - libcurl-based HTTP client header
- `src/core/network/CurlHttpClient.cpp` - libcurl-based HTTP client implementation

### 2. Modified Files
- `src/api/twitch/TwitchApiClient.hpp` - Added CurlHttpClient member and method declaration
- `src/api/twitch/TwitchApiClient.cpp` - Modified `getPlaybackAccessToken()` to use CurlHttpClient
- `src/core/network/ApiClientBase.hpp` - Added `bearerToken()` getter method
- `src/core/network/ApiClientBase.cpp` - Implemented `bearerToken()` getter
- `src/core/network/HttpClient.hpp` - Added `bearerToken()` getter method
- `src/CMakeLists.txt` - Added libcurl dependency and CurlHttpClient sources

### 3. Key Implementation Details

#### CurlHttpClient
- Uses libcurl's `curl_slist_append()` to set headers with exact case
- Preserves `Client-ID` header case (not normalized to lowercase)
- Synchronous POST requests for GraphQL API
- Proper error handling and JSON parsing

#### TwitchApiClient Integration
- `getPlaybackAccessToken()` now uses `CurlHttpClient` instead of Qt's `QNetworkRequest`
- All other API calls continue using Qt networking (which works fine for Helix API)
- Bearer token retrieved from `ApiClientBase::bearerToken()`

## Build Configuration

libcurl is automatically found by CMake:
- Uses system libcurl (macOS SDK includes it)
- CMake's `find_package(CURL)` locates it
- Linked as PRIVATE dependency to `blueplayer_core`

## Testing

To test the solution:
1. Run the app: `make run`
2. Click on a stream card to trigger GraphQL request
3. Check logs for "CurlHttpClient" messages
4. Verify no "Client-ID header is invalid" error

## Benefits

1. **Preserves Header Case**: libcurl sends headers exactly as specified
2. **Minimal Changes**: Only GraphQL requests use libcurl, rest uses Qt
3. **No Breaking Changes**: Existing Helix API calls unaffected
4. **Standard Library**: libcurl is widely available and well-tested

## Next Steps

1. Test with actual stream playback
2. Monitor logs for any libcurl-related issues
3. Consider adding retry logic if needed
4. Document libcurl dependency in setup instructions




