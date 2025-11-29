#!/bin/bash
set -e

echo "Building Huffman Linux binary..."

cd c
gcc huffman.c -o huffman       # Linux binary – NOT .exe
chmod +x huffman               # Make it executable

echo "Build complete."
