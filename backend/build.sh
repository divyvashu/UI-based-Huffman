#!/bin/bash

echo "Building Huffman C binary..."

cd c
gcc huffman.c -o huffman.exe
cd ..

echo "Installing Node dependencies..."

cd server
npm install

echo "Build completed successfully."
