#!/bin/bash

# Script to compile and run the MM checker

echo "=== EFX Allocation MM Checker ==="
echo ""

# Check if we're in the right directory
if [ ! -f "check_mm.cpp" ] || [ ! -f "allocation.cpp" ] || [ ! -f "allocation.hpp" ]; then
    echo "Error: Required source files not found in current directory."
    echo "Please run this script from the cpp_core directory."
    exit 1
fi

# Clean previous builds
echo "Cleaning previous builds..."
make clean

# Compile the program
echo "Compiling..."
make

if [ $? -ne 0 ]; then
    echo "Error: Compilation failed!"
    exit 1
fi

echo "Compilation successful!"
echo ""

# Run the program
echo "Running MM checker..."
echo ""

# Check if mm_checker executable exists
if [ ! -f "mm_checker" ]; then
    echo "Error: Executable not found after compilation!"
    exit 1
fi

# Run the program
./mm_checker

echo ""
echo "Program completed. Check mm.txt for detailed output." 