#!/usr/bin/env bash
set -euo pipefail

# LastChaos Installation Script for Linux
# This script installs dependencies and sets up the project

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "========================================"
echo "LastChaos Linux Installation"
echo "========================================"

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS_NAME=$ID
    OS_VERSION=$VERSION_ID
else
    echo "Error: Cannot detect Linux distribution"
    exit 1
fi

echo "Detected OS: ${OS_NAME} ${OS_VERSION}"

# Install dependencies based on distribution
case "${OS_NAME}" in
    ubuntu|debian)
        echo "Installing dependencies for Ubuntu/Debian..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            git \
            wget \
            curl \
            libgl1-mesa-dev \
            libglu1-mesa-dev \
            libx11-dev \
            libxrandr-dev \
            libxinerama-dev \
            libxcursor-dev \
            libxi-dev \
            libxext-dev \
            libxkbcommon-dev \
            libwayland-dev \
            libwayland-egl1-mesa-dev \
            libvulkan-dev \
            vulkan-tools \
            mesa-utils \
            libssl-dev \
            libcurl4-openssl-dev \
            libarchive-dev \
            liblzma-dev \
            libbz2-dev \
            zlib1g-dev
        ;;
    fedora|rhel|centos)
        echo "Installing dependencies for Fedora/RHEL/CentOS..."
        sudo dnf install -y \
            gcc-c++ \
            make \
            cmake \
            ninja-build \
            git \
            wget \
            curl \
            mesa-libGL-devel \
            mesa-libGLU-devel \
            mesa-libGLES-devel \
            libX11-devel \
            libXrandr-devel \
            libXinerama-devel \
            libXcursor-devel \
            libXi-devel \
            libXext-devel \
            libXkbcommon-devel \
            wayland-devel \
            wayland-protocols-devel \
            vulkan-loader-devel \
            vulkan-tools \
            mesa-utils \
            openssl-devel \
            libcurl-devel \
            libarchive-devel \
            xz-devel \
            bzip2-devel \
            zlib-devel
        ;;
    arch|manjaro)
        echo "Installing dependencies for Arch/Manjaro..."
        sudo pacman -S --noconfirm \
            base-devel \
            cmake \
            ninja \
            git \
            wget \
            curl \
            mesa \
            libgl \
            libglu \
            libx11 \
            libxrandr \
            libxinerama \
            libxcursor \
            libxi \
            libxext \
            libxkbcommon \
            wayland \
            vulkan-icd-loader \
            vulkan-tools \
            mesa-utils \
            openssl \
            curl \
            libarchive \
            xz \
            bzip2 \
            zlib
        ;;
    *)
        echo "Warning: Unsupported Linux distribution. Please install dependencies manually."
        echo "Required packages: build-essential, cmake, git, libgl1-mesa-dev, libglu1-mesa-dev"
        echo "                  libx11-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev"
        echo "                  libxi-dev, libxext-dev, libvulkan-dev, libssl-dev, libcurl4-openssl-dev"
        ;;
esac

# Install Vulkan SDK if not present
if ! command -v vulkaninfo &> /dev/null; then
    echo "Vulkan SDK not found. Installing..."
    if command -v pacman &> /dev/null; then
        sudo pacman -S --noconfirm vulkan-sdk
    elif command -v apt-get &> /dev/null; then
        sudo apt-get install -y vulkan-tools
    elif command -v dnf &> /dev/null; then
        sudo dnf install -y vulkan-loader-devel
    fi
fi

# Create build directory
mkdir -p "${ROOT_DIR}/build/linux"

# Run build
echo ""
echo "Building LastChaos..."
cd "${ROOT_DIR}"
bash "${ROOT_DIR}/scripts/build_cross_platform.sh" Release

echo ""
echo "========================================"
echo "Installation complete!"
echo "========================================"
echo "Build directory: ${ROOT_DIR}/build/linux"
echo "To run: cd ${ROOT_DIR}/build/linux && ./LastChaos"
