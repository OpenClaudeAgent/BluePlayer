# Investigation Findings: Twitch GraphQL Client-ID Error

## Key Findings

### 1. Qt Header Case Normalization Issue
- **Problem**: Qt's `QNetworkRequest` converts HTTP header names to lowercase when sending requests over the network
- **Evidence**: Stack Overflow discussions confirm this is a known Qt behavior
- **Impact**: Even though `setRawHeader("Client-ID", ...)` is used, Qt sends `client-id` (lowercase) over the wire
- **Reference**: https://stackoverflow.com/questions/79723789/qnetworkrequest-is-converting-header-names-to-lowercase-for-case-sensitive-servi

### 2. Twitch GraphQL API Limitations
- **Status**: Twitch GraphQL API (`gql.twitch.tv/gql`) is **not officially supported** for third-party developers
- **Purpose**: Intended for internal Twitch use
- **Issue**: The API may have stricter requirements or different behavior than Helix API
- **Reference**: https://discuss.dev.twitch.com/t/the-client-id-header-is-invalid-for-gql-twitch-tv/21845

### 3. Comparison: Working Helix vs Failing GraphQL

#### Working Helix API Calls
- `getStreams()` - Uses `getJson()` with default headers
- `getTopCategories()` - Uses `getJson()` with default headers  
- Both use GET requests to `https://api.twitch.tv/helix/*`
- Both successfully use `Client-ID` header (sent as `client-id` by Qt)

#### Failing GraphQL Call
- `getPlaybackAccessToken()` - Uses `postJson()` with default headers
- Uses POST request to `https://gql.twitch.tv/gql`
- Fails with "Client-ID header is invalid" error
- Same header mechanism as working calls

### 4. Differences Identified
1. **HTTP Method**: GET (Helix) vs POST (GraphQL)
2. **Endpoint**: `api.twitch.tv/helix/*` vs `gql.twitch.tv/gql`
3. **Content-Type**: Not explicitly set for GET, explicitly set to `application/json` for POST
4. **API Support**: Helix is officially supported, GraphQL is not

### 5. Qt Version
- **Version**: Qt6 6.5 (from CMakeLists.txt)
- **Known Issue**: Header case normalization is a Qt limitation, not a bug

## Potential Solutions

### Solution 1: Use Alternative Method to Get HLS URL
- Research if Helix API provides an alternative endpoint for stream URLs
- Check if there's an official way to get playback tokens via Helix

### Solution 2: Workaround for Qt Header Case Issue
- Use a custom HTTP library (libcurl) for GraphQL requests only
- Intercept QNetworkRequest before sending and modify headers
- Use QNetworkAccessManager's `createRequest()` override (if available)

### Solution 3: Verify Header Case Sensitivity
- Test with curl script to confirm if Twitch GraphQL accepts lowercase `client-id`
- If accepted, the issue may be something else (token mismatch, invalid Client-ID, etc.)

## Next Steps
1. Run curl test script to verify header case sensitivity
2. Research alternative methods to get HLS stream URLs
3. Test if explicitly setting Client-ID in custom headers (not default) helps
4. Consider using libcurl for GraphQL requests if Qt limitation confirmed


