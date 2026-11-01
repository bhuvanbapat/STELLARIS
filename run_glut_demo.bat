@echo off
rem Launch the freeGLUT/OpenGL demonstration (build first with build.bat).
cd /d "%~dp0"
if not exist "build\bin\Release\stellaris_glut_demo.exe" (
    echo stellaris_glut_demo.exe not found - run build.bat first.
    exit /b 1
)
start "" "build\bin\Release\stellaris_glut_demo.exe"
