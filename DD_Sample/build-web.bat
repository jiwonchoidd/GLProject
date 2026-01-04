@echo off
REM Emscripten Web Build Script for Windows

echo Building DD_Engine for Web...

REM Create build directory
if not exist web-build mkdir web-build
cd web-build

REM Configure with Emscripten
call emcmake cmake ..

REM Build
call emmake make

echo.
echo Build complete!
echo Output files are in web-build/
echo.
echo To test locally, run:
echo   cd web-build
echo   python -m http.server 8000
echo Then open http://localhost:8000/DD_Sample.html
