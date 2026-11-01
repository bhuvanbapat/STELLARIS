@echo off
rem Launch STELLARIS (build first with build.bat). Extra arguments are passed through,
rem e.g.  run.bat --fullscreen   or   run.bat --mass 15 --play
cd /d "%~dp0"
if not exist "build\bin\Release\stellaris.exe" (
    echo stellaris.exe not found - run build.bat first.
    exit /b 1
)
start "" "build\bin\Release\stellaris.exe" %*
