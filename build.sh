#!/bin/bash -e

BUILD_DIR=build-dir
TEST_DIR=test

if [ -d "$BUILD_DIR" ]; then
        rm -rf $BUILD_DIR
fi
mkdir $BUILD_DIR
cd $BUILD_DIR

#build
cmake .. -DCMAKE_CXX_FLAGS="-std=c++11 -g"
make

#run unit tests
./$TEST_DIR/qpid-imperative-proton-test
