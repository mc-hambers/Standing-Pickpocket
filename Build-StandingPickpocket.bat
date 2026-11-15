@echo off
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File Build-All.ps1 > build-log-v1.3.1.txt 2>&1
if errorlevel 1 (echo Build failed. See build-log-v1.3.1.txt.) else (echo Build complete. See output.)
pause

