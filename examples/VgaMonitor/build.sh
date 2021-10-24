#!/bin/sh

cmake -G "Eclipse CDT4 - Unix Makefiles" \
    -DCMAKE_ECLIPSE_VERSION=4.21.0 \
    -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE \
    -DCMAKE_BUILD_TYPE=Debug \
    -S ./ \
    -B ../../../build/examples/VgaMonitor
