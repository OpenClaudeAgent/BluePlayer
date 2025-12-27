#!/bin/bash

# Script to generate per-module coverage reports with threshold validation
# Provides detailed breakdown of coverage by source module

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
COVERAGE_DIR="$PROJECT_ROOT/coverage"

# Module definitions with thresholds
declare -A MODULE_THRESHOLDS
MODULE_THRESHOLDS["core"]=85
MODULE_THRESHOLDS["core/network"]=80
MODULE_THRESHOLDS["api/twitch"]=80
MODULE_THRESHOLDS["media"]=60
MODULE_THRESHOLDS["chat"]=75
MODULE_THRESHOLDS["ui"]=70

echo "=== Module Coverage Report ==="
echo ""

# Check prerequisites
if [ ! -f "$BUILD_DIR/default.profdata" ]; then
    echo "Error: No profdata found. Run tests with coverage enabled first."
    echo "Execute: scripts/generate_coverage.sh"
    exit 1
fi

# Find test binaries
TEST_BINARIES=$(find "$BUILD_DIR/tests" -type f -perm +111 -not -name "*.dylib" 2>/dev/null | \
    grep -E "test_|Test" || true)

if [ -z "$TEST_BINARIES" ]; then
    echo "Error: No test binaries found."
    exit 1
fi

# Build object list for llvm-cov
COV_OBJECTS=""
for binary in $TEST_BINARIES; do
    COV_OBJECTS="$COV_OBJECTS -object $binary"
done

# Header
printf "%-25s %10s %10s %10s %10s %10s\n" \
    "Module" "Lines" "Line%" "Branches" "Branch%" "Status"
echo "--------------------------------------------------------------------------------"

FAILED_MODULES=0
TOTAL_MODULES=0

for module in "${!MODULE_THRESHOLDS[@]}"; do
    THRESHOLD=${MODULE_THRESHOLDS[$module]}
    MODULE_PATH="$PROJECT_ROOT/src/$module"
    
    if [ ! -d "$MODULE_PATH" ]; then
        continue
    fi
    
    ((TOTAL_MODULES++))
    
    # Get source files in module
    SOURCE_FILES=$(find "$MODULE_PATH" -maxdepth 1 \( -name "*.cpp" -o -name "*.hpp" \) 2>/dev/null || true)
    
    if [ -z "$SOURCE_FILES" ]; then
        printf "%-25s %10s %10s %10s %10s %10s\n" \
            "$module" "N/A" "N/A" "N/A" "N/A" "NO_FILES"
        continue
    fi
    
    # Generate module-specific coverage
    COVERAGE_OUTPUT=$(llvm-cov report \
        -instr-profile="$BUILD_DIR/default.profdata" \
        $COV_OBJECTS \
        $SOURCE_FILES 2>/dev/null || echo "")
    
    if [ -z "$COVERAGE_OUTPUT" ]; then
        printf "%-25s %10s %10s %10s %10s %10s\n" \
            "$module" "N/A" "N/A" "N/A" "N/A" "NO_DATA"
        continue
    fi
    
    # Parse TOTAL line
    TOTAL_LINE=$(echo "$COVERAGE_OUTPUT" | grep "TOTAL" | tail -1 || echo "")
    
    if [ -z "$TOTAL_LINE" ]; then
        printf "%-25s %10s %10s %10s %10s %10s\n" \
            "$module" "N/A" "N/A" "N/A" "N/A" "NO_TOTAL"
        continue
    fi
    
    # Extract metrics (format: TOTAL regions missed cover lines missed cover branches missed cover)
    LINES=$(echo "$TOTAL_LINE" | awk '{print $4}')
    LINE_PCT=$(echo "$TOTAL_LINE" | awk '{print $6}' | sed 's/%//')
    BRANCHES=$(echo "$TOTAL_LINE" | awk '{print $7}')
    BRANCH_PCT=$(echo "$TOTAL_LINE" | awk '{print $9}' | sed 's/%//')
    
    # Determine status
    if [ -n "$LINE_PCT" ] && [ $(echo "$LINE_PCT >= $THRESHOLD" | bc 2>/dev/null || echo 0) -eq 1 ]; then
        STATUS="PASS"
    else
        STATUS="FAIL"
        ((FAILED_MODULES++))
    fi
    
    printf "%-25s %10s %10s%% %10s %10s%% %10s\n" \
        "$module" "${LINES:-N/A}" "${LINE_PCT:-N/A}" "${BRANCHES:-N/A}" "${BRANCH_PCT:-N/A}" "$STATUS"
done

echo "--------------------------------------------------------------------------------"
echo ""

# Summary
echo "Summary:"
echo "  Total modules: $TOTAL_MODULES"
echo "  Passing: $((TOTAL_MODULES - FAILED_MODULES))"
echo "  Failing: $FAILED_MODULES"
echo ""

# Generate JSON report
REPORT_FILE="$COVERAGE_DIR/module_coverage_$(date +%Y%m%d_%H%M%S).json"
mkdir -p "$COVERAGE_DIR"

cat > "$REPORT_FILE" << EOF
{
  "timestamp": "$(date -Iseconds)",
  "total_modules": $TOTAL_MODULES,
  "passing_modules": $((TOTAL_MODULES - FAILED_MODULES)),
  "failing_modules": $FAILED_MODULES,
  "thresholds": {
EOF

first=true
for module in "${!MODULE_THRESHOLDS[@]}"; do
    if [ "$first" = true ]; then
        first=false
    else
        echo "," >> "$REPORT_FILE"
    fi
    echo -n "    \"$module\": ${MODULE_THRESHOLDS[$module]}" >> "$REPORT_FILE"
done

cat >> "$REPORT_FILE" << EOF

  }
}
EOF

echo "Report saved: $REPORT_FILE"

if [ $FAILED_MODULES -gt 0 ]; then
    echo ""
    echo "WARNING: $FAILED_MODULES module(s) below coverage threshold!"
    exit 1
fi

echo ""
echo "All modules meet coverage thresholds."
