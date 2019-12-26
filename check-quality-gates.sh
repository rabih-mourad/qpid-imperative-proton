#!/bin/bash -e

BUILD_DIR=build-dir
TEST_DIR=test

cd $BUILD_DIR/$TEST_DIR

# Checking 
./qpid-imperative-proton-test --gtest_filter=* --gtest_repeat=500000 --gtest_break_on_failure