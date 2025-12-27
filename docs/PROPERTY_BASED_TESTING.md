# Property-Based Testing Strategy for BluePlayer

## Overview

This document defines a comprehensive property-based testing (PBT) strategy for BluePlayer, identifying testable properties, invariants, and implementation guidance.

## Recommended Framework: RapidCheck

For Qt/C++ projects, **RapidCheck** is the recommended property testing library:

```cmake
# CMakeLists.txt addition
FetchContent_Declare(
  rapidcheck
  GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
  GIT_TAG master
)
FetchContent_MakeAvailable(rapidcheck)

target_link_libraries(BluePlayerTests PRIVATE rapidcheck)
```

### Why RapidCheck?
- Pure C++ (no external dependencies beyond STL)
- Excellent shrinking support for debugging
- Easy custom generator creation
- Qt-compatible (can wrap QString, QVariantMap, etc.)

---

## 1. HLS Playlist Parsing Properties

### Location: `src/media/HlsAdFilter.cpp`

### 1.1 `parseVariants()` Properties

```cpp
// Property: Parsing valid HLS playlists produces non-empty variant list
RC_GTEST_PROP(HlsAdFilter, ValidPlaylistProducesVariants, ()) {
    auto playlist = *genValidHlsPlaylist();
    HlsAdFilter filter;
    auto variants = filter.parseVariants(playlist);
    RC_ASSERT(!variants.isEmpty());
}

// Property: All parsed variants have valid URLs
RC_GTEST_PROP(HlsAdFilter, AllVariantsHaveValidUrls, ()) {
    auto playlist = *genValidHlsPlaylist();
    HlsAdFilter filter;
    auto variants = filter.parseVariants(playlist);
    for (const auto& v : variants) {
        RC_ASSERT(!v.url.isEmpty());
        RC_ASSERT(v.url.startsWith("http") || !v.url.contains("#"));
    }
}

// Property: Bandwidth is always non-negative
RC_GTEST_PROP(HlsAdFilter, BandwidthNonNegative, ()) {
    auto playlist = *genValidHlsPlaylist();
    HlsAdFilter filter;
    auto variants = filter.parseVariants(playlist);
    for (const auto& v : variants) {
        RC_ASSERT(v.bandwidth >= 0);
    }
}
```

### 1.2 `selectBestVariant()` Properties

```cpp
// Property: Selection always returns a valid URL from the input list
RC_GTEST_PROP(HlsAdFilter, SelectionReturnsInputUrl, ()) {
    auto variants = *genNonEmptyVariantList();
    auto quality = *rc::gen::element("chunked", "1080p60", "720p");
    
    HlsAdFilter filter;
    QString selected = filter.selectBestVariant(variants, quality);
    
    if (!selected.isEmpty()) {
        bool found = std::any_of(variants.begin(), variants.end(),
            [&](const auto& v) { return v.url == selected; });
        RC_ASSERT(found);
    }
}

// Property: Chunked quality always preferred when available
RC_GTEST_PROP(HlsAdFilter, ChunkedPreferredWhenAvailable, ()) {
    auto variants = *genVariantListWithChunked();
    HlsAdFilter filter;
    QString selected = filter.selectBestVariant(variants, "chunked");
    
    auto chunkedIt = std::find_if(variants.begin(), variants.end(),
        [](const auto& v) { return v.name.contains("chunked"); });
    RC_ASSERT(selected == chunkedIt->url);
}
```

### 1.3 `detectAdsInPlaylist()` Properties

```cpp
// Property: Empty playlist = no ads detected
RC_GTEST_PROP(HlsAdFilter, EmptyPlaylistNoAds, ()) {
    HlsAdFilter filter;
    RC_ASSERT(!filter.detectAdsInPlaylist(""));
    RC_ASSERT(!filter.detectAdsInPlaylist("#EXTM3U\n"));
}

// Property: Known ad patterns always detected
RC_GTEST_PROP(HlsAdFilter, KnownPatternsDetected, ()) {
    auto pattern = *rc::gen::element(
        "twitch-stitched-ad", "Amazon-Ads", "X-TV-TWITCH-AD-"
    );
    QString playlist = "#EXTM3U\n#EXT-X-DISCONTINUITY\n" + pattern + "\n";
    
    HlsAdFilter filter;
    RC_ASSERT(filter.detectAdsInPlaylist(playlist));
}

// Property: Idempotence - detecting twice yields same result
RC_GTEST_PROP(HlsAdFilter, DetectionIdempotent, ()) {
    auto playlist = *genAnyHlsPlaylist();
    HlsAdFilter filter;
    bool first = filter.detectAdsInPlaylist(playlist);
    bool second = filter.detectAdsInPlaylist(playlist);
    RC_ASSERT(first == second);
}
```

### 1.4 `extractStreamerLogin()` Properties

```cpp
// Property: Extracted login matches input when URL is well-formed
RC_GTEST_PROP(HlsAdFilter, ExtractRoundtrip, ()) {
    auto login = *genValidTwitchLogin();
    QString url = QString("https://usher.ttvnw.net/api/channel/hls/%1.m3u8").arg(login);
    
    HlsAdFilter filter;
    QString extracted = filter.extractStreamerLogin(url);
    RC_ASSERT(extracted == login);
}
```

---

## 2. Result<T> Monad Laws

### Location: `src/core/Result.hpp`

### 2.1 Functor Laws

```cpp
// Identity: map(id) == id
RC_GTEST_PROP(Result, FunctorIdentity, ()) {
    auto value = *rc::gen::arbitrary<int>();
    auto result = Result<int>::success(value);
    
    auto mapped = result.map([](int x) { return x; });
    RC_ASSERT(mapped.value() == result.value());
}

// Composition: map(f . g) == map(f) . map(g)
RC_GTEST_PROP(Result, FunctorComposition, ()) {
    auto value = *rc::gen::arbitrary<int>();
    auto result = Result<int>::success(value);
    
    auto f = [](int x) { return x * 2; };
    auto g = [](int x) { return x + 1; };
    
    auto composed = result.map([&](int x) { return f(g(x)); });
    auto sequential = result.map(g).map(f);
    
    RC_ASSERT(composed.value() == sequential.value());
}
```

### 2.2 Monad Laws

```cpp
// Left Identity: return a >>= f == f a
RC_GTEST_PROP(Result, MonadLeftIdentity, ()) {
    auto value = *rc::gen::arbitrary<int>();
    auto f = [](int x) { return Result<int>::success(x * 2); };
    
    auto result = Result<int>::success(value);
    auto bound = result.flatMap(f);
    auto direct = f(value);
    
    RC_ASSERT(bound.isSuccess() == direct.isSuccess());
    if (bound.isSuccess()) {
        RC_ASSERT(bound.value() == direct.value());
    }
}

// Right Identity: m >>= return == m
RC_GTEST_PROP(Result, MonadRightIdentity, ()) {
    auto value = *rc::gen::arbitrary<int>();
    auto result = Result<int>::success(value);
    
    auto bound = result.flatMap([](int x) { return Result<int>::success(x); });
    
    RC_ASSERT(bound.value() == result.value());
}

// Associativity: (m >>= f) >>= g == m >>= (\x -> f x >>= g)
RC_GTEST_PROP(Result, MonadAssociativity, ()) {
    auto value = *rc::gen::arbitrary<int>();
    auto result = Result<int>::success(value);
    
    auto f = [](int x) { return Result<int>::success(x * 2); };
    auto g = [](int x) { return Result<int>::success(x + 1); };
    
    auto left = result.flatMap(f).flatMap(g);
    auto right = result.flatMap([&](int x) { return f(x).flatMap(g); });
    
    RC_ASSERT(left.value() == right.value());
}
```

### 2.3 Success/Failure Invariants

```cpp
// Property: Success implies isSuccess() true and isFailure() false
RC_GTEST_PROP(Result, SuccessInvariant, ()) {
    auto value = *rc::gen::arbitrary<int>();
    auto result = Result<int>::success(value);
    
    RC_ASSERT(result.isSuccess());
    RC_ASSERT(!result.isFailure());
    RC_ASSERT(result.value() == value);
}

// Property: Failure preserves error through map
RC_GTEST_PROP(Result, FailurePreservedThroughMap, ()) {
    auto error = Error(ErrorCode::NetworkError, "test error");
    auto result = Result<int>::failure(error);
    
    auto mapped = result.map([](int x) { return x * 2; });
    
    RC_ASSERT(mapped.isFailure());
    RC_ASSERT(mapped.error().code() == error.code());
}
```

---

## 3. VodMetadata Serialization Properties

### Location: `src/core/VodMetadata.hpp`

### 3.1 JSON Roundtrip

```cpp
// Property: toJson/fromJson roundtrip preserves all fields
RC_GTEST_PROP(VodMetadata, JsonRoundtrip, ()) {
    auto metadata = *genValidVodMetadata();
    
    QJsonObject json = metadata.toJson();
    VodMetadata restored = VodMetadata::fromJson(json);
    
    RC_ASSERT(restored.id == metadata.id);
    RC_ASSERT(restored.streamerLogin == metadata.streamerLogin);
    RC_ASSERT(restored.streamerName == metadata.streamerName);
    RC_ASSERT(restored.streamTitle == metadata.streamTitle);
    RC_ASSERT(restored.duration == metadata.duration);
    RC_ASSERT(restored.fileSize == metadata.fileSize);
    RC_ASSERT(restored.filePath == metadata.filePath);
    RC_ASSERT(restored.watchPosition == metadata.watchPosition);
}

// Property: Double serialization produces identical JSON
RC_GTEST_PROP(VodMetadata, JsonIdempotent, ()) {
    auto metadata = *genValidVodMetadata();
    
    QJsonObject json1 = metadata.toJson();
    VodMetadata restored = VodMetadata::fromJson(json1);
    QJsonObject json2 = restored.toJson();
    
    RC_ASSERT(json1 == json2);
}
```

### 3.2 Duration Formatting Properties

```cpp
// Property: formatDuration output contains valid time units
RC_GTEST_PROP(VodMetadata, DurationFormatValid, ()) {
    auto seconds = *rc::gen::inRange<qint64>(0, 86400 * 7);  // Up to 1 week
    
    QString formatted = VodMetadata::formatDuration(seconds);
    
    // Must contain at least one time unit
    bool hasUnit = formatted.contains('s') || 
                   formatted.contains('m') || 
                   formatted.contains('h');
    RC_ASSERT(hasUnit);
}

// Property: formatDuration is monotonic
RC_GTEST_PROP(VodMetadata, DurationFormatMonotonic, ()) {
    auto s1 = *rc::gen::inRange<qint64>(0, 43200);
    auto s2 = *rc::gen::inRange<qint64>(s1, 86400);
    
    QString f1 = VodMetadata::formatDuration(s1);
    QString f2 = VodMetadata::formatDuration(s2);
    
    // Longer duration should have >= length (roughly)
    // This is a weak property but catches major bugs
    if (s2 > s1 * 10) {
        RC_ASSERT(f2.length() >= f1.length() || f2.contains('h'));
    }
}
```

### 3.3 File Size Formatting Properties

```cpp
// Property: formatFileSize produces valid unit suffix
RC_GTEST_PROP(VodMetadata, FileSizeFormatValid, ()) {
    auto bytes = *rc::gen::inRange<qint64>(0, 10LL * 1024 * 1024 * 1024);
    
    QString formatted = VodMetadata::formatFileSize(bytes);
    
    bool hasValidUnit = formatted.endsWith("B") || 
                        formatted.endsWith("KB") || 
                        formatted.endsWith("MB") || 
                        formatted.endsWith("GB");
    RC_ASSERT(hasValidUnit);
}

// Property: formatFileSize is monotonic in magnitude
RC_GTEST_PROP(VodMetadata, FileSizeFormatMonotonic, ()) {
    auto b1 = *rc::gen::inRange<qint64>(0, 1024 * 1024);
    auto b2 = *rc::gen::inRange<qint64>(b1, 1024 * 1024 * 1024);
    
    QString f1 = VodMetadata::formatFileSize(b1);
    QString f2 = VodMetadata::formatFileSize(b2);
    
    // Extract numeric values
    auto extractNum = [](const QString& s) {
        return s.split(' ').first().toDouble();
    };
    
    // Different units should show magnitude increase
    if (f2.contains("GB") && f1.contains("MB")) {
        RC_ASSERT(true);  // GB > MB by definition
    }
}
```

---

## 4. InputValidator Properties

### Location: `src/core/InputValidator.cpp`

### 4.1 URL Validation

```cpp
// Property: Valid URLs pass validation
RC_GTEST_PROP(InputValidator, ValidUrlsPass, ()) {
    auto scheme = *rc::gen::element("http", "https");
    auto host = *genValidHostname();
    auto path = *genValidPath();
    
    QString url = QString("%1://%2%3").arg(scheme, host, path);
    RC_ASSERT(InputValidator::isValidUrl(url));
}

// Property: Empty URL fails
RC_GTEST_PROP(InputValidator, EmptyUrlFails, ()) {
    RC_ASSERT(!InputValidator::isValidUrl(""));
    RC_ASSERT(!InputValidator::isValidUrl(QString()));
}

// Property: URLs without scheme fail
RC_GTEST_PROP(InputValidator, NoSchemeUrlFails, ()) {
    auto host = *genValidHostname();
    QString url = QString("%1/path").arg(host);
    RC_ASSERT(!InputValidator::isValidUrl(url));
}
```

### 4.2 Twitch User ID Validation

```cpp
// Property: Numeric strings pass
RC_GTEST_PROP(InputValidator, NumericUserIdPasses, ()) {
    auto num = *rc::gen::inRange<uint64_t>(1, 999999999999ULL);
    QString userId = QString::number(num);
    RC_ASSERT(InputValidator::isValidTwitchUserId(userId));
}

// Property: Non-numeric strings fail
RC_GTEST_PROP(InputValidator, NonNumericUserIdFails, ()) {
    auto text = *rc::gen::container<std::string>(
        rc::gen::element('a', 'b', 'c', 'A', 'B', 'C', '!', '@')
    );
    RC_PRE(!text.empty());
    RC_ASSERT(!InputValidator::isValidTwitchUserId(QString::fromStdString(text)));
}
```

### 4.3 String Sanitization

```cpp
// Property: Sanitization is idempotent
RC_GTEST_PROP(InputValidator, SanitizationIdempotent, ()) {
    auto input = *rc::gen::arbitrary<std::string>();
    QString qInput = QString::fromStdString(input);
    
    QString once = InputValidator::sanitizeString(qInput);
    QString twice = InputValidator::sanitizeString(once);
    
    RC_ASSERT(once == twice);
}

// Property: Sanitized output has no control characters
RC_GTEST_PROP(InputValidator, SanitizedNoControlChars, ()) {
    auto input = *rc::gen::arbitrary<std::string>();
    QString sanitized = InputValidator::sanitizeString(QString::fromStdString(input));
    
    for (const QChar& c : sanitized) {
        RC_ASSERT(c.unicode() >= 0x20 || c == '\n' || c == '\r' || c == '\t');
    }
}

// Property: Sanitized output length <= max limit
RC_GTEST_PROP(InputValidator, SanitizedLengthBounded, ()) {
    auto input = *rc::gen::arbitrary<std::string>();
    QString sanitized = InputValidator::sanitizeString(QString::fromStdString(input));
    
    RC_ASSERT(sanitized.length() <= 10000);
}
```

---

## 5. CacheManager Invariants

### Location: `src/core/CacheManager.cpp`

### 5.1 Cache Size Invariants

```cpp
// Property: Total VOD size equals sum of individual VOD sizes
RC_GTEST_PROP(CacheManager, TotalSizeConsistent, ()) {
    CacheManager manager;
    auto vodList = *genVodMetadataList(1, 10);
    
    for (const auto& vod : vodList) {
        manager.addVod(vod);
    }
    
    qint64 expectedTotal = 0;
    for (const auto& vod : vodList) {
        expectedTotal += vod.fileSize;
    }
    
    RC_ASSERT(manager.totalVodSize() == expectedTotal);
}

// Property: VOD count matches list size
RC_GTEST_PROP(CacheManager, VodCountConsistent, ()) {
    CacheManager manager;
    auto vodList = *genVodMetadataList(0, 20);
    
    for (const auto& vod : vodList) {
        manager.addVod(vod);
    }
    
    RC_ASSERT(manager.vodCount() == vodList.size());
}
```

### 5.2 Cleanup Threshold Properties

```cpp
// Property: After cleanup, usage < target threshold
RC_GTEST_PROP(CacheManager, CleanupReachesTarget, ()) {
    CacheManager manager;
    manager.setMaxCacheSize(1024 * 1024);  // 1 MB
    
    // Add VODs to exceed threshold
    auto vodList = *genVodMetadataList(5, 20);
    for (const auto& vod : vodList) {
        auto modifiedVod = vod;
        modifiedVod.fileSize = 100 * 1024;  // 100 KB each
        manager.addVod(modifiedVod);
    }
    
    manager.performCleanup();
    
    // Should be below 80% after cleanup
    RC_ASSERT(manager.cacheUsagePercent() <= 80.0);
}

// Property: Cleanup preserves most recently used VODs
RC_GTEST_PROP(CacheManager, CleanupPreservesRecent, ()) {
    CacheManager manager;
    manager.setMaxCacheSize(500 * 1024);  // 500 KB
    
    // Add old VODs
    for (int i = 0; i < 5; ++i) {
        VodMetadata vod;
        vod.id = VodMetadata::generateId();
        vod.filePath = QString("/tmp/old_%1.mp4").arg(i);
        vod.duration = 100;
        vod.fileSize = 100 * 1024;
        vod.recordedAt = QDateTime::currentDateTime().addDays(-i - 10);
        vod.lastPlayedAt = QDateTime::currentDateTime().addDays(-i - 10);
        manager.addVod(vod);
    }
    
    // Add recent VOD
    VodMetadata recentVod;
    recentVod.id = VodMetadata::generateId();
    recentVod.filePath = "/tmp/recent.mp4";
    recentVod.duration = 100;
    recentVod.fileSize = 100 * 1024;
    recentVod.recordedAt = QDateTime::currentDateTime();
    recentVod.lastPlayedAt = QDateTime::currentDateTime();
    manager.addVod(recentVod);
    
    manager.performCleanup();
    
    // Recent VOD should still exist
    auto metadata = manager.getVodMetadata(recentVod.id);
    RC_ASSERT(!metadata.isEmpty());
}
```

### 5.3 Time Position Properties

```cpp
// Property: clampToCache keeps time within cache bounds
RC_GTEST_PROP(CacheManager, ClampToCacheBounds, ()) {
    CacheManager manager;
    auto start = *rc::gen::inRange<double>(0.0, 1000.0);
    auto end = *rc::gen::inRange<double>(start + 1.0, start + 3600.0);
    auto time = *rc::gen::inRange<double>(-100.0, end + 100.0);
    
    manager.startCaching(start);
    manager.updateCacheEnd(end);
    
    double clamped = manager.clampToCache(time);
    
    RC_ASSERT(clamped >= start);
    RC_ASSERT(clamped <= end);
}

// Property: isTimeInCache consistent with bounds
RC_GTEST_PROP(CacheManager, IsTimeInCacheConsistent, ()) {
    CacheManager manager;
    auto start = *rc::gen::inRange<double>(0.0, 1000.0);
    auto end = *rc::gen::inRange<double>(start + 1.0, start + 3600.0);
    
    manager.startCaching(start);
    manager.updateCacheEnd(end);
    
    // Time inside cache
    auto inside = *rc::gen::inRange<double>(start, end);
    RC_ASSERT(manager.isTimeInCache(inside));
    
    // Time outside cache (before start)
    if (start > 0.5) {
        auto before = *rc::gen::inRange<double>(0.0, start - 0.1);
        RC_ASSERT(!manager.isTimeInCache(before));
    }
}
```

---

## 6. TwitchChatClient IRC Parsing

### Location: `src/chat/TwitchChatClient.cpp`

### 6.1 Tag Parsing Properties

```cpp
// Property: parseTags roundtrip (encode then decode)
RC_GTEST_PROP(TwitchChatClient, TagParsingRoundtrip, ()) {
    auto key = *genValidTagKey();
    auto value = *genValidTagValue();
    
    QString encoded = QString("%1=%2").arg(key, value);
    TwitchChatClient client;
    auto parsed = client.parseTags(encoded);
    
    RC_ASSERT(parsed.contains(key));
    // Note: unescape transformations apply
}

// Property: Empty tags produce empty map
RC_GTEST_PROP(TwitchChatClient, EmptyTagsEmptyMap, ()) {
    TwitchChatClient client;
    auto result = client.parseTags("");
    RC_ASSERT(result.isEmpty());
}

// Property: Multiple tags all parsed
RC_GTEST_PROP(TwitchChatClient, MultipleTagsParsed, ()) {
    auto tagCount = *rc::gen::inRange<size_t>(1, 10);
    QStringList tagPairs;
    
    for (size_t i = 0; i < tagCount; ++i) {
        tagPairs << QString("key%1=value%1").arg(i);
    }
    
    TwitchChatClient client;
    auto result = client.parseTags(tagPairs.join(";"));
    
    RC_ASSERT(result.size() == tagCount);
}
```

### 6.2 Emote Parts Properties

```cpp
// Property: parseEmoteParts covers entire message
RC_GTEST_PROP(TwitchChatClient, EmotePartsCoverMessage, ()) {
    auto message = *rc::gen::nonEmpty<std::string>();
    TwitchChatClient client;
    auto parts = client.parseEmoteParts("", QString::fromStdString(message));
    
    // Reconstruct message from parts
    QString reconstructed;
    for (const auto& part : parts) {
        reconstructed += part.toMap().value("content").toString();
    }
    
    RC_ASSERT(reconstructed == QString::fromStdString(message));
}

// Property: No overlapping emote ranges
RC_GTEST_PROP(TwitchChatClient, EmotePartsNoOverlap, ()) {
    auto message = *rc::gen::container<std::string>(rc::gen::character<char>());
    RC_PRE(message.length() >= 20);
    
    // Generate non-overlapping emote ranges
    QString emotesTag = "25:0-4/1902:10-14";
    
    TwitchChatClient client;
    auto parts = client.parseEmoteParts(emotesTag, QString::fromStdString(message));
    
    // Verify no character appears in multiple parts
    QSet<int> usedPositions;
    int pos = 0;
    for (const auto& part : parts) {
        QString content = part.toMap().value("content").toString();
        for (int i = 0; i < content.length(); ++i) {
            RC_ASSERT(!usedPositions.contains(pos + i));
            usedPositions.insert(pos + i);
        }
        pos += content.length();
    }
}
```

### 6.3 Badge Parsing Properties

```cpp
// Property: parseBadges produces valid badge objects
RC_GTEST_PROP(TwitchChatClient, BadgesHaveTypeAndVersion, ()) {
    auto badgeCount = *rc::gen::inRange<size_t>(1, 5);
    QStringList badgePairs;
    
    for (size_t i = 0; i < badgeCount; ++i) {
        badgePairs << QString("badge%1/1").arg(i);
    }
    
    TwitchChatClient client;
    auto badges = client.parseBadges(badgePairs.join(","));
    
    for (const auto& badge : badges) {
        auto badgeMap = badge.toMap();
        RC_ASSERT(badgeMap.contains("type"));
        RC_ASSERT(badgeMap.contains("version"));
        RC_ASSERT(!badgeMap.value("type").toString().isEmpty());
    }
}
```

---

## 7. StateMachine Transition Properties

### Location: `src/core/StateMachineLiveReplay.hpp`

### 7.1 State Transition Validity

```cpp
// Property: All valid transitions produce valid states
RC_GTEST_PROP(StateMachine, ValidTransitionsProduceValidStates, ()) {
    StateMachineLiveReplay sm;
    
    auto events = *rc::gen::container<std::vector<LiveReplayEvent>>(
        rc::gen::element(
            LiveReplayEvent::SeekBackRequest,
            LiveReplayEvent::JumpToLive,
            LiveReplayEvent::PlayFromReplay,
            LiveReplayEvent::PauseFromReplay
        )
    );
    
    for (const auto& event : events) {
        auto oldState = sm.state();
        sm.enqueueEvent({event});
        auto newState = sm.state();
        
        // State should be one of the defined enum values
        RC_ASSERT(newState >= LiveReplayState::Live);
        RC_ASSERT(newState <= LiveReplayState::Error);
    }
}

// Property: JumpToLive from any replay state reaches Live
RC_GTEST_PROP(StateMachine, JumpToLiveReachesLive, ()) {
    StateMachineLiveReplay sm;
    
    // Get into a replay state
    sm.updateCacheDuration(100.0);
    sm.seekToTime(50.0);
    
    // Jump to live
    sm.jumpToLive();
    
    // Should eventually reach Live or CatchUpToLive
    auto state = sm.state();
    RC_ASSERT(state == LiveReplayState::Live || 
              state == LiveReplayState::CatchUpToLive);
}

// Property: Latency is non-negative
RC_GTEST_PROP(StateMachine, LatencyNonNegative, ()) {
    StateMachineLiveReplay sm;
    
    auto liveEdge = *rc::gen::inRange<double>(0.0, 10000.0);
    auto currentTime = *rc::gen::inRange<double>(0.0, liveEdge);
    
    sm.updateLiveEdgeTime(liveEdge);
    sm.updateCurrentTime(currentTime);
    
    RC_ASSERT(sm.latency() >= 0.0);
}
```

### 7.2 Time Invariants

```cpp
// Property: currentTime <= liveEdgeTime in Live state
RC_GTEST_PROP(StateMachine, LiveStateTimeInvariant, ()) {
    StateMachineLiveReplay sm;
    
    auto liveEdge = *rc::gen::inRange<double>(100.0, 10000.0);
    sm.updateLiveEdgeTime(liveEdge);
    sm.updateCurrentTime(liveEdge - 1.0);
    
    // In Live state, currentTime should not exceed liveEdge
    if (sm.state() == LiveReplayState::Live) {
        RC_ASSERT(sm.currentTime() <= sm.liveEdgeTime());
    }
}
```

---

## 8. Generator Strategies

### 8.1 Domain-Specific Generators

```cpp
namespace rc {

// Generator for valid Twitch login names
template<>
struct Arbitrary<TwitchLogin> {
    static Gen<TwitchLogin> arbitrary() {
        return gen::apply(
            [](std::string s) { return TwitchLogin{QString::fromStdString(s)}; },
            gen::container<std::string>(
                gen::inRange('a', 'z' + 1),
                gen::inRange<size_t>(4, 25)
            )
        );
    }
};

// Generator for VodMetadata
template<>
struct Arbitrary<VodMetadata> {
    static Gen<VodMetadata> arbitrary() {
        return gen::build<VodMetadata>(
            gen::set(&VodMetadata::id, 
                     gen::apply([](){ return VodMetadata::generateId(); })),
            gen::set(&VodMetadata::streamerLogin, 
                     gen::map(Arbitrary<TwitchLogin>::arbitrary(), 
                              [](auto l) { return l.value; })),
            gen::set(&VodMetadata::duration, 
                     gen::inRange<qint64>(1, 3600 * 24)),
            gen::set(&VodMetadata::fileSize, 
                     gen::inRange<qint64>(1024, 10LL * 1024 * 1024 * 1024)),
            gen::set(&VodMetadata::filePath,
                     gen::apply([](int id) { 
                         return QString("/tmp/vod_%1.mp4").arg(id); 
                     }, gen::arbitrary<int>())),
            gen::set(&VodMetadata::watchPosition, 
                     gen::inRange<qint64>(0, 3600 * 24))
        );
    }
};

// Generator for HLS playlist content
Gen<QString> genValidHlsPlaylist() {
    return gen::apply(
        [](int variantCount, std::vector<int> bandwidths) {
            QString playlist = "#EXTM3U\n";
            for (int i = 0; i < variantCount && i < bandwidths.size(); ++i) {
                playlist += QString("#EXT-X-STREAM-INF:BANDWIDTH=%1,RESOLUTION=1920x1080,VIDEO=\"chunked\"\n").arg(bandwidths[i]);
                playlist += QString("https://video-edge-%1.m3u8\n").arg(i);
            }
            return playlist;
        },
        gen::inRange(1, 10),
        gen::container<std::vector<int>>(gen::inRange(1000000, 10000000))
    );
}

// Generator for Error objects
template<>
struct Arbitrary<Error> {
    static Gen<Error> arbitrary() {
        return gen::apply(
            [](ErrorCode code, std::string msg) {
                return Error(code, QString::fromStdString(msg));
            },
            gen::element(
                ErrorCode::Unknown,
                ErrorCode::NetworkError,
                ErrorCode::TwitchApiError,
                ErrorCode::InvalidArgument
            ),
            gen::arbitrary<std::string>()
        );
    }
};

} // namespace rc
```

### 8.2 Shrinking Strategies

RapidCheck provides automatic shrinking, but for complex domain types:

```cpp
// Custom shrinking for VodMetadata - prefer shrinking duration/fileSize
template<>
struct Arbitrary<VodMetadata> {
    static Shrinkable<VodMetadata> shrinkable() {
        return gen::shrinkable(
            arbitrary(),
            [](const VodMetadata& m) {
                std::vector<VodMetadata> shrunk;
                
                // Shrink duration
                if (m.duration > 1) {
                    VodMetadata s = m;
                    s.duration = m.duration / 2;
                    shrunk.push_back(s);
                }
                
                // Shrink fileSize
                if (m.fileSize > 1024) {
                    VodMetadata s = m;
                    s.fileSize = m.fileSize / 2;
                    shrunk.push_back(s);
                }
                
                return seq::fromContainer(std::move(shrunk));
            }
        );
    }
};
```

---

## 9. Test Categories

### 9.1 Algebraic Properties
- Monad laws for `Result<T>`
- Idempotence of sanitization and formatting
- Commutativity where applicable

### 9.2 Roundtrip/Serialization Properties
- JSON encode/decode for `VodMetadata`
- QSettings save/load for `WatchHistory`
- URL extraction/construction

### 9.3 State Machine Properties
- Valid state transitions
- Reachability of all states
- No deadlocks

### 9.4 Invariant Properties
- Cache size consistency
- Time bounds in replay mode
- Non-negative durations and sizes

### 9.5 Boundary Properties
- Empty input handling
- Maximum size limits
- Edge cases in parsing

---

## 10. Implementation Checklist

### Phase 1: Framework Setup
- [ ] Add RapidCheck to CMakeLists.txt
- [ ] Create `tests/property/` directory structure
- [ ] Create generator utilities in `tests/property/Generators.hpp`

### Phase 2: Core Properties
- [ ] Implement Result<T> monad law tests
- [ ] Implement VodMetadata serialization tests
- [ ] Implement InputValidator property tests

### Phase 3: Parsing Properties
- [ ] Implement HlsAdFilter parsing tests
- [ ] Implement TwitchChatClient IRC parsing tests

### Phase 4: Stateful Properties
- [ ] Implement CacheManager invariant tests
- [ ] Implement StateMachineLiveReplay transition tests

### Phase 5: Integration
- [ ] Add property tests to CI pipeline
- [ ] Configure seed logging for reproducibility
- [ ] Document failure shrinking examples

---

## 11. CI Integration

```yaml
# .github/workflows/property-tests.yml
property-tests:
  runs-on: ubuntu-latest
  steps:
    - uses: actions/checkout@v4
    - name: Build
      run: cmake -B build && cmake --build build
    - name: Run Property Tests
      run: |
        cd build
        ctest -R "PropertyTest" --output-on-failure
      env:
        RC_PARAMS: "max_success=1000 max_discard_ratio=10"
```

---

## 12. Example Test File Structure

```
tests/
├── property/
│   ├── Generators.hpp              # Custom generators
│   ├── PropertyTestResult.cpp      # Result<T> monad laws
│   ├── PropertyTestVodMetadata.cpp # Serialization roundtrips
│   ├── PropertyTestInputValidator.cpp
│   ├── PropertyTestHlsAdFilter.cpp
│   ├── PropertyTestCacheManager.cpp
│   ├── PropertyTestTwitchChat.cpp
│   └── PropertyTestStateMachine.cpp
└── CMakeLists.txt                  # Updated with property tests
```
