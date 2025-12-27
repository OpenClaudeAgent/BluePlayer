# BluePlayer Test Metrics and Quality Gates Strategy

## Executive Summary

This document defines a comprehensive multi-dimensional test quality system for BluePlayer, 
covering code coverage, mutation testing, execution metrics, and CI/CD integration.

---

## 1. Multi-Dimensional Test Quality Dashboard

### 1.1 Coverage Metrics Architecture

```
+------------------------------------------------------------------+
|                    TEST QUALITY DASHBOARD                         |
+------------------------------------------------------------------+
|  CODE COVERAGE          |  MUTATION SCORE       |  EXECUTION      |
|  Line: 85.2%            |  Overall: 72.1%       |  Time: 45.3s    |
|  Branch: 78.4%          |  Core: 81.2%          |  Trend: -2.1s   |
|  Function: 91.3%        |  Media: 65.4%         |  Flaky: 0       |
+-------------------------+-----------------------+-----------------+
|  TEST HEALTH            |  MAINTENANCE          |  TEST-TO-CODE   |
|  Complexity: Low        |  Burden: 12%          |  Ratio: 1.04    |
|  Duplication: 3.2%      |  Age: 45 days avg     |  LOC: 8.2K/7.9K |
+------------------------------------------------------------------+
```

### 1.2 Coverage Dimensions

| Dimension | Description | Tool | Target |
|-----------|-------------|------|--------|
| Line Coverage | % of executed lines | llvm-cov | 80% |
| Branch Coverage | % of decision branches taken | llvm-cov | 75% |
| Function Coverage | % of functions called | llvm-cov | 90% |
| Region Coverage | % of code regions executed | llvm-cov | 75% |
| Condition Coverage | % of boolean conditions | llvm-cov | 70% |

### 1.3 Mutation Score by Module

| Module | Files | Mutators Applied | Target Score |
|--------|-------|------------------|--------------|
| `core/` | 13 | All 10 mutators | 75% |
| `core/network/` | 3 | All 10 mutators | 80% |
| `api/twitch/` | 3 | All 10 mutators | 75% |
| `media/` | 3 | Subset (no remove_void) | 65% |
| `chat/` | 1 | All 10 mutators | 70% |
| `ui/` (ViewModels) | 2 | All 10 mutators | 70% |

---

## 2. Quality Gates Specification

### 2.1 Coverage Thresholds by Module

```yaml
quality_gates:
  global:
    line_coverage: 70%      # Current threshold (validate_build.sh)
    branch_coverage: 65%
    function_coverage: 80%
    
  modules:
    core:
      line: 85%
      branch: 80%
      function: 95%
      critical: true
      
    core/network:
      line: 80%
      branch: 75%
      function: 90%
      critical: true
      
    api/twitch:
      line: 80%
      branch: 70%
      function: 90%
      critical: true
      
    media:
      line: 60%           # Lower due to hardware dependencies
      branch: 55%
      function: 75%
      critical: false
      
    chat:
      line: 75%
      branch: 70%
      function: 85%
      critical: false
      
    ui:
      line: 70%
      branch: 65%
      function: 80%
      critical: false
```

### 2.2 Mutation Score Requirements

```yaml
mutation_gates:
  global:
    minimum_score: 60%
    target_score: 75%
    
  per_module:
    core: 75%
    core/network: 80%
    api/twitch: 75%
    media: 60%
    chat: 70%
    ui: 65%
    
  mutator_effectiveness:
    negate_condition: 80%    # Most critical
    conditionals_boundary: 75%
    math_operations: 70%
    remove_void_function: 65%
    replace_call: 60%
```

### 2.3 Test Execution Constraints

```yaml
execution_gates:
  unit_tests:
    max_total_time: 60s      # All unit tests combined
    max_single_test: 5s      # Any individual test
    warning_threshold: 45s
    
  integration_tests:
    max_total_time: 120s
    max_single_test: 30s
    warning_threshold: 90s
    
  flaky_policy:
    max_allowed: 0           # Zero tolerance
    quarantine_after: 2      # Failures before quarantine
    auto_retry: 3            # Retries before marking flaky
```

### 2.4 Test-to-Code Ratio

```yaml
ratio_gates:
  minimum_ratio: 0.8         # test LOC / source LOC
  target_ratio: 1.2
  warning_ratio: 0.9
  
  per_module_minimum:
    core: 1.0
    core/network: 1.0
    api/twitch: 1.2
    media: 0.6
    chat: 0.8
    ui: 0.8
```

---

## 3. Critical Path Coverage Strategy

### 3.1 100% Coverage Requirements

These code paths MUST have 100% line and branch coverage:

```cpp
// CRITICAL: Authentication flow
class TwitchAuthManager {
    // All public methods: 100% coverage required
    bool authenticate();
    bool refreshToken();
    bool validateToken();
    void logout();
};

// CRITICAL: Error handling core
class ErrorHandler {
    void handleError(const Error& error);
    void handleNetworkError(int statusCode);
    void handleApiError(const QString& message);
};

// CRITICAL: Secure storage
class SecureStorage {
    bool store(const QString& key, const QString& value);
    QString retrieve(const QString& key);
    bool remove(const QString& key);
};

// CRITICAL: Input validation
class InputValidator {
    static bool validateUrl(const QString& url);
    static bool validateUsername(const QString& username);
    static bool sanitizeInput(QString& input);
};
```

### 3.2 Critical Path Matrix

| Component | Methods | Coverage Required | Mutation Score |
|-----------|---------|-------------------|----------------|
| `TwitchAuthManager` | 8 | 100% line, 100% branch | 85% |
| `SecureStorage` | 6 | 100% line, 100% branch | 80% |
| `InputValidator` | 12 | 100% line, 100% branch | 90% |
| `ErrorHandler` | 10 | 100% line, 95% branch | 80% |
| `HttpClient` | 8 | 95% line, 90% branch | 75% |
| `CurlHttpClient` | 6 | 90% line, 85% branch | 75% |

### 3.3 Edge Case Coverage Strategy

```yaml
edge_case_categories:
  boundary_values:
    - empty_strings: ["", " ", "\t", "\n"]
    - null_equivalents: [nullptr, nullopt, QJsonValue::Null]
    - numeric_limits: [0, -1, INT_MAX, INT_MIN]
    - size_limits: [0, 1, MAX_SIZE - 1, MAX_SIZE]
    
  error_conditions:
    - network_errors: [timeout, connection_refused, dns_failure]
    - api_errors: [401, 403, 404, 429, 500, 503]
    - parse_errors: [invalid_json, missing_fields, wrong_types]
    - state_errors: [not_authenticated, expired_token, revoked_access]
    
  async_edge_cases:
    - timing: [immediate_response, delayed_response, timeout]
    - cancellation: [cancel_before_start, cancel_during, cancel_after]
    - retry: [retry_success, retry_exhaust, partial_success]
```

### 3.4 Error Handling Coverage Matrix

| Error Type | Test Scenarios | Min Coverage |
|------------|----------------|--------------|
| Network Timeout | 5 scenarios | 100% |
| HTTP 4xx Errors | 8 scenarios | 100% |
| HTTP 5xx Errors | 4 scenarios | 100% |
| JSON Parse Errors | 6 scenarios | 100% |
| Authentication Errors | 7 scenarios | 100% |
| Rate Limiting | 3 scenarios | 100% |
| Connection Errors | 5 scenarios | 100% |

### 3.5 Async Code Coverage Strategy

```cpp
// Required async test patterns for each async operation:

// Pattern 1: Success path
TEST_F(AsyncTest, Operation_Success) {
    QSignalSpy successSpy(&obj, &Class::operationSucceeded);
    obj.startOperation();
    ASSERT_TRUE(successSpy.wait(5000));
}

// Pattern 2: Failure path
TEST_F(AsyncTest, Operation_Failure) {
    QSignalSpy errorSpy(&obj, &Class::operationFailed);
    mockServer.setNextResponse(500);
    obj.startOperation();
    ASSERT_TRUE(errorSpy.wait(5000));
}

// Pattern 3: Cancellation
TEST_F(AsyncTest, Operation_Cancellation) {
    obj.startOperation();
    obj.cancel();
    // Verify clean cancellation
}

// Pattern 4: Timeout
TEST_F(AsyncTest, Operation_Timeout) {
    QSignalSpy timeoutSpy(&obj, &Class::operationTimeout);
    mockServer.setResponseDelay(10000);
    obj.startOperation();
    ASSERT_TRUE(timeoutSpy.wait(6000));
}

// Pattern 5: Retry logic
TEST_F(AsyncTest, Operation_RetrySuccess) {
    mockServer.failFirst(2);
    QSignalSpy successSpy(&obj, &Class::operationSucceeded);
    obj.startOperation();
    ASSERT_TRUE(successSpy.wait(15000));
}
```

---

## 4. Test Health Metrics

### 4.1 Test Code Quality Metrics

```yaml
test_quality_metrics:
  complexity:
    max_cyclomatic_per_test: 5
    max_nesting_depth: 3
    max_assertions_per_test: 10
    max_lines_per_test: 50
    
  duplication:
    max_duplicate_blocks: 5%
    min_block_size: 6 lines
    tool: CPD (PMD Copy-Paste Detector)
    
  naming:
    pattern: "test_<Unit>_<Scenario>_<ExpectedResult>"
    max_name_length: 80
    require_descriptive: true
    
  organization:
    max_tests_per_file: 30
    require_fixture_class: true
    require_setup_teardown: true
```

### 4.2 Test Maintenance Burden Index

```yaml
maintenance_metrics:
  age_analysis:
    track_last_modified: true
    stale_threshold: 90 days
    review_required: 180 days
    
  churn_analysis:
    high_churn_threshold: 5 changes/month
    review_high_churn: true
    
  dependency_coupling:
    max_test_dependencies: 5
    prefer_mocks: true
    avoid_integration_in_unit: true
    
  flakiness_score:
    window: 30 days
    flaky_threshold: 2 failures
    quarantine_threshold: 3 failures
```

### 4.3 Test Execution Reliability

```yaml
reliability_metrics:
  pass_rate:
    target: 100%
    minimum: 99%
    rolling_window: 14 days
    
  determinism:
    require_seed_control: true
    require_isolation: true
    require_cleanup: true
    
  resource_leaks:
    detect_memory_leaks: true
    detect_file_handles: true
    detect_network_connections: true
    
  timing_stability:
    max_variance: 20%
    detect_timing_drift: true
```

### 4.4 Time-to-Feedback Optimization

```yaml
feedback_optimization:
  local_development:
    target_unit_test_time: 30s
    incremental_test_time: 10s
    
  pre_commit:
    target_time: 60s
    run_affected_only: true
    
  pull_request:
    target_time: 180s
    parallel_execution: true
    
  nightly:
    target_time: 600s
    include_slow_tests: true
    include_mutation: true
```

---

## 5. Dashboard Implementation

### 5.1 Metrics Collection Script

```bash
#!/bin/bash
# scripts/collect_test_metrics.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
METRICS_DIR="$PROJECT_ROOT/metrics"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

mkdir -p "$METRICS_DIR/history"

echo "=== Collecting Test Metrics ==="

# 1. Coverage metrics
echo "Collecting coverage metrics..."
COVERAGE_JSON="$METRICS_DIR/coverage_$TIMESTAMP.json"

llvm-cov export \
    -instr-profile="$PROJECT_ROOT/build/default.profdata" \
    -object "$PROJECT_ROOT/build/tests/test_*" \
    -format=text \
    > "$COVERAGE_JSON"

# Extract summary
LINE_COV=$(jq '.data[0].totals.lines.percent' "$COVERAGE_JSON")
BRANCH_COV=$(jq '.data[0].totals.branches.percent' "$COVERAGE_JSON")
FUNC_COV=$(jq '.data[0].totals.functions.percent' "$COVERAGE_JSON")

# 2. Test execution time
echo "Collecting execution metrics..."
TIMING_JSON="$METRICS_DIR/timing_$TIMESTAMP.json"

ctest --test-dir "$PROJECT_ROOT/build" \
    --output-on-failure \
    --output-junit "$METRICS_DIR/junit_$TIMESTAMP.xml" \
    2>&1 | tee "$METRICS_DIR/ctest_output.log"

# Parse timing from CTest output
TOTAL_TIME=$(grep "Total Test time" "$METRICS_DIR/ctest_output.log" | \
    awk '{print $NF}' | sed 's/s//')

# 3. Test-to-code ratio
echo "Calculating test-to-code ratio..."
SRC_LOC=$(find "$PROJECT_ROOT/src" -name "*.cpp" -o -name "*.hpp" | \
    xargs wc -l | tail -1 | awk '{print $1}')
TEST_LOC=$(find "$PROJECT_ROOT/tests" -name "*.cpp" -o -name "*.hpp" | \
    xargs wc -l | tail -1 | awk '{print $1}')
RATIO=$(echo "scale=2; $TEST_LOC / $SRC_LOC" | bc)

# 4. Generate metrics JSON
cat > "$METRICS_DIR/dashboard_$TIMESTAMP.json" << EOF
{
  "timestamp": "$TIMESTAMP",
  "coverage": {
    "line": $LINE_COV,
    "branch": $BRANCH_COV,
    "function": $FUNC_COV
  },
  "execution": {
    "total_time_seconds": $TOTAL_TIME,
    "test_count": $(grep -c "Test #" "$METRICS_DIR/ctest_output.log" || echo 0)
  },
  "codebase": {
    "source_loc": $SRC_LOC,
    "test_loc": $TEST_LOC,
    "ratio": $RATIO
  },
  "quality_gates": {
    "coverage_passed": $([ $(echo "$LINE_COV >= 70" | bc) -eq 1 ] && echo "true" || echo "false"),
    "timing_passed": $([ $(echo "$TOTAL_TIME <= 60" | bc) -eq 1 ] && echo "true" || echo "false")
  }
}
EOF

# Archive to history
cp "$METRICS_DIR/dashboard_$TIMESTAMP.json" "$METRICS_DIR/history/"

echo ""
echo "=== Metrics Summary ==="
echo "Line Coverage:     ${LINE_COV}%"
echo "Branch Coverage:   ${BRANCH_COV}%"
echo "Function Coverage: ${FUNC_COV}%"
echo "Execution Time:    ${TOTAL_TIME}s"
echo "Test-to-Code:      ${RATIO}"
echo ""
echo "Dashboard saved: $METRICS_DIR/dashboard_$TIMESTAMP.json"
```

### 5.2 Module-Level Coverage Report

```bash
#!/bin/bash
# scripts/module_coverage_report.sh

MODULES=(
    "core"
    "core/network"
    "api/twitch"
    "media"
    "chat"
    "ui"
)

THRESHOLDS=(
    85  # core
    80  # core/network
    80  # api/twitch
    60  # media
    75  # chat
    70  # ui
)

echo "=== Module Coverage Report ==="
echo ""
printf "%-20s %10s %10s %10s %10s\n" "Module" "Line%" "Branch%" "Func%" "Status"
echo "--------------------------------------------------------------"

for i in "${!MODULES[@]}"; do
    MODULE="${MODULES[$i]}"
    THRESHOLD="${THRESHOLDS[$i]}"
    
    # Extract module-specific coverage
    LINE=$(llvm-cov report \
        -instr-profile="build/default.profdata" \
        build/tests/test_* \
        "src/$MODULE" 2>/dev/null | \
        grep "TOTAL" | awk '{print $(NF-2)}' | sed 's/%//')
    
    BRANCH=$(llvm-cov report \
        -instr-profile="build/default.profdata" \
        build/tests/test_* \
        "src/$MODULE" 2>/dev/null | \
        grep "TOTAL" | awk '{print $(NF-1)}' | sed 's/%//')
    
    FUNC=$(llvm-cov report \
        -instr-profile="build/default.profdata" \
        build/tests/test_* \
        "src/$MODULE" 2>/dev/null | \
        grep "TOTAL" | awk '{print $NF}' | sed 's/%//')
    
    if [ $(echo "$LINE >= $THRESHOLD" | bc) -eq 1 ]; then
        STATUS="PASS"
    else
        STATUS="FAIL"
    fi
    
    printf "%-20s %10s %10s %10s %10s\n" \
        "$MODULE" "${LINE:-N/A}%" "${BRANCH:-N/A}%" "${FUNC:-N/A}%" "$STATUS"
done

echo ""
```

### 5.3 Trend Analysis

```bash
#!/bin/bash
# scripts/coverage_trend.sh

METRICS_DIR="metrics/history"
DAYS=${1:-7}

echo "=== Coverage Trend (Last $DAYS days) ==="
echo ""

# Get recent metrics files
FILES=$(find "$METRICS_DIR" -name "dashboard_*.json" -mtime -$DAYS | sort)

printf "%-20s %10s %10s %10s %10s\n" "Date" "Line%" "Branch%" "Time(s)" "Status"
echo "--------------------------------------------------------------"

for f in $FILES; do
    DATE=$(echo "$f" | grep -oE '[0-9]{8}' | head -1)
    FORMATTED_DATE="${DATE:0:4}-${DATE:4:2}-${DATE:6:2}"
    
    LINE=$(jq -r '.coverage.line' "$f")
    BRANCH=$(jq -r '.coverage.branch' "$f")
    TIME=$(jq -r '.execution.total_time_seconds' "$f")
    PASSED=$(jq -r '.quality_gates.coverage_passed' "$f")
    
    STATUS=$([ "$PASSED" = "true" ] && echo "PASS" || echo "FAIL")
    
    printf "%-20s %10.1f %10.1f %10.1f %10s\n" \
        "$FORMATTED_DATE" "$LINE" "$BRANCH" "$TIME" "$STATUS"
done

echo ""

# Calculate trend
FIRST_LINE=$(echo "$FILES" | head -1 | xargs jq -r '.coverage.line')
LAST_LINE=$(echo "$FILES" | tail -1 | xargs jq -r '.coverage.line')
TREND=$(echo "scale=1; $LAST_LINE - $FIRST_LINE" | bc)

if [ $(echo "$TREND >= 0" | bc) -eq 1 ]; then
    echo "Trend: +${TREND}% (improving)"
else
    echo "Trend: ${TREND}% (declining)"
fi
```

---

## 6. Flaky Test Detection System

### 6.1 Flaky Test Detector

```bash
#!/bin/bash
# scripts/detect_flaky_tests.sh

RUNS=${1:-5}
BUILD_DIR="build"
FLAKY_LOG="metrics/flaky_tests.log"

echo "=== Flaky Test Detection (${RUNS} runs) ==="
echo ""

declare -A TEST_RESULTS

# Run tests multiple times
for run in $(seq 1 $RUNS); do
    echo "Run $run/$RUNS..."
    
    ctest --test-dir "$BUILD_DIR" \
        --output-on-failure \
        --no-compress-output \
        -T Test 2>&1 | while read line; do
        
        if echo "$line" | grep -q "Test #"; then
            TEST_NAME=$(echo "$line" | grep -oE "Test #[0-9]+: [^ ]+" | \
                awk -F': ' '{print $2}')
            
            if echo "$line" | grep -q "Passed"; then
                TEST_RESULTS["$TEST_NAME"]="${TEST_RESULTS[$TEST_NAME]}P"
            elif echo "$line" | grep -q "Failed"; then
                TEST_RESULTS["$TEST_NAME"]="${TEST_RESULTS[$TEST_NAME]}F"
            fi
        fi
    done
done

# Analyze results
echo ""
echo "=== Flaky Test Report ==="
echo ""

FLAKY_COUNT=0
echo "Test Name,Pass Count,Fail Count,Status" > "$FLAKY_LOG"

for test in "${!TEST_RESULTS[@]}"; do
    RESULTS="${TEST_RESULTS[$test]}"
    PASS_COUNT=$(echo "$RESULTS" | grep -o "P" | wc -l)
    FAIL_COUNT=$(echo "$RESULTS" | grep -o "F" | wc -l)
    
    if [ $PASS_COUNT -gt 0 ] && [ $FAIL_COUNT -gt 0 ]; then
        echo "FLAKY: $test (P:$PASS_COUNT F:$FAIL_COUNT)"
        echo "$test,$PASS_COUNT,$FAIL_COUNT,FLAKY" >> "$FLAKY_LOG"
        ((FLAKY_COUNT++))
    else
        echo "$test,$PASS_COUNT,$FAIL_COUNT,STABLE" >> "$FLAKY_LOG"
    fi
done

echo ""
if [ $FLAKY_COUNT -eq 0 ]; then
    echo "No flaky tests detected."
    exit 0
else
    echo "Detected $FLAKY_COUNT flaky test(s)!"
    exit 1
fi
```

### 6.2 Flaky Test Quarantine

```yaml
# .github/flaky-tests.yml
quarantine:
  enabled: true
  max_quarantine_days: 14
  auto_unquarantine_after: 10 consecutive passes
  
  tests: []  # Currently empty - zero tolerance policy
  
  # Example quarantine entry:
  # - name: TwitchApiClient::testRateLimiting
  #   quarantined_date: 2024-01-15
  #   reason: "Timing-sensitive, depends on network conditions"
  #   owner: "@developer"
  #   issue: "#123"
```

---

## 7. CI/CD Integration

### 7.1 Pre-Commit Hook

```bash
#!/bin/bash
# .git/hooks/pre-commit

set -e

echo "=== Pre-Commit Quality Checks ==="

# 1. Run affected unit tests only
CHANGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp)$' || true)

if [ -z "$CHANGED_FILES" ]; then
    echo "No C++ files changed, skipping tests."
    exit 0
fi

# Determine affected test targets
AFFECTED_TESTS=""
for file in $CHANGED_FILES; do
    if echo "$file" | grep -q "src/core/"; then
        AFFECTED_TESTS="$AFFECTED_TESTS test_config test_error_handler test_logger"
    fi
    if echo "$file" | grep -q "src/api/twitch/"; then
        AFFECTED_TESTS="$AFFECTED_TESTS test_twitch_api_client test_twitch_auth_manager"
    fi
    if echo "$file" | grep -q "src/media/"; then
        AFFECTED_TESTS="$AFFECTED_TESTS test_hls_ad_filter"
    fi
done

AFFECTED_TESTS=$(echo "$AFFECTED_TESTS" | tr ' ' '\n' | sort -u | tr '\n' ' ')

if [ -n "$AFFECTED_TESTS" ]; then
    echo "Running affected tests: $AFFECTED_TESTS"
    cd build
    for test in $AFFECTED_TESTS; do
        if [ -f "tests/$test" ]; then
            ./tests/$test --silent || exit 1
        fi
    done
fi

# 2. Quick format check
echo "Checking code format..."
if ! scripts/format.sh --check; then
    echo "Code formatting issues detected. Run: scripts/format.sh"
    exit 1
fi

echo "Pre-commit checks passed."
```

### 7.2 Pull Request Quality Gates

```yaml
# .github/workflows/pr-quality-gates.yml
name: PR Quality Gates

on:
  pull_request:
    branches: [main, develop]

jobs:
  quality-gates:
    runs-on: macos-latest
    timeout-minutes: 30
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup Build Environment
        run: |
          brew install qt@6 ninja llvm@19 curl
          
      - name: Configure
        run: |
          mkdir -p build && cd build
          cmake .. -G Ninja \
            -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) \
            -DBLUEPLAYER_ENABLE_COVERAGE=ON \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
            
      - name: Build
        run: cmake --build build
        
      - name: Run Tests
        id: tests
        run: |
          cd build
          ctest --output-on-failure --output-junit test-results.xml
          
      - name: Generate Coverage
        run: scripts/generate_coverage.sh
        
      - name: Check Coverage Thresholds
        id: coverage
        run: |
          COVERAGE=$(grep "TOTAL" coverage/coverage_report.txt | awk '{print $NF}' | sed 's/%//')
          echo "coverage=$COVERAGE" >> $GITHUB_OUTPUT
          
          if (( $(echo "$COVERAGE < 70" | bc -l) )); then
            echo "::error::Coverage ($COVERAGE%) is below threshold (70%)"
            exit 1
          fi
          
      - name: Check Test Execution Time
        run: |
          TIME=$(grep "Total Test time" build/Testing/Temporary/LastTest.log | \
            awk '{print $NF}' | sed 's/s//')
          
          if (( $(echo "$TIME > 60" | bc -l) )); then
            echo "::warning::Test execution time (${TIME}s) exceeds target (60s)"
          fi
          
      - name: Upload Coverage Report
        uses: actions/upload-artifact@v4
        with:
          name: coverage-report
          path: coverage/
          
      - name: Comment PR with Coverage
        uses: actions/github-script@v7
        with:
          script: |
            const coverage = '${{ steps.coverage.outputs.coverage }}';
            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: `## Test Quality Report\n\n` +
                    `| Metric | Value | Status |\n` +
                    `|--------|-------|--------|\n` +
                    `| Coverage | ${coverage}% | ${coverage >= 70 ? 'PASS' : 'FAIL'} |\n`
            })

  mutation-testing:
    runs-on: macos-latest
    timeout-minutes: 60
    if: github.event.pull_request.draft == false
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Setup Mull
        run: |
          # Install Mull for mutation testing
          brew install llvm@19
          # Download mull-runner from releases
          
      - name: Run Mutation Tests
        continue-on-error: true
        run: scripts/run_mutation_tests.sh
        
      - name: Upload Mutation Report
        uses: actions/upload-artifact@v4
        with:
          name: mutation-report
          path: mutation-reports/
```

### 7.3 Nightly Comprehensive Run

```yaml
# .github/workflows/nightly-quality.yml
name: Nightly Quality Analysis

on:
  schedule:
    - cron: '0 2 * * *'  # 2 AM UTC daily
  workflow_dispatch:

jobs:
  comprehensive-analysis:
    runs-on: macos-latest
    timeout-minutes: 120
    
    steps:
      - uses: actions/checkout@v4
      
      - name: Full Build and Test
        run: scripts/validate_build.sh
        
      - name: Full Mutation Testing
        run: scripts/run_mutation_tests.sh
        
      - name: Flaky Test Detection
        run: scripts/detect_flaky_tests.sh 10
        
      - name: Collect All Metrics
        run: scripts/collect_test_metrics.sh
        
      - name: Generate Trend Report
        run: scripts/coverage_trend.sh 30
        
      - name: Check Quality Trends
        run: |
          # Compare with previous run
          CURRENT=$(jq '.coverage.line' metrics/dashboard_*.json | tail -1)
          PREVIOUS=$(jq '.coverage.line' metrics/history/dashboard_*.json | tail -2 | head -1)
          
          DIFF=$(echo "$CURRENT - $PREVIOUS" | bc)
          
          if (( $(echo "$DIFF < -5" | bc -l) )); then
            echo "::error::Coverage dropped by more than 5% ($DIFF%)"
            exit 1
          fi
          
      - name: Store Metrics History
        run: |
          git config user.name "GitHub Actions"
          git config user.email "actions@github.com"
          git add metrics/history/
          git commit -m "chore: update test metrics [skip ci]" || true
          git push || true
          
      - name: Notify on Failure
        if: failure()
        uses: actions/github-script@v7
        with:
          script: |
            github.rest.issues.create({
              owner: context.repo.owner,
              repo: context.repo.repo,
              title: 'Nightly Quality Check Failed',
              body: 'The nightly quality analysis has failed. Please investigate.',
              labels: ['quality', 'automated']
            })
```

### 7.4 Coverage Trend Reporting

```yaml
# .github/workflows/coverage-trend.yml
name: Coverage Trend Report

on:
  push:
    branches: [main]

jobs:
  update-trend:
    runs-on: macos-latest
    
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0
          
      - name: Build and Test
        run: scripts/validate_build.sh
        
      - name: Collect Metrics
        run: scripts/collect_test_metrics.sh
        
      - name: Generate Badge
        run: |
          COVERAGE=$(jq '.coverage.line' metrics/dashboard_*.json | tail -1)
          
          if (( $(echo "$COVERAGE >= 80" | bc -l) )); then
            COLOR="brightgreen"
          elif (( $(echo "$COVERAGE >= 70" | bc -l) )); then
            COLOR="green"
          elif (( $(echo "$COVERAGE >= 60" | bc -l) )); then
            COLOR="yellow"
          else
            COLOR="red"
          fi
          
          echo "{\"schemaVersion\":1,\"label\":\"coverage\",\"message\":\"${COVERAGE}%\",\"color\":\"$COLOR\"}" \
            > metrics/coverage-badge.json
            
      - name: Update Gist Badge
        uses: exuanbo/actions-deploy-gist@v1
        with:
          token: ${{ secrets.GIST_TOKEN }}
          gist_id: YOUR_GIST_ID
          file_path: metrics/coverage-badge.json
```

---

## 8. Test Quality Improvement Roadmap

### Phase 1: Foundation (Current Sprint)

| Task | Priority | Status |
|------|----------|--------|
| Implement metrics collection script | High | TODO |
| Set up module-level thresholds | High | TODO |
| Configure pre-commit hooks | Medium | TODO |
| Document critical path coverage | High | DONE |

### Phase 2: Automation (Next Sprint)

| Task | Priority | Status |
|------|----------|--------|
| PR quality gate workflow | High | TODO |
| Nightly comprehensive runs | Medium | TODO |
| Flaky test detection | Medium | TODO |
| Coverage trend reporting | Low | TODO |

### Phase 3: Optimization (Future)

| Task | Priority | Status |
|------|----------|--------|
| Test parallelization | Medium | TODO |
| Incremental testing | High | TODO |
| Test impact analysis | Medium | TODO |
| Mutation score by PR | Low | TODO |

---

## 9. Current State Analysis

### 9.1 Existing Infrastructure Summary

| Component | Current State | Recommended Improvement |
|-----------|--------------|------------------------|
| Coverage Tool | llvm-cov | Add module-level reporting |
| Coverage Threshold | 70% global | Add per-module thresholds |
| Mutation Testing | Mull configured | Add score requirements |
| Build Validation | validate_build.sh | Add timing constraints |
| CI/CD | None visible | Full GitHub Actions setup |

### 9.2 Test Matrix

| Module | Source Files | Test Files | Ratio | Status |
|--------|--------------|------------|-------|--------|
| core/ | 13 | 11 | 0.85 | NEEDS WORK |
| core/network/ | 3 | 3 | 1.00 | OK |
| api/twitch/ | 3 | 3 | 1.00 | OK |
| media/ | 3 | 1 | 0.33 | CRITICAL |
| chat/ | 2 | 1 | 0.50 | NEEDS WORK |
| ui/ | 2 | 2 | 1.00 | OK |

### 9.3 Missing Test Coverage (Identified Gaps)

```
Files without dedicated tests:
- src/core/Application.cpp (integration tested only)
- src/core/Constants.hpp (header-only, may not need)
- src/core/VodMetadata.hpp (header-only, may not need)
- src/media/MpvQuickItem.cpp (hardware dependent)
- src/media/MpvFboItem.cpp (hardware dependent)
- src/chat/ChatMessage.hpp (header-only)
```

---

## 10. Appendix: Reference Configurations

### A. Enhanced mull.yml

```yaml
# mull.yml - Enhanced mutation testing configuration
mutators:
  - and_or_replacement
  - conditionals_boundary
  - math_add
  - math_div
  - math_mul
  - math_sub
  - negate_condition
  - remove_void_function
  - replace_assignment
  - replace_call

excludePaths:
  - "**/moc_*.cpp"
  - "**/qrc_*.cpp"
  - "**/uic_*.cpp"
  - "**/*_autogen/**"
  - "**/tests/**"
  - "**/main.mm"

includePaths:
  - "src/core/**"
  - "src/media/**"
  - "src/api/**"
  - "src/ui/**"
  - "src/chat/**"

timeout: 30000
threads: 4
workers: 4

# Quality gates for mutation testing
qualityGates:
  minMutationScore: 60
  targetMutationScore: 75
  failOnScoreBelowMin: true
```

### B. Quality Gate Configuration File

```yaml
# .quality-gates.yml
version: 1.0

coverage:
  global:
    line: 70
    branch: 65
    function: 80
    
  modules:
    core:
      line: 85
      branch: 80
      critical: true
    api:
      line: 80
      branch: 70
      critical: true
    media:
      line: 60
      branch: 55
      critical: false

mutation:
  global:
    min_score: 60
    target_score: 75
    
execution:
  max_time: 60
  max_single_test: 5
  
flaky:
  tolerance: 0
  quarantine_after: 2
  
ratio:
  min_test_to_code: 0.8
  target: 1.2
```

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2024-12-27 | BluePlayer Team | Initial specification |
