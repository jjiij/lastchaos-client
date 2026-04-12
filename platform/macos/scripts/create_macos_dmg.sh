#!/usr/bin/env bash
set -euo pipefail

# Wraps an existing LastChaos.app into a single distributable .dmg (disk image).
# Usage:
#   ./platform/macos/scripts/create_macos_dmg.sh [path-to/LastChaos.app] [output-name.dmg]
#
# Defaults:
#   App:  client-source/build/macos/LastChaos.app
#   DMG:  client-source/build/macos/LastChaos.dmg

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/macos"
APP_SRC="${1:-${BUILD_DIR}/LastChaos.app}"
DMG_OUT="${2:-${BUILD_DIR}/LastChaos.dmg}"

if [[ ! -d "${APP_SRC}" ]]; then
  echo "App bundle not found: ${APP_SRC}" >&2
  exit 2
fi

APP_NAME="$(basename "${APP_SRC}")"
STAGING="$(mktemp -d "/tmp/lastchaos-dmg.XXXXXX")"
cleanup() { rm -rf "${STAGING}"; }
trap cleanup EXIT

mkdir -p "${STAGING}/dist"
cp -R "${APP_SRC}" "${STAGING}/dist/${APP_NAME}"

# Uncompressed UDZO is widely compatible; -volname shows in Finder sidebar.
rm -f "${DMG_OUT}"
hdiutil create -volname "LastChaos" -srcfolder "${STAGING}/dist" -ov -format UDZO "${DMG_OUT}"

echo "Created: ${DMG_OUT}"
