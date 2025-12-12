#!/bin/bash

# Script pour exécuter l'analyse statique du code

set -e

REPO_ROOT=$(git rev-parse --show-toplevel)
if [ -z "$REPO_ROOT" ]; then
  echo "Error: Not in a Git repository."
  exit 1
fi

cd "$REPO_ROOT"

echo "Running static analysis on BluePlayer..."

# Clang-tidy
if command -v clang-tidy &> /dev/null; then
  echo "Running clang-tidy..."
  find src -name "*.cpp" -o -name "*.hpp" | while read -r file; do
    echo "Analyzing $file"
    clang-tidy "$file" -- -I src -std=c++20 $(pkg-config --cflags Qt6Core Qt6Quick Qt6Gui Qt6Multimedia) 2>&1 | head -20
  done
else
  echo "Warning: clang-tidy not found. Install it to enable static analysis."
fi

# cppcheck
if command -v cppcheck &> /dev/null; then
  echo "Running cppcheck..."
  cppcheck --enable=all --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    --std=c++20 \
    -I src \
    src/ 2>&1 | head -50
else
  echo "Warning: cppcheck not found. Install it to enable static analysis."
fi

echo "Static analysis complete."












