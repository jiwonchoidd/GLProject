#!/bin/bash

# Emscripten Web Build Script

echo "Building DD_Engine for Web..."

# Create build directory
mkdir -p web-build
cd web-build

# Configure with Emscripten
emcmake cmake ..

# Build
emmake make

echo ""
echo "Build complete!"
echo "Output files are in web-build/"
echo ""
echo "To test locally, run:"
echo "  cd web-build"
echo "  python -m http.server 8000"
echo "Then open http://localhost:8000/DD_Sample.html"
