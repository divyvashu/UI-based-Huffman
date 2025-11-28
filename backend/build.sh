#!/bin/bash

echo "Compiling Huffman C binary..."

cd c
gcc huffman.c -o huffman.exe
cd ..

echo "Installing Node dependencies..."
npm install

echo "Build complete."
