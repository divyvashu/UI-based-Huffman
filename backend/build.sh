#!/bin/bash
set -e

echo "Building Huffman Linux binary..."

cd c
gcc huffman.c -o huffman -lm
chmod +x huffman

echo "Huffman build complete."
