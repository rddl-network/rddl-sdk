#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Run CMake to generate build files if necessary
cmake -S . -B build

# Compile modified files using --parallel flag for faster build
cmake --build build --parallel $(nproc)
