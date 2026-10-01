@echo off
setlocal

echo ==========================================
echo     PLC MODBUS - BUILD AND RUN
echo ==========================================
echo.

cd /d "%~dp0"


REM ============================================================
REM CLEAN OLD BUILD
REM ============================================================

echo Cleaning old build directory...
echo.

if exist build (
    rmdir /s /q build
)

mkdir build

cd build


REM ============================================================
REM CMAKE CONFIGURE
REM ============================================================

echo.
echo Configuring CMake...
echo.

cmake ..

if errorlevel 1 (
    echo.
    echo ==========================================
    echo CMAKE CONFIGURATION FAILED
    echo ==========================================
    echo.
    pause
    exit /b 1
)


REM ============================================================
REM BUILD RELEASE
REM ============================================================

echo.
echo Building Release...
echo.

cmake --build . --config Release

if errorlevel 1 (
    echo.
    echo ==========================================
    echo BUILD FAILED
    echo ==========================================
    echo.
    pause
    exit /b 1
)


REM ============================================================
REM RUN
REM ============================================================

echo.
echo ==========================================
echo BUILD SUCCESSFUL
echo ==========================================
echo.

echo Starting PLC Modbus application...
echo.

"%~dp0build\Release\PLC_Modbus.exe"


exit /b 0
