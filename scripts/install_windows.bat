@echo off
setlocal enabledelayedexpansion

:: LastChaos Installation Script for Windows
:: This script installs dependencies and sets up the project

echo ========================================
echo LastChaos Windows Installation
echo ========================================

:: Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Error: Please run this script as Administrator
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo Installing dependencies...

:: Install Chocolatey if not present
where choco >nul 2>&1
if %errorLevel% neq 0 (
    echo Installing Chocolatey package manager...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))"
)

:: Install dependencies using Chocolatey
echo Installing dependencies...
choco install -y cmake ninja git curl vulkan-sdk glslang-tools

:: Set up environment variables
echo Setting up environment variables...
set PATH=%PATH%;C:\Program Files\CMake\bin;C:\Program Files\Git\cmd;C:\Program Files\Vulkan\SDK\%VULKAN_SDK%\Bin

:: Create build directory
mkdir "%~dp0build\windows" 2>nul

:: Run build
echo.
echo Building LastChaos...
cd /d "%~dp0"
call build_cross_platform.bat Release

echo.
echo ========================================
echo Installation complete!
echo ========================================
echo Build directory: %~dp0build\windows
echo To run: cd build\windows && LastChaos.exe
pause