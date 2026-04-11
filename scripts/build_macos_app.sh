#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/macos"
APP_NAME="LastChaos.app"
APP_DIR="${BUILD_DIR}/${APP_NAME}"
TEMPLATE_DIR="${ROOT_DIR}/macos/AppTemplate"
# shellcheck source=lib_macos_app_bundle.sh
source "${ROOT_DIR}/scripts/lib_macos_app_bundle.sh"

LC_LOGIN_NAME="${LASTCHAOS_LOGIN_NAME:-Local}"
LC_LOGIN_HOST="${LASTCHAOS_LOGIN_HOST:-127.0.0.1}"
LC_LOGIN_PORT="${LASTCHAOS_LOGIN_PORT:-4001}"
LC_LOGIN_VERSION="${LASTCHAOS_LOGIN_VERSION:-1000}"
LC_LOGIN_USERS="${LASTCHAOS_LOGIN_USERS:-300}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G "Unix Makefiles" \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DLASTCHAOS_LOGIN_NAME="${LC_LOGIN_NAME}" \
  -DLASTCHAOS_LOGIN_HOST="${LC_LOGIN_HOST}" \
  -DLASTCHAOS_LOGIN_PORT="${LC_LOGIN_PORT}" \
  -DLASTCHAOS_LOGIN_VERSION="${LC_LOGIN_VERSION}" \
  -DLASTCHAOS_LOGIN_USERS="${LC_LOGIN_USERS}"
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
cmake --build "${BUILD_DIR}" --target lastchaos_login_check -j "${JOBS}"

rm -rf "${APP_DIR}"
mkdir -p "${APP_DIR}"
cp -R "${TEMPLATE_DIR}/Contents" "${APP_DIR}/"

# Optional: full client Data tree (explicit env or default game-assets/).
ASSET_ROOT="${LASTCHAOS_ASSET_ROOT:-${ROOT_DIR}/game-assets}"
bundle_game_resources_if_available "${APP_DIR}" "${ASSET_ROOT}"

if [[ -n "${LASTCHAOS_CLIENT_BINARY:-}" ]]; then
  if [[ ! -f "${LASTCHAOS_CLIENT_BINARY}" ]]; then
    echo "LASTCHAOS_CLIENT_BINARY does not exist: ${LASTCHAOS_CLIENT_BINARY}" >&2
    exit 2
  fi
  install_macos_terminal_launcher "${APP_DIR}" "${LASTCHAOS_CLIENT_BINARY}"
else
  install_macos_terminal_launcher "${APP_DIR}" "${BUILD_DIR}/porting/lastchaos_login_check"
fi

install_sl_dta_for_bundle "${APP_DIR}" "${BUILD_DIR}/porting/sl.dta"
install_macos_vulkan_runtime_if_available "${APP_DIR}" "${LASTCHAOS_MOLTENVK_DYLIB:-}"

sign_macos_app_bundle "${APP_DIR}"

cat <<EOF
Created app bundle:
${APP_DIR}

Main executable (opens Terminal):
${APP_DIR}/Contents/MacOS/LastChaosLauncher

Inner script:
${APP_DIR}/Contents/MacOS/LastChaosInner.command

Payload binary:
${APP_DIR}/Contents/MacOS/LastChaosLauncher.bin

Login config:
${APP_DIR}/Contents/MacOS/sl.dta

Bundled Vulkan runtime (if found/provided):
${APP_DIR}/Contents/Frameworks/libvulkan.1.dylib

Login endpoint entry:
${LC_LOGIN_NAME} ${LC_LOGIN_HOST}:${LC_LOGIN_PORT} (ver=${LC_LOGIN_VERSION}, users=${LC_LOGIN_USERS})
EOF

if [[ -d "${APP_DIR}/Contents/Resources/Game/Data" || -d "${APP_DIR}/Contents/Resources/Game/data" ]]; then
  echo ""
  echo "Bundled game data: ${APP_DIR}/Contents/Resources/Game"
else
  echo ""
  echo "No Data/ bundled yet. Add client files under ${ROOT_DIR}/game-assets/ or set LASTCHAOS_ASSET_ROOT."
fi
