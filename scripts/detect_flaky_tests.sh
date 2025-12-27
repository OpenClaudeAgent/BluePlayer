#!/bin/bash

# Script to detect flaky tests by running the test suite multiple times
# and identifying tests with inconsistent results

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
METRICS_DIR="$PROJECT_ROOT/metrics"
RUNS=${1:-5}

mkdir -p "$METRICS_DIR"

echo "=== Flaky Test Detection ==="
echo "Running test suite ${RUNS} times to detect flaky tests..."
echo ""

# Ensure build exists
if [ ! -d "$BUILD_DIR/tests" ]; then
    echo "Error: Test binaries not found. Build the project first."
    exit 1
fi

# Create temporary directory for results
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

# Run tests multiple times
for run in $(seq 1 $RUNS); do
    echo "Run $run/$RUNS..."
    
    cd "$BUILD_DIR"
    ctest --output-on-failure --timeout 30 2>&1 | \
        tee "$TEMP_DIR/run_$run.log" > /dev/null || true
done

echo ""
echo "Analyzing results..."
echo ""

# Extract test results from each run
declare -A TEST_PASS_COUNT
declare -A TEST_FAIL_COUNT
declare -A TEST_NAMES

for run in $(seq 1 $RUNS); do
    LOG="$TEMP_DIR/run_$run.log"
    
    # Parse test results - format: "Test #N: TestName ... Passed/Failed"
    while IFS= read -r line; do
        if echo "$line" | grep -qE "Test #[0-9]+:"; then
            # Extract test name
            TEST_NAME=$(echo "$line" | sed -E 's/.*Test #[0-9]+: ([^ ]+).*/\1/')
            TEST_NAMES["$TEST_NAME"]=1
            
            if echo "$line" | grep -q "Passed"; then
                current=${TEST_PASS_COUNT["$TEST_NAME"]:-0}
                TEST_PASS_COUNT["$TEST_NAME"]=$((current + 1))
            elif echo "$line" | grep -q "Failed"; then
                current=${TEST_FAIL_COUNT["$TEST_NAME"]:-0}
                TEST_FAIL_COUNT["$TEST_NAME"]=$((current + 1))
            fi
        fi
    done < "$LOG"
done

# Generate report
FLAKY_LOG="$METRICS_DIR/flaky_tests_$(date +%Y%m%d_%H%M%S).csv"
echo "Test Name,Pass Count,Fail Count,Total Runs,Status" > "$FLAKY_LOG"

FLAKY_COUNT=0
STABLE_COUNT=0

echo "=== Test Stability Report ==="
echo ""
printf "%-40s %6s %6s %10s\n" "Test Name" "Pass" "Fail" "Status"
echo "----------------------------------------------------------------------"

for test in "${!TEST_NAMES[@]}"; do
    PASSES=${TEST_PASS_COUNT["$test"]:-0}
    FAILS=${TEST_FAIL_COUNT["$test"]:-0}
    TOTAL=$((PASSES + FAILS))
    
    if [ "$PASSES" -gt 0 ] && [ "$FAILS" -gt 0 ]; then
        STATUS="FLAKY"
        ((FLAKY_COUNT++))
        printf "%-40s %6d %6d %10s\n" "$test" "$PASSES" "$FAILS" "$STATUS"
    elif [ "$FAILS" -eq "$TOTAL" ]; then
        STATUS="ALWAYS_FAILS"
        printf "%-40s %6d %6d %10s\n" "$test" "$PASSES" "$FAILS" "$STATUS"
    else
        STATUS="STABLE"
        ((STABLE_COUNT++))
    fi
    
    echo "$test,$PASSES,$FAILS,$TOTAL,$STATUS" >> "$FLAKY_LOG"
done

echo ""
echo "----------------------------------------------------------------------"
echo ""
echo "Summary:"
echo "  Total tests analyzed: ${#TEST_NAMES[@]}"
echo "  Stable tests: $STABLE_COUNT"
echo "  Flaky tests: $FLAKY_COUNT"
echo ""
echo "Report saved: $FLAKY_LOG"
echo ""

# Generate JSON summary
cat > "$METRICS_DIR/flaky_summary.json" << EOF
{
  "timestamp": "$(date -Iseconds)",
  "runs": $RUNS,
  "total_tests": ${#TEST_NAMES[@]},
  "stable_tests": $STABLE_COUNT,
  "flaky_tests": $FLAKY_COUNT,
  "flaky_tolerance": 0,
  "status": "$([ $FLAKY_COUNT -eq 0 ] && echo 'PASS' || echo 'FAIL')"
}
EOF

if [ $FLAKY_COUNT -eq 0 ]; then
    echo "No flaky tests detected. Test suite is stable."
    exit 0
else
    echo "WARNING: Detected $FLAKY_COUNT flaky test(s)!"
    echo ""
    echo "Flaky tests should be investigated and fixed."
    echo "Common causes:"
    echo "  - Timing-dependent assertions"
    echo "  - Shared state between tests"
    echo "  - External dependencies (network, filesystem)"
    echo "  - Race conditions"
    exit 1
fi
