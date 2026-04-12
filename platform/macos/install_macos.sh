#!/usr/bin/env bash
set -euo pipefail

# LastChaos Installation Script for macOS
# This script installs dependencies and sets up the project

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "========================================"
echo "LastChaos macOS Installation"
echo "========================================"

# Check if running on macOS
if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "Error: This script is only for macOS"
    exit 1
fi

# Check for Homebrew
if ! command -v brew &> /dev/null; then
    echo "Homebrew not found. Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

echo "Installing dependencies using Homebrew..."
brew install cmake ninja git wget curl

# Install Vulkan SDK
if ! command -v vulkaninfo &> /dev/null; then
    echo "Vulkan SDK not found. Installing..."
    brew install vulkan-tools
fi

# Install OpenGL and related libraries
brew install mesa mesa-utils

# Install additional dependencies
brew install openssl curl libarchive xz bzip2 zlib

# Create build directory
mkdir -p "${ROOT_DIR}/build/macos"

# Run build
echo ""
echo "Building LastChaos..."
cd "${ROOT_DIR}"
bash "${ROOT_DIR}/scripts/build_cross_platform.sh" Release

echo ""
echo "========================================"
echo "Installation complete!"
echo "========================================"
echo "Build directory: ${ROOT_DIR}/build/macos"
echo "To run: cd ${ROOT_DIR}/build/macos && ./LastChaos"
