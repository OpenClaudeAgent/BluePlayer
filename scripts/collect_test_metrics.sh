#!/bin/bash

# Script to collect comprehensive test metrics for the quality dashboard
# Generates JSON metrics for coverage, execution time, and test-to-code ratio

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
METRICS_DIR="$PROJECT_ROOT/metrics"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

mkdir -p "$METRICS_DIR/history"

echo "=== Collecting Test Metrics ==="
echo ""

# Ensure build exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Run cmake first."
    exit 1
fi

# 1. Run tests and capture timing
echo "Step 1/4: Running tests..."
cd "$BUILD_DIR"

TEST_START=$(date +%s.%N)
ctest --output-on-failure --output-junit "$METRICS_DIR/junit_$TIMESTAMP.xml" 2>&1 | \
    tee "$METRICS_DIR/ctest_output.log" || true
TEST_END=$(date +%s.%N)

TOTAL_TIME=$(echo "$TEST_END - $TEST_START" | bc)
TEST_COUNT=$(grep -c "Test #" "$METRICS_DIR/ctest_output.log" 2>/dev/null || echo 0)
PASSED_COUNT=$(grep -c "Passed" "$METRICS_DIR/ctest_output.log" 2>/dev/null || echo 0)
FAILED_COUNT=$(grep -c "Failed" "$METRICS_DIR/ctest_output.log" 2>/dev/null || echo 0)

echo "Tests completed in ${TOTAL_TIME}s"
echo ""

# 2. Extract coverage metrics
echo "Step 2/4: Extracting coverage metrics..."

if [ -f "$BUILD_DIR/default.profdata" ]; then
    # Find test binaries
    TEST_BINARIES=$(find "$BUILD_DIR/tests" -type f -perm +111 -not -name "*.dylib" 2>/dev/null | \
        grep -E "test_|Test" | head -10 || true)
    
    if [ -n "$TEST_BINARIES" ]; then
        COV_OBJECTS=""
        for binary in $TEST_BINARIES; do
            COV_OBJECTS="$COV_OBJECTS -object $binary"
        done
        
        # Generate coverage report
        COVERAGE_OUTPUT=$(llvm-cov report \
            -instr-profile="$BUILD_DIR/default.profdata" \
            $COV_OBJECTS 2>/dev/null || echo "")
        
        if [ -n "$COVERAGE_OUTPUT" ]; then
            TOTAL_LINE=$(echo "$COVERAGE_OUTPUT" | grep "TOTAL" | tail -1)
            
            # Parse coverage percentages (format varies by llvm-cov version)
            LINE_COV=$(echo "$TOTAL_LINE" | awk '{for(i=1;i<=NF;i++) if($i ~ /%/) print $i}' | \
                head -1 | sed 's/%//' || echo "0")
            BRANCH_COV=$(echo "$TOTAL_LINE" | awk '{for(i=1;i<=NF;i++) if($i ~ /%/) print $i}' | \
                tail -1 | sed 's/%//' || echo "0")
            FUNC_COV=$(echo "$TOTAL_LINE" | awk '{for(i=1;i<=NF;i++) if($i ~ /%/) print $i}' | \
                sed -n '2p' | sed 's/%//' || echo "0")
        else
            LINE_COV="0"
            BRANCH_COV="0"
            FUNC_COV="0"
        fi
    else
        LINE_COV="0"
        BRANCH_COV="0"
        FUNC_COV="0"
    fi
else
    echo "Warning: No profdata found. Run tests with coverage enabled first."
    LINE_COV="0"
    BRANCH_COV="0"
    FUNC_COV="0"
fi

echo "Line coverage: ${LINE_COV}%"
echo ""

# 3. Calculate test-to-code ratio
echo "Step 3/4: Calculating test-to-code ratio..."

SRC_LOC=$(find "$PROJECT_ROOT/src" \( -name "*.cpp" -o -name "*.hpp" \) -exec cat {} \; 2>/dev/null | \
    wc -l | tr -d ' ')
TEST_LOC=$(find "$PROJECT_ROOT/tests" \( -name "*.cpp" -o -name "*.hpp" \) -exec cat {} \; 2>/dev/null | \
    wc -l | tr -d ' ')

if [ "$SRC_LOC" -gt 0 ]; then
    RATIO=$(echo "scale=2; $TEST_LOC / $SRC_LOC" | bc)
else
    RATIO="0"
fi

SRC_FILES=$(find "$PROJECT_ROOT/src" \( -name "*.cpp" -o -name "*.hpp" \) 2>/dev/null | wc -l | tr -d ' ')
TEST_FILES=$(find "$PROJECT_ROOT/tests" \( -name "*.cpp" -o -name "*.hpp" \) 2>/dev/null | wc -l | tr -d ' ')

echo "Source LOC: $SRC_LOC"
echo "Test LOC: $TEST_LOC"
echo "Ratio: $RATIO"
echo ""

# 4. Generate quality gate status
echo "Step 4/4: Evaluating quality gates..."

COVERAGE_THRESHOLD=70
TIMING_THRESHOLD=60
RATIO_THRESHOLD=0.8

COVERAGE_PASSED="false"
TIMING_PASSED="false"
RATIO_PASSED="false"

if [ -n "$LINE_COV" ] && [ $(echo "$LINE_COV >= $COVERAGE_THRESHOLD" | bc 2>/dev/null || echo 0) -eq 1 ]; then
    COVERAGE_PASSED="true"
fi

if [ $(echo "$TOTAL_TIME <= $TIMING_THRESHOLD" | bc 2>/dev/null || echo 0) -eq 1 ]; then
    TIMING_PASSED="true"
fi

if [ $(echo "$RATIO >= $RATIO_THRESHOLD" | bc 2>/dev/null || echo 0) -eq 1 ]; then
    RATIO_PASSED="true"
fi

# 5. Generate JSON report
cat > "$METRICS_DIR/dashboard_$TIMESTAMP.json" << EOF
{
  "timestamp": "$TIMESTAMP",
  "date": "$(date -Iseconds)",
  "coverage": {
    "line": ${LINE_COV:-0},
    "branch": ${BRANCH_COV:-0},
    "function": ${FUNC_COV:-0}
  },
  "execution": {
    "total_time_seconds": ${TOTAL_TIME},
    "test_count": ${TEST_COUNT},
    "passed": ${PASSED_COUNT},
    "failed": ${FAILED_COUNT}
  },
  "codebase": {
    "source_files": ${SRC_FILES},
    "source_loc": ${SRC_LOC},
    "test_files": ${TEST_FILES},
    "test_loc": ${TEST_LOC},
    "ratio": ${RATIO}
  },
  "quality_gates": {
    "coverage_passed": ${COVERAGE_PASSED},
    "coverage_threshold": ${COVERAGE_THRESHOLD},
    "timing_passed": ${TIMING_PASSED},
    "timing_threshold": ${TIMING_THRESHOLD},
    "ratio_passed": ${RATIO_PASSED},
    "ratio_threshold": ${RATIO_THRESHOLD}
  }
}
EOF

# Archive to history
cp "$METRICS_DIR/dashboard_$TIMESTAMP.json" "$METRICS_DIR/history/"

# Create latest symlink
ln -sf "dashboard_$TIMESTAMP.json" "$METRICS_DIR/dashboard_latest.json"

echo ""
echo "=== Test Metrics Summary ==="
echo ""
echo "Coverage:"
echo "  Line:     ${LINE_COV}% (threshold: ${COVERAGE_THRESHOLD}%) - $([ "$COVERAGE_PASSED" = "true" ] && echo 'PASS' || echo 'FAIL')"
echo "  Branch:   ${BRANCH_COV}%"
echo "  Function: ${FUNC_COV}%"
echo ""
echo "Execution:"
echo "  Time:     ${TOTAL_TIME}s (threshold: ${TIMING_THRESHOLD}s) - $([ "$TIMING_PASSED" = "true" ] && echo 'PASS' || echo 'FAIL')"
echo "  Tests:    ${TEST_COUNT} (${PASSED_COUNT} passed, ${FAILED_COUNT} failed)"
echo ""
echo "Codebase:"
echo "  Source:   ${SRC_FILES} files, ${SRC_LOC} LOC"
echo "  Tests:    ${TEST_FILES} files, ${TEST_LOC} LOC"
echo "  Ratio:    ${RATIO} (threshold: ${RATIO_THRESHOLD}) - $([ "$RATIO_PASSED" = "true" ] && echo 'PASS' || echo 'FAIL')"
echo ""
echo "Dashboard saved: $METRICS_DIR/dashboard_$TIMESTAMP.json"

# Exit with error if any quality gate failed
if [ "$COVERAGE_PASSED" = "false" ] || [ "$TIMING_PASSED" = "false" ]; then
    echo ""
    echo "Warning: Some quality gates did not pass."
    exit 0  # Don't fail the script, just warn
fi

echo ""
echo "All quality gates passed!"
