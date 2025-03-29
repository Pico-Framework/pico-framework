#!/bin/bash

# Set up paths
TEST_DIR="tests"
BUILD_DIR="build-tests"
TOOLCHAIN_FILE="$TEST_DIR/host-toolchain.cmake"
TEST_EXECUTABLE="HttpRequestTest"

# Create build dir if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

# Run CMake with host toolchain
echo "Running CMake with host toolchain..."
cmake -DCMAKE_TOOLCHAIN_FILE="../$TOOLCHAIN_FILE" "../$TEST_DIR" || exit 1

# Build the test executable
echo "Building tests..."
make "$TEST_EXECUTABLE" || exit 1

# Run the test binary
echo "Running $TEST_EXECUTABLE..."
./"$TEST_EXECUTABLE"
