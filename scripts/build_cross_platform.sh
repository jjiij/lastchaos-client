#!/usr/bin/env bash
set -euo pipefail

# LastChaos Cross-Platform Build Script
# Supports: Windows, Linux, macOS

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Parse command line arguments
BUILD_TYPE="${1:-Release}"
BUILD_DIR="${ROOT_DIR}/build/${BUILD_TYPE}"
CMAKE_GENERATOR="${CMAKE_GENERATOR:-Unix Makefiles}"

# Platform-specific settings
case "$(uname -s)" in
    Darwin*)
        PLATFORM="macOS"
        CMAKE_GENERATOR="${CMAKE_GENERATOR:-Xcode}"
        ;;
    Linux*)
        PLATFORM="Linux"
        CMAKE_GENERATOR="${CMAKE_GENERATOR:-Unix Makefiles}"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        PLATFORM="Windows"
        CMAKE_GENERATOR="${CMAKE_GENERATOR:-MinGW Makefiles}"
        ;;
    *)
        PLATFORM="Unknown"
        ;;
esac

echo "========================================"
echo "LastChaos Cross-Platform Build"
echo "========================================"
echo "Platform: ${PLATFORM}"
echo "Build Type: ${BUILD_TYPE}"
echo "Build Directory: ${BUILD_DIR}"
echo "Generator: ${CMAKE_GENERATOR}"
echo "========================================"

# Create build directory
mkdir -p "${BUILD_DIR}"

# Configure CMake
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_GENERATOR="${CMAKE_GENERATOR}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -G "${CMAKE_GENERATOR}" \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF

# Build the project
echo ""
echo "Building LastChaos..."
cmake --build "${BUILD_DIR}" -j "$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

# Check for build artifacts
echo ""
echo "Build completed successfully!"
echo "Build artifacts:"
find "${BUILD_DIR}" -name "*.exe" -o -name "LastChaos" -o -name "LastChaos.app" 2>/dev/null | head -10

# Platform-specific post-build actions
case "${PLATFORM}" in
    macOS)
        echo ""
        echo "macOS: Creating app bundle..."
        if [ -f "${SCRIPT_DIR}/build_macos_app.sh" ]; then
            bash "${SCRIPT_DIR}/build_macos_app.sh"
        fi
        ;;
    Linux)
        echo ""
        echo "Linux: Build complete. Check ${BUILD_DIR} for binaries."
        ;;
    Windows)
        echo ""
        echo "Windows: Build complete. Check ${BUILD_DIR} for binaries."
        ;;
esac

echo ""
echo "========================================"
echo "Build process finished!"
echo "========================================"