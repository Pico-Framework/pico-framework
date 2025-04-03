#!/bin/bash

set -e  # Exit on error

mkdir -p cpputest/build-arm
cd cpputest/build-arm

cmake .. #\
#   -DCMAKE_TOOLCHAIN_FILE=../../build_cpputest_pico.cmake \
#   -DCMAKE_INSTALL_PREFIX=../cpputest-install-arm \
#   -DCMAKE_BUILD_TYPE=Release \
#   -DBUILD_TESTING=OFF \
#   -DTESTS=OFF \
#   -DEXAMPLES=OFF \
#   -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
#   -DCPPUTEST_MEMORY_LEAK_DETECTION=OFF \
#     -DCPPUTEST_USE_CPP11=ON \
#   -DCPPUTEST_USE_LONG_LONG=ON

make -j 
