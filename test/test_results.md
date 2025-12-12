# Test Results: Twitch GraphQL Client-ID Investigation

## App Execution Results

### Logs Captured
- **Log file**: `/tmp/blueplayer_output.log`
- **Total lines**: 567
- **Duration**: ~15 seconds

### Key Observations

1. **Headers Set Correctly in Code**:
   ```
   Default header bytes - Name: 'Client-ID' (9 bytes), Value: 'kpt3f7w9eh9y4kg53g0zbe0grqhabm' (30 bytes)
   ```
   - Code correctly sets `Client-ID` with exact case
   - Value is correct: `kpt3f7w9eh9y4kg53g0zbe0grqhabm`

2. **Headers Normalized by Qt**:
   ```
   Final request headers (3): authorization: Bearer ***; client-id: kpt3f7w9eh9y4kg53g0zbe0grqhabm; accept: application/json
   ```
   - `rawHeaderList()` shows headers in lowercase: `client-id`
   - This confirms Qt normalizes headers during transmission

3. **No GraphQL Request Triggered**:
   - No `getPlaybackAccessToken` calls found in logs
   - No `gql.twitch.tv` requests found
   - **Reason**: User needs to click on a stream in the UI to trigger GraphQL request

4. **Helix API Calls Working**:
   - Multiple successful Helix API calls observed:
     - `/helix/clips` - Status 200
     - `/helix/users` - Status 200
   - All use same header mechanism (default headers)
   - All show lowercase `client-id` in `rawHeaderList()`
   - **Conclusion**: Helix API accepts lowercase headers

## Next Steps

### To Complete Investigation:

1. **Trigger GraphQL Request**:
   - Run the app
   - Click on a stream card in the UI
   - This will trigger `getPlaybackAccessToken()` and the GraphQL request
   - Capture the error response

2. **Run curl Test**:
   ```bash
   # Get OAuth token (from app secure storage or browser)
   export TWITCH_ACCESS_TOKEN='your_token_here'
   
   # Run test
   ./test/twitch_graphql_test.sh kamet0
   ```
   
   This will test:
   - `Client-ID` with exact case
   - `client-id` in lowercase  
   - No Client-ID header (should fail)

3. **Expected Results**:
   - If curl with `Client-ID` works → Qt header normalization is the issue
   - If curl with `client-id` works → Issue is something else (token, scopes, etc.)
   - If both fail → Client-ID or token mismatch issue

## Current Status

✅ **Completed**:
- Enhanced logging added to HttpClient
- Header bytes investigation logging
- curl test script created
- App runs successfully
- Helix API calls work with lowercase headers

⏳ **Pending**:
- GraphQL request needs to be triggered (user interaction required)
- curl test needs OAuth token (requires user to provide or extract from secure storage)

## Findings Summary

1. **Qt normalizes HTTP headers to lowercase** - Confirmed by `rawHeaderList()` output
2. **Helix API accepts lowercase headers** - All Helix calls return 200 OK
3. **GraphQL API behavior unknown** - Need to trigger request to see error
4. **Code sets headers correctly** - `Client-ID` with exact case in code
5. **Token masking works** - Authorization header properly masked in logs

## Recommendations

Based on current findings:

1. **If GraphQL rejects lowercase headers**: Use libcurl for GraphQL requests only
2. **If GraphQL accepts lowercase headers**: Investigate other causes (token scopes, Client-ID format, etc.)
3. **Alternative**: Research if Helix API provides stream URLs directly (avoid GraphQL)









