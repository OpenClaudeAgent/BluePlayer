# Contract Testing Specification for BluePlayer

## Overview

This document defines the contract testing strategy for BluePlayer's external API integrations. Contract testing ensures that the application correctly handles API responses and gracefully degrades when contracts change.

## External API Contracts Identified

### 1. Twitch Helix API (REST)

| Endpoint | Method | Authentication | Usage Location |
|----------|--------|----------------|----------------|
| `/helix/streams` | GET | OAuth + Client-ID | `TwitchApiClient::listStreams()`, `getRecommendedStreams()` |
| `/helix/streams/followed` | GET | OAuth + Client-ID | `TwitchApiClient::listFollowedStreams()` |
| `/helix/users` | GET | OAuth + Client-ID | `TwitchApiClient::getUserInfo()`, `getUsersInfo()` |
| `/helix/games/top` | GET | OAuth + Client-ID | `TwitchApiClient::getTopCategories()` |
| `/helix/clips` | GET | OAuth + Client-ID | `TwitchApiClient::getPopularClips()`, `getFollowedClips()` |
| `/helix/videos` | GET | OAuth + Client-ID | `TwitchApiClient::getVideos()` |
| `/helix/channels/followed` | GET | OAuth + Client-ID | `TwitchApiClient::getFollowedChannels()` |
| `/helix/search/channels` | GET | OAuth + Client-ID | `TwitchApiClient::searchChannels()` |
| `/helix/search/categories` | GET | OAuth + Client-ID | `TwitchApiClient::searchCategories()` |

### 2. Twitch OAuth2 API

| Endpoint | Method | Usage Location |
|----------|--------|----------------|
| `/oauth2/authorize` | GET (Browser) | `TwitchAuthManager::login()` |
| `/oauth2/token` | POST | `TwitchAuthManager::requestAccessToken()`, `refresh()` |

### 3. Twitch GraphQL API (Unofficial)

| Operation | Usage Location |
|-----------|----------------|
| `PlaybackAccessToken` | `TwitchApiClient::getPlaybackAccessToken()` |

### 4. Twitch IRC/WebSocket (Chat)

| Protocol | Endpoint | Usage Location |
|----------|----------|----------------|
| WebSocket IRC | `wss://irc-ws.chat.twitch.tv:443` | `TwitchChatClient` |

### 5. HLS Playlist Format

| Format | Usage Location |
|--------|----------------|
| Master Playlist (M3U8) | `TwitchService::selectBestQualityFromPlaylist()` |
| Variant Playlist (M3U8) | `HlsAdFilter::detectAdsInPlaylist()` |

---

## Contract Definitions

### 1. Helix API Streams Contract

#### Request Schema
```json
{
  "endpoint": "https://api.twitch.tv/helix/streams",
  "method": "GET",
  "headers": {
    "Client-ID": "string (required)",
    "Authorization": "Bearer <token> (required)"
  },
  "queryParams": {
    "first": "integer (1-100, optional, default: 20)",
    "after": "string (pagination cursor, optional)",
    "user_id": "string[] (optional, max 100)",
    "user_login": "string[] (optional, max 100)",
    "game_id": "string[] (optional, max 100)",
    "language": "string[] (optional)"
  }
}
```

#### Response Schema (Success - 200)
```json
{
  "data": [
    {
      "id": "string (required)",
      "user_id": "string (required)",
      "user_login": "string (required)",
      "user_name": "string (required)",
      "game_id": "string (required)",
      "game_name": "string (required)",
      "type": "string (required, enum: 'live')",
      "title": "string (required)",
      "viewer_count": "integer (required)",
      "started_at": "string (required, ISO 8601)",
      "language": "string (required)",
      "thumbnail_url": "string (required, contains {width} {height})",
      "tag_ids": "string[] (deprecated, may be null)",
      "tags": "string[] (optional)",
      "is_mature": "boolean (required)"
    }
  ],
  "pagination": {
    "cursor": "string (optional)"
  }
}
```

#### Error Response Contracts
```json
// 400 Bad Request - Invalid Client-ID
{
  "error": "Bad Request",
  "status": 400,
  "message": "Invalid Client-ID header"
}

// 401 Unauthorized - Invalid/Expired Token
{
  "error": "Unauthorized",
  "status": 401,
  "message": "Invalid OAuth token"
}

// 429 Too Many Requests - Rate Limited
{
  "error": "Too Many Requests",
  "status": 429,
  "message": "Request limit exceeded"
}
// Headers: Ratelimit-Limit, Ratelimit-Remaining, Ratelimit-Reset
```

---

### 2. Helix API Users Contract

#### Request Schema
```json
{
  "endpoint": "https://api.twitch.tv/helix/users",
  "method": "GET",
  "headers": {
    "Client-ID": "string (required)",
    "Authorization": "Bearer <token> (required)"
  },
  "queryParams": {
    "id": "string[] (optional, max 100)",
    "login": "string[] (optional, max 100)"
  }
}
```

#### Response Schema (Success - 200)
```json
{
  "data": [
    {
      "id": "string (required)",
      "login": "string (required)",
      "display_name": "string (required)",
      "type": "string (required, enum: '', 'admin', 'staff', 'global_mod')",
      "broadcaster_type": "string (required, enum: '', 'affiliate', 'partner')",
      "description": "string (required)",
      "profile_image_url": "string (required)",
      "offline_image_url": "string (required)",
      "view_count": "integer (deprecated)",
      "email": "string (optional, requires user:read:email scope)",
      "created_at": "string (required, ISO 8601)"
    }
  ]
}
```

---

### 3. Helix API Clips Contract

#### Response Schema (Success - 200)
```json
{
  "data": [
    {
      "id": "string (required)",
      "url": "string (required)",
      "embed_url": "string (required)",
      "broadcaster_id": "string (required)",
      "broadcaster_name": "string (required)",
      "creator_id": "string (required)",
      "creator_name": "string (required)",
      "video_id": "string (required)",
      "game_id": "string (required)",
      "language": "string (required)",
      "title": "string (required)",
      "view_count": "integer (required)",
      "created_at": "string (required, ISO 8601)",
      "thumbnail_url": "string (required)",
      "duration": "number (required, seconds)",
      "vod_offset": "integer (nullable)"
    }
  ],
  "pagination": {
    "cursor": "string (optional)"
  }
}
```

---

### 4. Helix API Videos Contract

#### Response Schema (Success - 200)
```json
{
  "data": [
    {
      "id": "string (required)",
      "stream_id": "string (nullable)",
      "user_id": "string (required)",
      "user_login": "string (required)",
      "user_name": "string (required)",
      "title": "string (required)",
      "description": "string (required)",
      "created_at": "string (required, ISO 8601)",
      "published_at": "string (required, ISO 8601)",
      "url": "string (required)",
      "thumbnail_url": "string (required, contains {width} {height})",
      "viewable": "string (required, enum: 'public', 'private')",
      "view_count": "integer (required)",
      "language": "string (required)",
      "type": "string (required, enum: 'upload', 'archive', 'highlight')",
      "duration": "string (required, format: '1h2m3s')",
      "muted_segments": "array (nullable)"
    }
  ],
  "pagination": {
    "cursor": "string (optional)"
  }
}
```

---

### 5. Helix API Categories/Games Contract

#### Response Schema (Success - 200)
```json
{
  "data": [
    {
      "id": "string (required)",
      "name": "string (required)",
      "box_art_url": "string (required, contains {width} {height})",
      "igdb_id": "string (optional)"
    }
  ],
  "pagination": {
    "cursor": "string (optional)"
  }
}
```

---

### 6. OAuth2 Token Contract

#### Request Schema (Authorization Code)
```json
{
  "endpoint": "https://id.twitch.tv/oauth2/token",
  "method": "POST",
  "headers": {
    "Content-Type": "application/x-www-form-urlencoded"
  },
  "body": {
    "client_id": "string (required)",
    "client_secret": "string (required for confidential clients)",
    "code": "string (required)",
    "grant_type": "authorization_code",
    "redirect_uri": "string (required)",
    "code_verifier": "string (required for PKCE)"
  }
}
```

#### Request Schema (Refresh Token)
```json
{
  "endpoint": "https://id.twitch.tv/oauth2/token",
  "method": "POST",
  "headers": {
    "Content-Type": "application/x-www-form-urlencoded"
  },
  "body": {
    "client_id": "string (required)",
    "client_secret": "string (required for confidential clients)",
    "grant_type": "refresh_token",
    "refresh_token": "string (required)"
  }
}
```

#### Response Schema (Success - 200)
```json
{
  "access_token": "string (required)",
  "refresh_token": "string (required)",
  "expires_in": "integer (required, seconds)",
  "scope": "string[] (required)",
  "token_type": "string (required, value: 'bearer')"
}
```

#### Error Response Contract
```json
{
  "status": 400,
  "message": "Invalid refresh token"
}
```

---

### 7. GraphQL PlaybackAccessToken Contract

#### Request Schema
```json
{
  "endpoint": "https://gql.twitch.tv/gql",
  "method": "POST",
  "headers": {
    "Client-ID": "kimne78kx3ncx6brgo4mv6wki5h1ko (Twitch web client ID)",
    "Content-Type": "application/json"
  },
  "body": {
    "query": "query PlaybackAccessToken($login: String!) { streamPlaybackAccessToken(channelName: $login, params: {...}) { value signature } }",
    "variables": {
      "login": "string (required)"
    }
  }
}
```

#### Response Schema (Success - 200)
```json
{
  "data": {
    "streamPlaybackAccessToken": {
      "value": "string (required, JSON-encoded token)",
      "signature": "string (required)"
    }
  }
}
```

---

### 8. IRC/WebSocket Chat Contract

#### Connection Flow
1. Connect to `wss://irc-ws.chat.twitch.tv:443`
2. Send: `CAP REQ :twitch.tv/tags twitch.tv/commands`
3. Send: `PASS oauth:<token>` or `PASS oauth:anonymous`
4. Send: `NICK <username>` or `NICK justinfan<random>`
5. Send: `JOIN #<channel>`

#### Message Format (PRIVMSG with Tags)
```
@badge-info=subscriber/12;badges=subscriber/12,premium/1;color=#FF0000;
display-name=Username;emotes=25:0-4;id=<uuid>;mod=0;room-id=12345;
subscriber=1;tmi-sent-ts=1234567890;turbo=0;user-id=67890;user-type=
:username!username@username.tmi.twitch.tv PRIVMSG #channel :Message text
```

#### Parsed Message Contract
```json
{
  "id": "string (uuid)",
  "username": "string",
  "displayName": "string",
  "message": "string",
  "color": "string (#RRGGBB)",
  "channel": "string",
  "timestamp": "integer (ms since epoch)",
  "badges": [
    {
      "type": "string (e.g., 'subscriber', 'moderator')",
      "version": "string"
    }
  ],
  "emoteParts": [
    {
      "type": "string (enum: 'text', 'emote')",
      "content": "string",
      "emoteId": "string (optional, for emotes)"
    }
  ]
}
```

---

### 9. HLS Playlist Contracts

#### Master Playlist Format
```
#EXTM3U
#EXT-X-TWITCH-INFO:NODE="...",MANIFEST-NODE-TYPE="weaver_cluster",...
#EXT-X-MEDIA:TYPE=VIDEO,GROUP-ID="chunked",NAME="1080p60 (source)",...
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,CODECS="avc1.4D402A,mp4a.40.2",VIDEO="chunked"
https://video-edge-xxx.m3u8?sig=...&token=...
#EXT-X-MEDIA:TYPE=VIDEO,GROUP-ID="720p60",NAME="720p60",...
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,...,VIDEO="720p60"
https://video-edge-xxx.m3u8?sig=...&token=...
```

#### Ad Detection Markers (Variant Playlist)
```
#EXT-X-DISCONTINUITY
#EXT-X-DATERANGE:ID="stitched-ad-...",CLASS="twitch-stitched-ad",...
#EXT-X-SCTE35-OUT
#EXTINF:2.000,live-ad
https://ad-segment-url.ts
```

---

## Contract Testing Implementation

### Phase 1: Contract Recording

Create recorded contracts from real API responses:

```cpp
// tests/contracts/ContractRecorder.hpp
class ContractRecorder {
public:
    static void recordResponse(const QString& contractName,
                               const QByteArray& response,
                               int statusCode,
                               const QHash<QString, QString>& headers);
    
    static QByteArray loadContract(const QString& contractName);
};
```

### Phase 2: Schema Validation Tests

```cpp
// tests/contracts/TestHelixContracts.cpp
class TestHelixContracts : public QObject {
    Q_OBJECT

private slots:
    // Stream endpoint contracts
    void testStreamResponseHasRequiredFields();
    void testStreamResponseParsesCorrectly();
    void testStreamPaginationContract();
    void testStreamEmptyDataHandled();
    
    // User endpoint contracts
    void testUserResponseHasRequiredFields();
    void testUserResponseNoEmailWithoutScope();
    
    // Error contracts
    void testUnauthorizedErrorContract();
    void testRateLimitErrorContract();
    void testInvalidClientIdErrorContract();
    
    // Contract versioning
    void testBackwardCompatibilityWithDeprecatedFields();
    void testNewOptionalFieldsIgnored();
};
```

### Phase 3: Mock Server Implementation

```cpp
// tests/mocks/TwitchContractServer.hpp
class TwitchContractServer : public QObject {
    Q_OBJECT

public:
    void start(quint16 port);
    void stop();
    
    void setContractResponse(const QString& endpoint,
                             const QString& contractFile);
    void setErrorResponse(const QString& endpoint,
                          int statusCode,
                          const QString& errorContract);
    void simulateRateLimit();
    void simulateNetworkLatency(int ms);

signals:
    void requestReceived(const QString& endpoint,
                         const QHash<QString, QString>& headers);
};
```

---

## Contract Test Cases

### 1. Helix API Contract Tests

```cpp
// Verify required fields present
void TestStreamContract::testRequiredFieldsPresent() {
    auto response = loadContract("streams_success.json");
    auto streams = parseStreams(response);
    
    for (const auto& stream : streams) {
        QVERIFY(!stream["id"].toString().isEmpty());
        QVERIFY(!stream["user_id"].toString().isEmpty());
        QVERIFY(!stream["user_name"].toString().isEmpty());
        QVERIFY(!stream["title"].toString().isEmpty());
        QVERIFY(stream.contains("viewer_count"));
        QVERIFY(!stream["thumbnail_url"].toString().isEmpty());
    }
}

// Verify thumbnail URL format
void TestStreamContract::testThumbnailUrlFormat() {
    auto response = loadContract("streams_success.json");
    auto streams = parseStreams(response);
    
    for (const auto& stream : streams) {
        QString url = stream["thumbnail_url"].toString();
        QVERIFY(url.contains("{width}"));
        QVERIFY(url.contains("{height}"));
    }
}

// Verify error response format
void TestStreamContract::testUnauthorizedErrorFormat() {
    auto response = loadContract("streams_unauthorized.json");
    auto doc = QJsonDocument::fromJson(response);
    auto obj = doc.object();
    
    QCOMPARE(obj["status"].toInt(), 401);
    QVERIFY(!obj["message"].toString().isEmpty());
    QCOMPARE(obj["error"].toString(), QString("Unauthorized"));
}
```

### 2. OAuth Contract Tests

```cpp
void TestOAuthContract::testTokenResponseFormat() {
    auto response = loadContract("oauth_token_success.json");
    auto doc = QJsonDocument::fromJson(response);
    auto obj = doc.object();
    
    QVERIFY(!obj["access_token"].toString().isEmpty());
    QVERIFY(!obj["refresh_token"].toString().isEmpty());
    QVERIFY(obj["expires_in"].toInt() > 0);
    QCOMPARE(obj["token_type"].toString(), QString("bearer"));
}

void TestOAuthContract::testRefreshTokenInvalidError() {
    auto response = loadContract("oauth_refresh_invalid.json");
    auto doc = QJsonDocument::fromJson(response);
    auto obj = doc.object();
    
    QCOMPARE(obj["status"].toInt(), 400);
    QVERIFY(obj["message"].toString().contains("Invalid refresh token"));
}
```

### 3. IRC Message Contract Tests

```cpp
void TestIrcContract::testPrivmsgParsing() {
    QString raw = "@badge-info=;badges=broadcaster/1;color=#FF0000;"
                  "display-name=TestUser;emotes=;id=abc123;"
                  "mod=0;room-id=12345;subscriber=0;tmi-sent-ts=1609459200000;"
                  "user-id=67890;user-type= "
                  ":testuser!testuser@testuser.tmi.twitch.tv PRIVMSG #channel :Hello!";
    
    auto msg = parseIrcMessage(raw);
    
    QCOMPARE(msg["displayName"].toString(), QString("TestUser"));
    QCOMPARE(msg["color"].toString(), QString("#FF0000"));
    QCOMPARE(msg["message"].toString(), QString("Hello!"));
    QVERIFY(!msg["id"].toString().isEmpty());
}

void TestIrcContract::testEmoteParsingContract() {
    QString emotesTag = "25:0-4,6-10/1902:12-16";
    QString message = "Kappa Kappa PogChamp";
    
    auto parts = parseEmoteParts(emotesTag, message);
    
    QCOMPARE(parts.size(), 3);
    QCOMPARE(parts[0].toMap()["type"].toString(), QString("emote"));
    QCOMPARE(parts[0].toMap()["emoteId"].toString(), QString("25"));
}
```

### 4. HLS Playlist Contract Tests

```cpp
void TestHlsContract::testMasterPlaylistParsing() {
    QString playlist = loadContract("master_playlist.m3u8");
    auto variants = parseVariants(playlist);
    
    QVERIFY(!variants.isEmpty());
    
    for (const auto& v : variants) {
        QVERIFY(!v.url.isEmpty());
        QVERIFY(v.bandwidth > 0);
    }
}

void TestHlsContract::testAdMarkerDetection() {
    QString playlistWithAds = loadContract("variant_with_ads.m3u8");
    
    QVERIFY(detectAdsInPlaylist(playlistWithAds));
    QVERIFY(playlistWithAds.contains("twitch-stitched-ad") ||
            playlistWithAds.contains("EXT-X-SCTE35-OUT"));
}
```

---

## Rate Limiting Contract

```json
{
  "headers": {
    "Ratelimit-Limit": "800",
    "Ratelimit-Remaining": "799",
    "Ratelimit-Reset": "1609459200"
  },
  "behavior": {
    "requests_per_minute": 800,
    "points_per_request": 1,
    "burst_allowed": true
  }
}
```

### Rate Limit Test
```cpp
void TestRateLimitContract::testRateLimitHeadersParsed() {
    auto headers = loadContractHeaders("rate_limited_response");
    
    QVERIFY(headers.contains("Ratelimit-Limit"));
    QVERIFY(headers.contains("Ratelimit-Remaining"));
    QVERIFY(headers.contains("Ratelimit-Reset"));
    
    int limit = headers["Ratelimit-Limit"].toInt();
    int remaining = headers["Ratelimit-Remaining"].toInt();
    
    QVERIFY(limit > 0);
    QVERIFY(remaining >= 0);
    QVERIFY(remaining <= limit);
}
```

---

## Contract Versioning Strategy

### 1. Breaking Change Detection

```cpp
class ContractVersionChecker {
public:
    enum ChangeType {
        Compatible,      // New optional fields added
        Breaking,        // Required fields removed/renamed
        TypeChange,      // Field type changed
        EnumChange       // Enum values changed
    };
    
    static ChangeType compareContracts(const QString& oldContract,
                                        const QString& newContract);
    static QStringList detectBreakingChanges(const QString& oldContract,
                                              const QString& newContract);
};
```

### 2. Backward Compatibility Tests

```cpp
void TestBackwardCompatibility::testDeprecatedFieldsStillParsed() {
    // tag_ids was deprecated but should still work if present
    auto response = loadContract("streams_with_deprecated_fields.json");
    auto streams = parseStreams(response);
    
    // Should not crash, deprecated fields ignored
    QVERIFY(!streams.isEmpty());
}

void TestBackwardCompatibility::testNewFieldsIgnored() {
    // API may add new fields - ensure they don't break parsing
    auto response = loadContract("streams_with_new_fields.json");
    auto streams = parseStreams(response);
    
    QVERIFY(!streams.isEmpty());
    // Verify known fields still work
    QVERIFY(!streams[0].toMap()["id"].toString().isEmpty());
}
```

### 3. API Deprecation Handling

```cpp
// Log warnings for deprecated endpoints/fields
class DeprecationWarner {
public:
    static void checkForDeprecations(const QString& endpoint,
                                     const QJsonObject& response);
};

// Example deprecation config
{
  "deprecations": [
    {
      "field": "tag_ids",
      "deprecated_since": "2023-07-13",
      "replacement": "tags",
      "removal_date": "2024-01-01"
    },
    {
      "field": "view_count",
      "deprecated_since": "2022-06-01",
      "replacement": null,
      "note": "Always returns 0, do not rely on this field"
    }
  ]
}
```

---

## Integration Test Strategy

### When to Use Real API

| Scenario | Use Real API | Use Mocks |
|----------|-------------|-----------|
| CI/CD Pipeline | No | Yes |
| Local Development | Sparingly | Yes |
| Contract Recording | Yes | N/A |
| Load Testing | No | Yes |
| Pre-Release Validation | Yes (sandbox) | No |
| Unit Tests | No | Yes |

### Sandbox vs Production Testing

```cpp
class TwitchApiEnvironment {
public:
    enum Environment {
        Production,  // api.twitch.tv
        Mock,        // localhost mock server
        Recorded     // Use recorded contracts
    };
    
    static QString getBaseUrl(Environment env);
    static bool shouldRecordResponses();
};

// In tests
void TestTwitchIntegration::initTestCase() {
    if (qEnvironmentVariableIsSet("TWITCH_RECORD_CONTRACTS")) {
        m_env = TwitchApiEnvironment::Production;
        m_recorder = new ContractRecorder(this);
    } else {
        m_env = TwitchApiEnvironment::Mock;
        m_mockServer = new TwitchContractServer(this);
        m_mockServer->start(8080);
    }
}
```

---

## Contract Files Structure

```
tests/
  contracts/
    helix/
      streams_success.json
      streams_empty.json
      streams_unauthorized.json
      streams_rate_limited.json
      users_success.json
      users_not_found.json
      clips_success.json
      videos_success.json
      categories_success.json
      search_channels_success.json
      search_categories_success.json
    oauth/
      token_success.json
      token_refresh_success.json
      token_invalid_code.json
      token_invalid_refresh.json
    graphql/
      playback_token_success.json
      playback_token_error.json
    irc/
      privmsg_with_emotes.txt
      privmsg_with_badges.txt
      notice_auth_failed.txt
    hls/
      master_playlist.m3u8
      variant_clean.m3u8
      variant_with_ads.m3u8
    ContractRecorder.cpp
    ContractRecorder.hpp
    ContractValidator.cpp
    ContractValidator.hpp
```

---

## Implementation Priorities

### Phase 1: Core Contracts (Week 1-2)
1. [ ] Record real API responses for all Helix endpoints
2. [ ] Implement schema validation for streams, users, clips
3. [ ] Create mock server with contract responses
4. [ ] Add tests for error response contracts

### Phase 2: Authentication Contracts (Week 2-3)
1. [ ] OAuth token exchange contracts
2. [ ] Token refresh contracts
3. [ ] Error handling contracts (invalid token, expired)

### Phase 3: Real-time Contracts (Week 3-4)
1. [ ] IRC message parsing contracts
2. [ ] HLS playlist parsing contracts
3. [ ] Ad marker detection contracts

### Phase 4: Versioning & CI (Week 4-5)
1. [ ] Breaking change detection
2. [ ] Backward compatibility tests
3. [ ] CI integration with mock server
4. [ ] Deprecation warning system

---

## Test Data Factory Integration

The existing `TwitchTestData` class in `tests/helpers/TwitchTestData.hpp` should be enhanced:

```cpp
class TwitchTestData {
public:
    // Existing methods...
    
    // New contract-based methods
    static QString loadStreamContract(const QString& variant = "success");
    static QString loadUserContract(const QString& variant = "success");
    static QString loadErrorContract(int statusCode);
    
    // Schema validation
    static bool validateAgainstContract(const QJsonDocument& doc,
                                        const QString& contractName);
};
```

---

## Continuous Integration

### GitHub Actions Workflow

```yaml
name: Contract Tests

on: [push, pull_request]

jobs:
  contract-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Start Mock Server
        run: ./tests/contracts/start_mock_server.sh
        
      - name: Run Contract Tests
        env:
          TWITCH_API_BASE: http://localhost:8080
        run: ctest --test-dir build -R "Contract"
        
      - name: Validate Contract Schemas
        run: ./scripts/validate_contracts.sh
```

---

## Metrics & Monitoring

Track contract test health:

- Contract coverage: % of API endpoints with contract tests
- Breaking change alerts: Automated detection when real API differs
- Response time baselines: Detect performance regressions
- Schema drift: When recorded contracts become stale

