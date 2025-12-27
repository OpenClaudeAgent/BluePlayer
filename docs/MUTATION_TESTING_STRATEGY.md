# BluePlayer Advanced Mutation Testing Strategy

## Executive Summary

This document outlines an advanced mutation testing strategy for BluePlayer, a Twitch streaming client built with C++/Qt. Based on comprehensive source code analysis, we identify critical code paths, propose domain-specific mutation operators, and design an efficient mutation testing pipeline.

---

## 1. Current Setup Analysis

### 1.1 Existing Configuration (mull.yml)

Current mutators:
- `and_or_replacement` - Swaps && and ||
- `conditionals_boundary` - Changes >, <, >=, <=
- `math_add/div/mul/sub` - Arithmetic operator mutations
- `negate_condition` - Negates boolean conditions
- `remove_void_function` - Removes void function calls
- `replace_assignment` - Modifies assignments
- `replace_call` - Replaces function calls

### 1.2 Current Execution Script

The script (`run_mutation_tests.sh`):
- Requires LLVM 19 and Mull
- Uses `compile_commands.json` for context
- Generates HTML and JSON reports
- Runs with 4 threads/workers
- 30-second timeout per mutation

---

## 2. Source File Risk Categorization

### 2.1 HIGH RISK - Security & Data Integrity

| File | Risk Factors | Priority |
|------|--------------|----------|
| `SecureStorage.cpp:89-125` | Token encryption/decryption, XOR cipher | CRITICAL |
| `SecureStorage.cpp:31-87` | Key derivation, system UUID handling | CRITICAL |
| `TwitchAuthManager.cpp:349-411` | OAuth token refresh, state validation | CRITICAL |
| `TwitchAuthManager.cpp:413-436` | Authorization code handling, state match | CRITICAL |
| `TwitchAuthManager.cpp:641-669` | Token persistence with Client-ID | HIGH |
| `InputValidator.cpp:19-101` | URL/path/token validation | HIGH |

**Critical Mutations to Ensure Kill:**
```cpp
// SecureStorage.cpp:104 - XOR cipher
encrypted.append(data[i] ^ key[i % key.size()]);  // ^ -> &, |, +, - MUST BE KILLED

// TwitchAuthManager.cpp:419 - State validation
if (state != m_state) {  // != -> == MUST BE KILLED

// InputValidator.cpp:95-96 - Token length bounds
if (token.length() < 20 || token.length() > 2000) {  // < -> <=, > -> >= 
```

### 2.2 HIGH RISK - Financial/API Rate Limiting

| File | Risk Factors | Priority |
|------|--------------|----------|
| `TwitchApiClient.cpp:342-347` | Limit validation (1-100) | HIGH |
| `CacheManager.cpp:440-484` | Cache cleanup thresholds | HIGH |
| `NetworkCache.cpp:25-44` | TTL calculations | HIGH |

**Critical Mutations:**
```cpp
// TwitchApiClient.cpp:342
if (limit < 1 || limit > 100) {  // Boundary mutations critical

// CacheManager.cpp:445
if (usage < kCleanupTriggerThreshold * 100.0) {  // Threshold mutations
```

### 2.3 MEDIUM RISK - Business Logic & State Machines

| File | Risk Factors | Priority |
|------|--------------|----------|
| `StateMachineLiveReplay.cpp:21-28` | Replay state detection | MEDIUM-HIGH |
| `StateMachineLiveReplay.cpp:270-390` | State transition logic | MEDIUM-HIGH |
| `HlsAdFilter.cpp:289-356` | Ad detection patterns | MEDIUM |
| `TwitchChatClient.cpp:158-173` | Reconnection logic | MEDIUM |
| `TwitchService.cpp:935-1053` | Quality selection algorithm | MEDIUM |

**State Machine Mutations:**
```cpp
// StateMachineLiveReplay.cpp:23-27 - isInReplay()
return currentState == LiveReplayState::SeekbackPrep ||
       currentState == LiveReplayState::SeekbackActive ||  // || -> && MUST BE KILLED
       currentState == LiveReplayState::SeekbackSeeking ||
       currentState == LiveReplayState::ReplayPlaying ||
       currentState == LiveReplayState::ReplayPaused;

// StateMachineLiveReplay.cpp:30-33 - latency calculation
double live = m_liveEdgeTime.load(std::memory_order_acquire);
double current = m_currentTime.load(std::memory_order_acquire);
return live - current;  // - -> + MUST BE KILLED
```

### 2.4 MEDIUM RISK - Data Processing & Parsing

| File | Risk Factors | Priority |
|------|--------------|----------|
| `TwitchApiClient.cpp:439-460` | Stream array parsing | MEDIUM |
| `TwitchApiClient.cpp:535-557` | Clips array parsing | MEDIUM |
| `TwitchChatClient.cpp:206-285` | IRC message parsing | MEDIUM |
| `TwitchChatClient.cpp:287-307` | Tag parsing | MEDIUM |
| `TwitchChatClient.cpp:309-387` | Emote parsing | MEDIUM |

**Parsing Mutations:**
```cpp
// TwitchChatClient.cpp:344-345 - Position parsing
qsizetype start = range.left(dashIdx).toLongLong(&okStart);
qsizetype end = range.mid(dashIdx + 1).toLongLong(&okEnd);  // + 1 -> + 0

// TwitchChatClient.cpp:347
if (okStart && okEnd && start >= 0 && end >= start) {  // >= -> >
```

### 2.5 LOW RISK - UI Helpers & Formatters

| File | Risk Factors | Priority |
|------|--------------|----------|
| `CacheManager.cpp:185-191` | Size formatting | LOW |
| `Error.cpp:21-65` | Message localization | LOW |
| `Logger.cpp` | Debug output | LOW |
| `VodMetadata.hpp` | Data structures | LOW |

---

## 3. Domain-Specific Mutation Operators

### 3.1 HLS Playlist Parsing Mutations

```yaml
hls_playlist_mutations:
  - name: extinf_duration_mutation
    description: "Mutate EXTINF duration parsing"
    targets:
      - pattern: "#EXTINF:([\\d.]+),"
        mutations:
          - remove_decimal     # "10.5" -> "10"
          - negate_duration    # positive -> negative
          - zero_duration      # any -> 0
    files:
      - "HlsAdFilter.cpp"
      - "TwitchService.cpp"
  
  - name: ext_x_stream_inf_mutation
    description: "Mutate stream info parsing"
    targets:
      - bandwidth_extraction: "BANDWIDTH=(\\d+)"
      - resolution_extraction: "RESOLUTION=(\\d+)x(\\d+)"
    mutations:
      - swap_width_height     # 1920x1080 -> 1080x1920
      - zero_bandwidth        # any bandwidth -> 0
      - invalid_resolution    # remove x separator
    files:
      - "HlsAdFilter.cpp:209-259"
      - "TwitchService.cpp:935-1053"
  
  - name: ad_marker_mutation
    description: "Mutate ad detection patterns"
    targets:
      - ad_patterns: ["twitch-stitched-ad", "Amazon-Ads", "SCTE35-OUT"]
    mutations:
      - pattern_substring     # Full pattern -> partial match
      - case_flip             # Lower -> upper and vice versa
      - empty_pattern         # Pattern -> empty string
    files:
      - "HlsAdFilter.cpp:289-356"
```

### 3.2 OAuth Flow Mutations

```yaml
oauth_mutations:
  - name: pkce_challenge_mutation
    description: "Mutate PKCE code challenge generation"
    targets:
      - code_verifier_length: 64
      - hash_algorithm: SHA256
    mutations:
      - verifier_length_boundary  # 64 -> 63, 65
      - algorithm_swap            # SHA256 -> SHA1 (if testable)
    files:
      - "TwitchAuthManager.cpp:975-987"
  
  - name: token_expiration_mutation
    description: "Mutate token expiration handling"
    targets:
      - refresh_threshold: 300  # 5 minutes
      - default_expiry: 14400   # 4 hours
    mutations:
      - threshold_zero          # 300 -> 0
      - threshold_negative      # 300 -> -300
      - expiry_immediate        # 14400 -> 0
    files:
      - "TwitchAuthManager.cpp:877-892"
  
  - name: state_validation_mutation
    description: "Mutate OAuth state validation"
    targets:
      - state_comparison: "state != m_state"
    mutations:
      - always_match            # != -> ==
      - null_check_removal      # Skip empty check
      - case_insensitive        # Exact -> case-insensitive
    files:
      - "TwitchAuthManager.cpp:413-436"
```

### 3.3 WebSocket State Mutations

```yaml
websocket_mutations:
  - name: connection_state_mutation
    description: "Mutate WebSocket connection states"
    targets:
      - states: [Disconnected, Connecting, Connected, Error]
    mutations:
      - state_swap              # Connected <-> Connecting
      - skip_state              # Connecting -> Connected directly
      - error_ignore            # Error -> Connected
    files:
      - "TwitchChatClient.cpp:38-57"
  
  - name: reconnect_logic_mutation
    description: "Mutate reconnection handling"
    targets:
      - max_attempts: 3
      - delay_ms: 2000
    mutations:
      - zero_attempts           # 3 -> 0
      - infinite_attempts       # 3 -> MAX_INT
      - zero_delay              # 2000 -> 0
    files:
      - "TwitchChatClient.cpp:158-173"
  
  - name: irc_parsing_mutation
    description: "Mutate IRC message parsing"
    targets:
      - ping_response: "PONG :tmi.twitch.tv"
      - message_delimiters: ["\r\n", " :", "@", "!"]
    mutations:
      - wrong_pong_response     # PONG -> PING
      - delimiter_removal       # Remove delimiter handling
      - prefix_skip             # Skip @ or : handling
    files:
      - "TwitchChatClient.cpp:175-189"
      - "TwitchChatClient.cpp:206-285"
```

### 3.4 Cache Management Mutations

```yaml
cache_mutations:
  - name: lru_ordering_mutation
    description: "Mutate LRU cache ordering"
    targets:
      - sort_comparison: "aTime < bTime"
    mutations:
      - reverse_order           # < -> >
      - stable_sort_to_unstable # std::stable_sort -> std::sort
      - null_datetime_handling  # Different handling for invalid dates
    files:
      - "CacheManager.cpp:600-612"
  
  - name: threshold_mutation
    description: "Mutate cache threshold calculations"
    targets:
      - trigger_threshold: 0.90
      - target_threshold: 0.80
    mutations:
      - threshold_swap          # trigger <-> target
      - over_100_percent        # 0.90 -> 1.10
      - negative_threshold      # 0.90 -> -0.90
    files:
      - "CacheManager.cpp:286-287"
      - "CacheManager.cpp:442-484"
```

---

## 4. Mutation Testing Pipeline Design

### 4.1 Test-Mutation Mapping Strategy

```yaml
mutation_test_mapping:
  # Critical security paths - ALL tests must run
  security_critical:
    files:
      - "SecureStorage.cpp"
      - "TwitchAuthManager.cpp"
      - "InputValidator.cpp"
    tests:
      - "TestSecureStorage"
      - "TestTwitchAuthManager"
      - "TestInputValidator"
      - "TestTwitchFlow"  # Integration test
    strategy: exhaustive
    timeout: 60000

  # API client - Run targeted tests
  api_client:
    files:
      - "TwitchApiClient.cpp"
      - "ApiClientBase.cpp"
      - "HttpClient.cpp"
      - "CurlHttpClient.cpp"
    tests:
      - "TestTwitchApiClient"
      - "TestApiClientBase"
      - "TestHttpClient"
      - "TestCurlHttpClient"
    strategy: targeted
    timeout: 30000

  # State machine - Run state-focused tests
  state_machine:
    files:
      - "StateMachineLiveReplay.cpp"
    tests:
      - "TestStateMachineLiveReplay"
    strategy: targeted
    timeout: 30000

  # Media processing - Run media tests
  media:
    files:
      - "HlsAdFilter.cpp"
    tests:
      - "TestHlsAdFilter"
    strategy: targeted
    timeout: 45000

  # Chat client - Run chat tests
  chat:
    files:
      - "TwitchChatClient.cpp"
    tests:
      - "TestTwitchChatClient"
    strategy: targeted
    timeout: 30000

  # Cache management - Run cache tests
  cache:
    files:
      - "CacheManager.cpp"
      - "NetworkCache.cpp"
    tests:
      - "TestCacheManager"
      - "TestNetworkCache"
    strategy: targeted
    timeout: 30000
```

### 4.2 Parallel Execution Strategy

```yaml
parallel_execution:
  # Phase 1: Quick smoke test (fastest tests first)
  phase_1:
    name: "Smoke Test"
    parallel_jobs: 8
    files:
      - "Error.cpp"
      - "Logger.cpp"
      - "Config.cpp"
      - "InputValidator.cpp"
    max_mutations_per_file: 50
    timeout_per_mutation: 10000
    
  # Phase 2: Core business logic
  phase_2:
    name: "Core Logic"
    parallel_jobs: 4
    files:
      - "StateMachineLiveReplay.cpp"
      - "CacheManager.cpp"
      - "WatchHistory.cpp"
    max_mutations_per_file: 200
    timeout_per_mutation: 30000
    
  # Phase 3: Network and API
  phase_3:
    name: "Network Layer"
    parallel_jobs: 4
    files:
      - "HttpClient.cpp"
      - "CurlHttpClient.cpp"
      - "ApiClientBase.cpp"
      - "TwitchApiClient.cpp"
    max_mutations_per_file: 300
    timeout_per_mutation: 45000
    
  # Phase 4: Security critical (slower, more thorough)
  phase_4:
    name: "Security Critical"
    parallel_jobs: 2  # More isolation
    files:
      - "SecureStorage.cpp"
      - "TwitchAuthManager.cpp"
    max_mutations_per_file: 500
    timeout_per_mutation: 60000
    
  # Phase 5: Integration tests against all mutations
  phase_5:
    name: "Integration Validation"
    parallel_jobs: 2
    files:
      - "TwitchService.cpp"
    tests:
      - "TestTwitchFlow"
    max_mutations_per_file: 400
    timeout_per_mutation: 90000
```

### 4.3 Mutation Score Thresholds

```yaml
mutation_thresholds:
  # Module-specific thresholds
  modules:
    security:
      files:
        - "SecureStorage.cpp"
        - "TwitchAuthManager.cpp"
        - "InputValidator.cpp"
      min_score: 95  # CRITICAL - nearly all mutations must be killed
      alert_on: 90
      
    api_core:
      files:
        - "TwitchApiClient.cpp"
        - "ApiClientBase.cpp"
        - "HttpClient.cpp"
      min_score: 85
      alert_on: 80
      
    state_management:
      files:
        - "StateMachineLiveReplay.cpp"
        - "CacheManager.cpp"
      min_score: 90
      alert_on: 85
      
    media_processing:
      files:
        - "HlsAdFilter.cpp"
      min_score: 80
      alert_on: 75
      
    chat:
      files:
        - "TwitchChatClient.cpp"
      min_score: 80
      alert_on: 75
      
    infrastructure:
      files:
        - "Error.cpp"
        - "Logger.cpp"
        - "Config.cpp"
        - "NetworkCache.cpp"
        - "WatchHistory.cpp"
      min_score: 70
      alert_on: 65

  # Overall project threshold
  project:
    min_score: 82
    alert_on: 78
```

---

## 5. CI Integration Plan

### 5.1 GitHub Actions Workflow

```yaml
# .github/workflows/mutation-testing.yml
name: Mutation Testing

on:
  push:
    branches: [main, develop]
    paths:
      - 'src/**/*.cpp'
      - 'src/**/*.hpp'
  pull_request:
    branches: [main]
    paths:
      - 'src/**/*.cpp'
      - 'src/**/*.hpp'
  schedule:
    - cron: '0 2 * * 0'  # Weekly full run on Sunday at 2 AM

env:
  LLVM_VERSION: 19
  MULL_VERSION: 14.0.0

jobs:
  # Quick mutation check for PRs
  quick-mutation-check:
    if: github.event_name == 'pull_request'
    runs-on: macos-14
    timeout-minutes: 30
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup LLVM
        run: brew install llvm@19
        
      - name: Setup Mull
        run: |
          curl -L https://github.com/mull-project/mull/releases/download/${{ env.MULL_VERSION }}/mull-${{ env.MULL_VERSION }}-macos.tar.gz | tar xz
          sudo mv mull-runner-19 /usr/local/bin/
          
      - name: Get changed files
        id: changed-files
        uses: tj-actions/changed-files@v42
        with:
          files: |
            src/**/*.cpp
            src/**/*.hpp
            
      - name: Configure and Build
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_CXX_COMPILER=$(brew --prefix llvm@19)/bin/clang++ \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_BUILD_TYPE=Debug
          cmake --build build
          
      - name: Run targeted mutation tests
        run: |
          # Only test changed files
          CHANGED="${{ steps.changed-files.outputs.all_changed_files }}"
          ./scripts/run_mutation_tests.sh --files "$CHANGED" --quick
          
      - name: Check mutation score
        run: |
          SCORE=$(jq '.mutation_score' mutation-reports/mutation-report.json)
          if (( $(echo "$SCORE < 78" | bc -l) )); then
            echo "::error::Mutation score $SCORE% is below threshold (78%)"
            exit 1
          fi

  # Full mutation testing (scheduled and main branch)
  full-mutation-test:
    if: github.event_name == 'schedule' || (github.event_name == 'push' && github.ref == 'refs/heads/main')
    runs-on: macos-14
    timeout-minutes: 180
    strategy:
      matrix:
        phase: [1, 2, 3, 4, 5]
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup environment
        run: |
          brew install llvm@19 ninja
          
      - name: Configure and Build
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_CXX_COMPILER=$(brew --prefix llvm@19)/bin/clang++ \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_BUILD_TYPE=Debug
          cmake --build build
          
      - name: Run mutation phase ${{ matrix.phase }}
        run: ./scripts/run_mutation_tests.sh --phase ${{ matrix.phase }}
        
      - name: Upload mutation report
        uses: actions/upload-artifact@v4
        with:
          name: mutation-report-phase-${{ matrix.phase }}
          path: mutation-reports/
          
  # Aggregate results
  aggregate-mutation-results:
    needs: full-mutation-test
    runs-on: ubuntu-latest
    steps:
      - uses: actions/download-artifact@v4
        with:
          pattern: mutation-report-phase-*
          merge-multiple: true
          
      - name: Aggregate and analyze
        run: |
          python3 scripts/aggregate_mutation_results.py
          
      - name: Post summary to PR
        if: github.event_name == 'pull_request'
        uses: actions/github-script@v7
        with:
          script: |
            const fs = require('fs');
            const summary = fs.readFileSync('mutation-summary.md', 'utf8');
            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: summary
            });
```

### 5.2 Enhanced Run Script

```bash
#!/bin/bash
# scripts/run_mutation_tests_advanced.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
REPORTS_DIR="$PROJECT_ROOT/mutation-reports"

# Parse arguments
PHASE=""
QUICK=false
FILES=""

while [[ $# -gt 0 ]]; do
  case $1 in
    --phase) PHASE="$2"; shift 2 ;;
    --quick) QUICK=true; shift ;;
    --files) FILES="$2"; shift 2 ;;
    *) shift ;;
  esac
done

# Define phase configurations
declare -A PHASE_CONFIG=(
  [1]="Error.cpp Logger.cpp Config.cpp InputValidator.cpp"
  [2]="StateMachineLiveReplay.cpp CacheManager.cpp WatchHistory.cpp"
  [3]="HttpClient.cpp CurlHttpClient.cpp ApiClientBase.cpp TwitchApiClient.cpp"
  [4]="SecureStorage.cpp TwitchAuthManager.cpp"
  [5]="TwitchService.cpp"
)

declare -A PHASE_TIMEOUT=(
  [1]=10000
  [2]=30000
  [3]=45000
  [4]=60000
  [5]=90000
)

declare -A PHASE_JOBS=(
  [1]=8
  [2]=4
  [3]=4
  [4]=2
  [5]=2
)

# Build filter for specific files
build_file_filter() {
  local files="$1"
  local filter=""
  for f in $files; do
    if [[ -n "$filter" ]]; then
      filter="$filter|$f"
    else
      filter="$f"
    fi
  done
  echo "$filter"
}

# Run Mull with configuration
run_mull() {
  local timeout=$1
  local jobs=$2
  local include_filter=$3
  
  mkdir -p "$REPORTS_DIR"
  
  local mull_cmd="mull-runner-19"
  mull_cmd="$mull_cmd --config $PROJECT_ROOT/mull.yml"
  mull_cmd="$mull_cmd --compilation-database $BUILD_DIR/compile_commands.json"
  mull_cmd="$mull_cmd --timeout $timeout"
  mull_cmd="$mull_cmd --workers $jobs"
  mull_cmd="$mull_cmd --reporters=IDE"
  mull_cmd="$mull_cmd --reporters=HTML:$REPORTS_DIR"
  mull_cmd="$mull_cmd --reporters=JSON:$REPORTS_DIR/mutation-report.json"
  
  if [[ -n "$include_filter" ]]; then
    mull_cmd="$mull_cmd --include-path '$include_filter'"
  fi
  
  # Find test binaries
  local test_bins=$(find "$BUILD_DIR/tests" -type f -perm +111 -not -name "*.dylib" | tr '\n' ' ')
  mull_cmd="$mull_cmd $test_bins"
  
  echo "Running: $mull_cmd"
  eval "$mull_cmd" || true
}

# Main execution
if [[ "$QUICK" == true ]]; then
  echo "=== Quick Mutation Test ==="
  if [[ -n "$FILES" ]]; then
    filter=$(build_file_filter "$FILES")
  else
    filter=$(build_file_filter "${PHASE_CONFIG[1]}")
  fi
  run_mull 15000 8 "$filter"
  
elif [[ -n "$PHASE" ]]; then
  echo "=== Mutation Test Phase $PHASE ==="
  filter=$(build_file_filter "${PHASE_CONFIG[$PHASE]}")
  run_mull "${PHASE_TIMEOUT[$PHASE]}" "${PHASE_JOBS[$PHASE]}" "$filter"
  
else
  echo "=== Full Mutation Test ==="
  for phase in 1 2 3 4 5; do
    echo "--- Phase $phase ---"
    filter=$(build_file_filter "${PHASE_CONFIG[$phase]}")
    run_mull "${PHASE_TIMEOUT[$phase]}" "${PHASE_JOBS[$phase]}" "$filter"
  done
fi

# Generate summary
if [[ -f "$REPORTS_DIR/mutation-report.json" ]]; then
  echo ""
  echo "=== Mutation Testing Summary ==="
  jq -r '
    "Total Mutants: \(.total_mutants)",
    "Killed: \(.killed) (\(.mutation_score)%)",
    "Survived: \(.survived)",
    "Timeout: \(.timeout)",
    "Skipped: \(.skipped)"
  ' "$REPORTS_DIR/mutation-report.json"
fi
```

---

## 6. Specific Test Recommendations

### 6.1 Tests Required for High-Risk Mutations

```cpp
// Tests for SecureStorage.cpp encryption
TEST_CASE("SecureStorage encryption is reversible") {
  SecureStorage storage;
  QString original = "test_oauth_token_12345";
  storage.store("test_key", original);
  REQUIRE(storage.retrieve("test_key") == original);
}

TEST_CASE("SecureStorage with empty string") {
  SecureStorage storage;
  storage.store("empty", "");
  REQUIRE(storage.retrieve("empty") == "");
}

TEST_CASE("SecureStorage key derivation is deterministic") {
  SecureStorage storage1, storage2;
  storage1.store("key", "value");
  REQUIRE(storage2.retrieve("key") == "value");
}

// Tests for TwitchAuthManager state validation
TEST_CASE("OAuth state mismatch is rejected") {
  TwitchAuthManager auth;
  // Simulate callback with wrong state
  QUrl callback("https://127.0.0.1:8443/callback?state=wrong&code=valid");
  // Should emit error, not authenticate
}

TEST_CASE("Token expiration boundary - exactly 5 minutes") {
  TwitchAuthManager auth;
  // Set token expiring in exactly 300 seconds
  // Should trigger refresh
}

// Tests for StateMachineLiveReplay
TEST_CASE("isInReplay returns true for all replay states") {
  StateMachineLiveReplay sm;
  // Test each replay state individually
  for (auto state : {SeekbackPrep, SeekbackActive, SeekbackSeeking, 
                     ReplayPlaying, ReplayPaused}) {
    // Set state and verify isInReplay() == true
  }
}

TEST_CASE("latency calculation is correct") {
  StateMachineLiveReplay sm;
  sm.updateLiveEdgeTime(100.0);
  sm.updateCurrentTime(95.0);
  REQUIRE(sm.latency() == 5.0);  // Must be live - current, not current - live
}
```

### 6.2 Boundary Tests for Critical Values

```cpp
// TwitchApiClient limit validation
TEST_CASE("listFollowedStreams rejects invalid limits") {
  TwitchApiClient client("test_id");
  
  SECTION("limit = 0") {
    client.listFollowedStreams("user", 0);
    // Should emit error
  }
  
  SECTION("limit = 1 (boundary)") {
    client.listFollowedStreams("user", 1);
    // Should succeed
  }
  
  SECTION("limit = 100 (boundary)") {
    client.listFollowedStreams("user", 100);
    // Should succeed
  }
  
  SECTION("limit = 101") {
    client.listFollowedStreams("user", 101);
    // Should emit error
  }
}

// CacheManager threshold tests
TEST_CASE("Cache cleanup triggers at correct threshold") {
  CacheManager cache;
  cache.setMaxCacheSize(1000);
  
  SECTION("At 89% - no cleanup") {
    // Add 890 bytes
    REQUIRE(cache.cacheUsagePercent() < 90.0);
    // performCleanup should return 0
  }
  
  SECTION("At 90% - triggers cleanup") {
    // Add 900 bytes
    REQUIRE(cache.cacheUsagePercent() >= 90.0);
    // performCleanup should remove items
  }
  
  SECTION("Cleanup targets 80%") {
    // Add 950 bytes, trigger cleanup
    // Should remove until <= 80%
    REQUIRE(cache.cacheUsagePercent() <= 80.0);
  }
}
```

---

## 7. Expected Mutation Score Targets

| Module | Current (Est.) | Target | Timeline |
|--------|----------------|--------|----------|
| Security (SecureStorage, Auth) | 70% | 95% | 2 weeks |
| Input Validation | 75% | 95% | 1 week |
| State Machine | 65% | 90% | 2 weeks |
| API Client | 60% | 85% | 3 weeks |
| Media (HlsAdFilter) | 55% | 80% | 2 weeks |
| Chat Client | 50% | 80% | 2 weeks |
| Cache Management | 65% | 85% | 2 weeks |
| **Overall Project** | **~62%** | **82%** | **4 weeks** |

---

## 8. Implementation Roadmap

### Week 1: Foundation
1. Update `mull.yml` with new mutators
2. Create enhanced `run_mutation_tests_advanced.sh`
3. Run baseline mutation testing
4. Identify lowest-scoring files

### Week 2: Security Focus
1. Add missing tests for SecureStorage
2. Add OAuth state validation tests
3. Add token boundary tests
4. Achieve 95% on security modules

### Week 3: Core Logic
1. Add StateMachineLiveReplay tests
2. Add CacheManager threshold tests
3. Add HlsAdFilter ad detection tests
4. Achieve 85% on core modules

### Week 4: Integration & CI
1. Set up GitHub Actions workflow
2. Implement PR mutation checking
3. Add scheduled full runs
4. Create reporting dashboard

---

## 9. Appendix: mull.yml Enhancement

```yaml
# Enhanced mull.yml for BluePlayer
mutators:
  # Standard mutations
  - and_or_replacement
  - conditionals_boundary
  - conditionals_negation
  - math_add
  - math_div
  - math_mul
  - math_sub
  - negate_condition
  - remove_void_function
  - replace_assignment
  - replace_call
  
  # Additional mutations for thorough testing
  - scalar_value_mutator   # Change constants
  - remove_negation        # Remove ! operators
  
excludePaths:
  - "**/moc_*.cpp"
  - "**/qrc_*.cpp"
  - "**/uic_*.cpp"
  - "**/*_autogen/**"
  - "**/tests/**"
  - "**/main.mm"
  - "**/main.cpp"
  # Exclude low-priority UI files
  - "**/themes/**"
  - "**/*.qml"

includePaths:
  - "src/core/**"
  - "src/media/**"
  - "src/api/**"
  - "src/chat/**"
  - "src/ui/*.cpp"  # Only C++ view models

# Tiered timeout based on file complexity
timeout: 30000  # Default
# Use script for file-specific timeouts

threads: 4
workers: 4
reporter: IDE
dryRun: false

# Enable mutation caching for faster reruns
enableMutationCaching: true
mutationCachePath: ".mutation-cache"
```

---

## 10. Conclusion

This mutation testing strategy provides:

1. **Risk-based prioritization** - Focus testing resources on security and data integrity
2. **Domain-specific operators** - HLS, OAuth, WebSocket mutations tailored for streaming apps
3. **Efficient execution** - Phased parallel approach with targeted test mapping
4. **Clear thresholds** - Module-specific goals with measurable targets
5. **CI integration** - Automated enforcement with PR blocking

Implementing this strategy will significantly improve test quality and code reliability for BluePlayer.
