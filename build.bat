@echo off
rem Configure (first run downloads pinned dependencies) and build everything in Release.
rem Requires: CMake 3.24+ and Visual Studio 2022 (or Build Tools) with "Desktop development with C++".
setlocal
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 || goto :error
cmake --build build --config Release --parallel || goto :error
ctest --test-dir build -C Release --output-on-failure || goto :error
echo.
echo Build OK.  Run:  run.bat          (main application)
echo                  run_glut_demo.bat (freeGLUT demonstration)
exit /b 0
:error
echo.
echo BUILD FAILED - see the messages above.
exit /b 1
