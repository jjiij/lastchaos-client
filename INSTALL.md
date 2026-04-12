# LastChaos Cross-Platform Installation Guide

This guide explains how to install and build LastChaos on Windows, Linux, and macOS.

## Prerequisites

### Windows
- Visual Studio 2019 or later (with C++ desktop development workload)
- CMake (3.15 or later)
- Git for Windows
- Vulkan SDK (1.3 or later)
- Chocolatey (for automated dependency installation)

### Linux
- GCC or Clang compiler
- CMake (3.15 or later)
- Ninja build system
- Git
- Vulkan SDK
- OpenGL libraries (Mesa)

### macOS
- Xcode Command Line Tools
- Homebrew
- CMake (3.15 or later)
- Ninja build system
- Vulkan SDK
- OpenGL libraries (Mesa)

## Installation

### Windows

1. **Run the installer as Administrator:**
   ```cmd
   right-click install_windows.bat and select "Run as administrator"
   ```

2. **Or install manually:**
   - Install Visual Studio with C++ desktop development
   - Install CMake, Git, and Vulkan SDK
   - Run `build_cross_platform.bat`

### Linux

1. **Run the installer:**
   ```bash
   bash install_linux.sh
   ```

2. **Or install manually:**
   - Install dependencies based on your distribution (see below)
   - Run `bash build_cross_platform.sh`

### macOS

1. **Run the installer:**
   ```bash
   bash install_macos.sh
   ```

2. **Or install manually:**
   - Install Xcode Command Line Tools
   - Install Homebrew
   - Run `bash build_cross_platform.sh`

## Building

After installation, build the project:

### Windows
```cmd
cd build\windows
cmake --build . --config Release
```

### Linux/macOS
```bash
cd build/linux  # or build/macos
cmake --build .
```

## Running

### Windows
```cmd
cd build\windows\Release
LastChaos.exe
```

### Linux/macOS
```bash
cd build/linux  # or build/macos
./LastChaos
```

## Dependencies by Distribution

### Ubuntu/Debian
```bash
sudo apt-get install build-essential cmake ninja-build git wget curl libgl1-mesa-dev libglu1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev libxkbcommon-dev libwayland-dev libwayland-egl1-mesa-dev libvulkan-dev vulkan-tools mesa-utils libssl-dev libcurl4-openssl-dev libarchive-dev liblzma-dev libbz2-dev zlib1g-dev
```

### Fedora/RHEL/CentOS
```bash
sudo dnf install gcc-c++ make cmake ninja-build git wget curl mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel libXext-devel libXkbcommon-devel wayland-devel wayland-protocols-devel vulkan-loader-devel vulkan-tools mesa-utils openssl-devel libcurl-devel libarchive-devel xz-devel bzip2-devel zlib-devel
```

### Arch/Manjaro
```bash
sudo pacman -S base-devel cmake ninja git wget curl mesa libgl libglu libx11 libxrandr libxinerama libxcursor libxi libxext libxkbcommon wayland vulkan-icd-loader vulkan-tools mesa-utils openssl curl libarchive xz bzip2 zlib
```

## Troubleshooting

### Vulkan Not Found
Ensure Vulkan SDK is installed and added to your PATH:
- Windows: Add `C:\VulkanSDK\[version]\Bin` to PATH
- Linux: Install `vulkan-tools` package
- macOS: Install via Homebrew

### OpenGL Errors
- Windows: Ensure graphics drivers are up to date
- Linux: Install `mesa-utils` and `libgl1-mesa-dev`
- macOS: Install via Homebrew

### Build Errors
- Clean build directory: `rm -rf build/*` (Linux/macOS) or `rmdir /s /q build` (Windows)
- Reconfigure: `cmake -B build -S .`
- Rebuild: `cmake --build build`

## Project Structure

```
lastchaos/
├── client-source/          # Client source code
├── server-main/            # Server source code
├── _references/            # Reference archives (read-only)
├── _archive/               # Client backups (read-only)
├── build/                  # Build output directories
│   ├── linux/
│   ├── macos/
│   └── windows/
└── scripts/                # Installation and build scripts
    ├── install_linux.sh
    ├── install_macos.sh
    ├── install_windows.bat
    ├── build_cross_platform.sh
    └── build_cross_platform.bat
```