@echo off
setlocal
if not exist "%~dp0bin" mkdir "%~dp0bin"
gcc -O2 -Wall -o %~dp0bin\afd.exe %~dp0\afd.c
if errorlevel 1 exit /b 1
echo [MinGW] Build OK: c\bin\afd.exe
