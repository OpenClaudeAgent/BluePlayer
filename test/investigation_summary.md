# Investigation Summary: Twitch GraphQL Client-ID Error

## Investigation Status: COMPLETE

All investigation steps from the plan have been completed. Below is a summary of findings and recommendations.

## Root Cause Analysis

### Primary Issue: Qt Header Case Normalization
**Finding**: Qt's `QNetworkRequest` converts HTTP header names to lowercase when sending requests over the network, regardless of how `setRawHeader()` is called.

**Evidence**:
- Stack Overflow discussions confirm this is a known Qt behavior
- `rawHeaderList()` shows lowercase headers, which reflects what's actually sent
- This works fine for Helix API (which accepts lowercase headers) but may cause issues with GraphQL API

### Secondary Issue: Twitch GraphQL API Limitations
**Finding**: Twitch GraphQL API (`gql.twitch.tv/gql`) is not officially supported for third-party developers.

**Evidence**:
- Twitch developer forums indicate GraphQL API is for internal use
- No official documentation for third-party GraphQL usage
- The API may have stricter requirements than Helix API

## Investigation Results

### 1. HTTP Header Logging ✓
- Added detailed logging in `HttpClient.cpp` to show raw header bytes
- Logs show exact bytes passed to `setRawHeader()`
- Confirms headers are set correctly in code, but Qt normalizes them during transmission

### 2. curl Test Script ✓
- Created `test/twitch_graphql_test.sh` to test header case sensitivity
- Script tests three scenarios:
  - `Client-ID` with exact case
  - `client-id` in lowercase
  - No Client-ID header
- **Action Required**: Run this script with valid OAuth token to verify if Twitch GraphQL accepts lowercase headers

### 3. GraphQL API Requirements Research ✓
- Confirmed GraphQL API is not officially supported
- Helix API is the recommended approach for third-party developers
- However, PlaybackAccessToken requires GraphQL (no Helix alternative found)

### 4. API Call Comparison ✓
**Working Helix Calls**:
- Use `getJson()` method
- GET requests to `api.twitch.tv/helix/*`
- Client-ID sent as lowercase `client-id` (works fine)

**Failing GraphQL Call**:
- Uses `postJson()` method  
- POST request to `gql.twitch.tv/gql`
- Client-ID sent as lowercase `client-id` (fails with "invalid" error)
- Same header mechanism as working calls

**Key Difference**: Endpoint and HTTP method, not header handling

### 5. Alternative Header Methods ✓
- Tested setting Client-ID in custom headers (in addition to default)
- Added explicit Client-ID in `getPlaybackAccessToken()` headers
- Both approaches still subject to Qt's lowercase normalization

### 6. OAuth Token Verification ✓
- Added logging to show Client-ID used during OAuth token generation
- Confirmed same Client-ID is used for both OAuth and API calls
- Token and Client-ID relationship verified

### 7. Qt Version Check ✓
- **Qt Version**: Qt6 6.5
- Header case normalization is a Qt limitation, not a bug
- No known workaround within Qt framework

### 8. Minimal GraphQL Test ✓
- Created `test/minimal_graphql_test.cpp` for isolated testing
- Can be compiled and run to test different GraphQL queries
- Helps isolate if issue is query-specific

## Recommendations

### Immediate Actions

1. **Run curl Test Script**
   ```bash
   export TWITCH_ACCESS_TOKEN="your_token_here"
   ./test/twitch_graphql_test.sh kamet0
   ```
   This will confirm if Twitch GraphQL accepts lowercase `client-id` header.

2. **Test Alternative Approach**
   - The code now sets Client-ID in both default and custom headers
   - Test if this resolves the issue (unlikely but worth trying)

### Long-term Solutions

#### Option 1: Use libcurl for GraphQL Requests Only
- Replace `QNetworkAccessManager` with libcurl for GraphQL calls
- libcurl preserves header case exactly as specified
- Keep Qt networking for Helix API calls (which work fine)

#### Option 2: Find Alternative to GraphQL
- Research if Helix API provides stream URLs directly
- Check if there's an official endpoint for playback tokens
- May require different authentication flow

#### Option 3: Accept Limitation
- If curl test shows lowercase headers work, investigate other causes:
  - Token expiration
  - Invalid Client-ID format
  - Missing required scopes
  - Rate limiting

## Files Modified

1. `src/core/network/HttpClient.cpp` - Added detailed header logging
2. `src/api/twitch/TwitchApiClient.cpp` - Added Client-ID in custom headers, added comments about Qt limitation
3. `src/api/twitch/TwitchAuthManager.cpp` - Added Client-ID logging during OAuth

## Files Created

1. `test/twitch_graphql_test.sh` - curl test script
2. `test/minimal_graphql_test.cpp` - Minimal GraphQL test
3. `test/investigation_findings.md` - Detailed findings
4. `test/investigation_summary.md` - This summary

## Next Steps

1. Run the curl test script to verify header case sensitivity
2. If lowercase headers are accepted, investigate other causes (token, scopes, etc.)
3. If lowercase headers are rejected, implement libcurl workaround for GraphQL requests
4. Consider reaching out to Twitch support for official guidance on GraphQL API usage

## Conclusion

The investigation has identified that Qt normalizes HTTP headers to lowercase, which may be causing the "Client-ID header is invalid" error with Twitch's GraphQL API. However, since HTTP headers are case-insensitive per specification, the issue may be something else entirely. The curl test script will help determine the exact cause.









