#!/bin/bash

if [[ $# -eq 0 ]] ; then
    ARGS="-DBUILD_TESTS=OFF -DBUILD_COVER=OFF -DCMAKE_BUILD_TYPE=Release"
else
    ARGS="$@"
fi

mkdir -p build
cd build
cmake ../ $ARGS
make && make install
