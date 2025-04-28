#! /usr/bin/bash

if [ ! -d "out" ]; then
    mkdir out
fi

cd out
cmake ..
make

if [ "$?" -eq 0 ]; then
    ./vulkan
fi
