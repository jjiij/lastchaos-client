#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/macos"
APP_NAME="LastChaos.app"
APP_DIR="${BUILD_DIR}/${APP_NAME}"
TEMPLATE_DIR="${ROOT_DIR}/macos/AppTemplate"
# shellcheck source=lib_macos_app_bundle.sh
source "${ROOT_DIR}/scripts/lib_macos_app_bundle.sh"

# Required inputs for a playable bundle.
: "${LASTCHAOS_CLIENT_BINARY:?Set LASTCHAOS_CLIENT_BINARY to the native macOS client executable path}"
: "${LASTCHAOS_ASSET_ROOT:?Set LASTCHAOS_ASSET_ROOT to the client asset root (contains Data/)}"

if [[ ! -f "${LASTCHAOS_CLIENT_BINARY}" ]]; then
  echo "Missing client binary: ${LASTCHAOS_CLIENT_BINARY}" >&2
  exit 2
fi

if [[ ! -d "${LASTCHAOS_ASSET_ROOT}" ]]; then
  echo "Missing asset root: ${LASTCHAOS_ASSET_ROOT}" >&2
  exit 3
fi

if [[ ! -d "${LASTCHAOS_ASSET_ROOT}/Data" && ! -d "${LASTCHAOS_ASSET_ROOT}/data" ]]; then
  echo "Asset root must contain Data/ or data/: ${LASTCHAOS_ASSET_ROOT}" >&2
  exit 4
fi

LC_LOGIN_NAME="${LASTCHAOS_LOGIN_NAME:-Local}"
LC_LOGIN_HOST="${LASTCHAOS_LOGIN_HOST:-127.0.0.1}"
LC_LOGIN_PORT="${LASTCHAOS_LOGIN_PORT:-4001}"
LC_LOGIN_VERSION="${LASTCHAOS_LOGIN_VERSION:-1000}"
LC_LOGIN_USERS="${LASTCHAOS_LOGIN_USERS:-300}"
LC_MACOS_ARCHS="${LASTCHAOS_MACOS_ARCHS:-x86_64;arm64}"
LC_BUNDLE_MANIFEST="${BUILD_DIR}/LastChaos.bundle_manifest.txt"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G "Unix Makefiles" \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_OSX_ARCHITECTURES="${LC_MACOS_ARCHS}" \
  -DLASTCHAOS_LOGIN_NAME="${LC_LOGIN_NAME}" \
  -DLASTCHAOS_LOGIN_HOST="${LC_LOGIN_HOST}" \
  -DLASTCHAOS_LOGIN_PORT="${LC_LOGIN_PORT}" \
  -DLASTCHAOS_LOGIN_VERSION="${LC_LOGIN_VERSION}" \
  -DLASTCHAOS_LOGIN_USERS="${LC_LOGIN_USERS}"
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
cmake --build "${BUILD_DIR}" --target lastchaos_local_sl_dta -j "${JOBS}"

rm -rf "${APP_DIR}"
mkdir -p "${APP_DIR}"
cp -R "${TEMPLATE_DIR}/Contents" "${APP_DIR}/"

bundle_game_resources_if_available "${APP_DIR}" "${LASTCHAOS_ASSET_ROOT}"
install_macos_terminal_launcher "${APP_DIR}" "${LASTCHAOS_CLIENT_BINARY}"
install_sl_dta_for_bundle "${APP_DIR}" "${BUILD_DIR}/porting/sl.dta"
install_macos_vulkan_runtime_if_available "${APP_DIR}" "${LASTCHAOS_MOLTENVK_DYLIB:-}"
write_macos_bundle_manifest \
  "${APP_DIR}" \
  "${LC_BUNDLE_MANIFEST}" \
  "${LC_LOGIN_NAME}" \
  "${LC_LOGIN_HOST}" \
  "${LC_LOGIN_PORT}" \
  "${LC_LOGIN_VERSION}" \
  "${LC_LOGIN_USERS}" \
  "${LC_MACOS_ARCHS}"

sign_macos_app_bundle "${APP_DIR}"
verify_macos_app_bundle_security "${APP_DIR}"

cat <<EOF
Created playable app bundle:
${APP_DIR}

Launcher (opens Terminal):
${APP_DIR}/Contents/MacOS/LastChaosLauncher

Assets:
${APP_DIR}/Contents/Resources/Game

Login config:
${APP_DIR}/Contents/MacOS/sl.dta
${APP_DIR}/Contents/Resources/Game/sl.dta

Bundled Vulkan runtime (if found/provided):
${APP_DIR}/Contents/Frameworks/libvulkan.1.dylib

Bundle verification:
codesign --verify --verbose=2 ${APP_DIR}
codesign --display --verbose=2 ${APP_DIR}
spctl --assess --type execute --verbose=4 ${APP_DIR}   (when available)

Login endpoint entry:
${LC_LOGIN_NAME} ${LC_LOGIN_HOST}:${LC_LOGIN_PORT} (ver=${LC_LOGIN_VERSION}, users=${LC_LOGIN_USERS})

Requested macOS architectures (CMake):
${LC_MACOS_ARCHS}

Packaging manifest:
${LC_BUNDLE_MANIFEST}
EOF
