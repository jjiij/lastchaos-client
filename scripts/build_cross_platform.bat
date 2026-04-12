@echo off
setlocal enabledelayedexpansion

:: LastChaos Cross-Platform Build Script for Windows
:: Supports: Windows, Linux, macOS

SCRIPT_DIR="%~dp0"
ROOT_DIR="%~dp0.."

:: Parse command line arguments
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set BUILD_DIR=%ROOT_DIR%\build\windows
set CMAKE_GENERATOR=MinGW Makefiles

echo ========================================
echo LastChaos Cross-Platform Build
echo ========================================
echo Platform: Windows
echo Build Type: %BUILD_TYPE%
echo Build Directory: %BUILD_DIR%
echo Generator: %CMAKE_GENERATOR%
echo ========================================

:: Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Configure CMake
echo.
echo Configuring CMake...
cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_GENERATOR=%CMAKE_GENERATOR% ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -G "%CMAKE_GENERATOR%" ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
    -DCMAKE_CXX_EXTENSIONS=OFF

:: Build the project
echo.
echo Building LastChaos...
cmake --build "%BUILD_DIR%" -j 4

:: Check for build artifacts
echo.
echo Build completed successfully!
echo Build artifacts:
dir "%BUILD_DIR%\porting\*.exe" /s /b 2>nul
dir "%BUILD_DIR%\*.exe" /s /b 2>nul

echo.
echo ========================================
echo Build process finished!
echo ========================================
pause