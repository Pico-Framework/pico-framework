#!/bin/bash

# Usage: ./run_http_tests.sh [TestExecutableTargetName]
# If no argument is given, it defaults to "HttpRequestTest"

TEST_DIR="tests"
BUILD_DIR="build-tests"
TOOLCHAIN_FILE="$TEST_DIR/host-toolchain.cmake"
TEST_EXECUTABLE="${1:-HttpRequestTest}"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

echo "Running CMake with host toolchain..."
cmake -DCMAKE_TOOLCHAIN_FILE="../$TOOLCHAIN_FILE" "../$TEST_DIR" || exit 1

echo "Building tests for target: $TEST_EXECUTABLE..."
make "$TEST_EXECUTABLE" || exit 1

echo "Running $TEST_EXECUTABLE..."
./"$TEST_EXECUTABLE"
