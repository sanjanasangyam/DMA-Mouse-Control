@echo off
REM Simple batch file to build with Visual Studio environment

echo === Building DMA Mouse Handler ===
echo.

REM Find and load Visual Studio environment
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

if not defined VS_PATH (
    set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"
)

if exist "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" (
    echo Loading Visual Studio environment...
    call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
    echo.
) else (
    echo ERROR: Could not find Visual Studio installation
    echo Please install Visual Studio 2022 with C++ Desktop Development
    pause
    exit /b 1
)

REM Ensure bin directory exists
if not exist "bin" (
    echo Creating bin directory...
    mkdir bin
    echo.
)

REM Build MouseHandler
echo Building MouseHandler.exe (Main PC)...
cl /nologo /O2 /EHsc src\MouseHandler.cpp user32.lib /Fe:bin\MouseHandler.exe
if %ERRORLEVEL% EQU 0 (
    echo [✓] MouseHandler.exe built successfully!
) else (
    echo [✗] Failed to build MouseHandler.exe
)
echo.

REM Build Controller
echo Building Controller.exe (Radar PC)...
cl /nologo /O2 /EHsc src\Controller.cpp /I. external\memprocfs\vmm.lib user32.lib /Fe:bin\Controller.exe
if %ERRORLEVEL% EQU 0 (
    echo [✓] Controller.exe built successfully!
) else (
    echo [✗] Failed to build Controller.exe
)
echo.

REM Build Example (Library usage demo)
echo Building Example.exe (Library Demo)...
cl /nologo /O2 /EHsc src\Example.cpp src\DMAMouse.cpp /I. external\memprocfs\vmm.lib user32.lib /Fe:bin\Example.exe
if %ERRORLEVEL% EQU 0 (
    echo [✓] Example.exe built successfully!
) else (
    echo [✗] Failed to build Example.exe
)
echo.

REM Copy MemProcFS DLLs to executable directory
echo Copying MemProcFS DLLs...
copy /Y external\memprocfs\*.dll bin\ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [✓] DLLs copied successfully!
) else (
    echo [!] Warning: Could not copy DLLs
)
echo.

echo === Build Complete ===
echo.
echo Executables built:
echo   - bin\MouseHandler.exe (Run on target PC)
echo   - bin\Controller.exe (Full controller with cursor tracking)
echo   - bin\Example.exe (Simple library usage demo)
echo.
echo To use the library in your own code:
echo   1. Include src\DMAMouse.h in your project
echo   2. Link src\DMAMouse.cpp, external\memprocfs\vmm.lib, user32.lib
echo   3. See Example.cpp for usage
echo.

REM Clean up build artifacts
del *.obj 2>nul

pause
