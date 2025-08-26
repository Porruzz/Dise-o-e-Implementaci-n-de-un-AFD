@echo off
setlocal ENABLEDELAYEDEXPANSION

rem --- localizar Visual Studio con vswhere (se instala con los Build Tools) ---
set VSWHERE="C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
  echo [ERROR] No encuentro vswhere.exe. Instala "Microsoft Visual Studio Build Tools" (C++), o abre el "x64 Native Tools Command Prompt for VS" y corre este mismo .bat.
  exit /b 1
)

for /f "usebackq tokens=* delims=" %%I in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VSINSTALL=%%I
if "%VSINSTALL%"=="" (
  echo [ERROR] No encuentro instalacion de Visual C++ Build Tools.
  exit /b 1
)

set VCVARS="%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if not exist %VCVARS% (
  echo [ERROR] No encuentro vcvars64.bat en: %VCVARS%
  exit /b 1
)

call %VCVARS%
if errorlevel 1 exit /b 1

if not exist "%~dp0bin" mkdir "%~dp0bin"
cl /O2 /W3 /nologo /Fe:%~dp0bin\afd.exe %~dp0\afd.c
if errorlevel 1 exit /b 1
echo [MSVC] Build OK: c\bin\afd.exe
