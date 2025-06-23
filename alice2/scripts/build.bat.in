@echo off
echo Building alice2...

:: Create build directory
if not exist "build" mkdir build
cd build

:: Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

:: Build
cmake --build . --config Release

:: Copy executable to root directory for convenience
copy Release\alice2.exe ..\alice2.exe

echo Build complete!
pause
