#!/usr/bin/env bash
# Build a *full* LastChaos.app: native client binary + entire Data/ tree + sl.dta + Terminal launcher + codesign.
#
# The CMake tree does not yet produce a playable macOS game executable,
# and the repository does not ship client assets. You must supply:
#   LASTCHAOS_CLIENT_BINARY  — path to the macOS client Mach-O
#   LASTCHAOS_ASSET_ROOT     — folder whose contents include Data/ or data/
# Optional:
#   LASTCHAOS_MOLTENVK_DYLIB — path to libMoltenVK.dylib or libvulkan*.dylib to bundle into .app
#
# You can store these in client-source/.lastchaos_full_bundle.env (see .lastchaos_full_bundle.env.example).
#
# Usage:
#   ./scripts/build_full_macos_app.sh
#   LASTCHAOS_CLIENT_BINARY=/path/to/client LASTCHAOS_ASSET_ROOT=/path/to/root ./scripts/build_full_macos_app.sh

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENV_FILE="${ROOT_DIR}/.lastchaos_full_bundle.env"
EXPECTED_ARCHS="${LASTCHAOS_EXPECT_CLIENT_ARCHS:-${LASTCHAOS_MACOS_ARCHS:-x86_64;arm64}}"

if [[ -f "${ENV_FILE}" ]]; then
  set -a
  # shellcheck disable=SC1090
  source "${ENV_FILE}"
  set +a
fi

if [[ -z "${LASTCHAOS_CLIENT_BINARY:-}" || -z "${LASTCHAOS_ASSET_ROOT:-}" ]]; then
  cat >&2 <<'EOF'
build_full_macos_app.sh: missing inputs.

Set environment variables (or create client-source/.lastchaos_full_bundle.env):

  LASTCHAOS_CLIENT_BINARY   Absolute path to the macOS game client executable (Mach-O).
  LASTCHAOS_ASSET_ROOT      Absolute path to a directory that contains Data/ or data/

This repository does not currently build a playable macOS client from source (see NATIVE_MAC_BLOCKERS.md),
and does not include the multi-gigabyte client asset tree. Copy Data from a Windows client install
or extracted client package into a folder and point LASTCHAOS_ASSET_ROOT at its parent.

Example:

  LASTCHAOS_CLIENT_BINARY="$HOME/lc-macos/LastChaos" \
  LASTCHAOS_ASSET_ROOT="$HOME/lc-windows-client" \
  ./scripts/build_full_macos_app.sh

Optional login endpoint override:

  LASTCHAOS_LOGIN_NAME="Local Docker" \
  LASTCHAOS_LOGIN_HOST="127.0.0.1" \
  LASTCHAOS_LOGIN_PORT="4001" \
  ./scripts/build_full_macos_app.sh

Copy .lastchaos_full_bundle.env.example to .lastchaos_full_bundle.env and edit paths there.
EOF
  exit 1
fi

if [[ ! -f "${LASTCHAOS_CLIENT_BINARY}" ]]; then
  echo "LASTCHAOS_CLIENT_BINARY is not a file: ${LASTCHAOS_CLIENT_BINARY}" >&2
  exit 2
fi
if [[ ! -d "${LASTCHAOS_ASSET_ROOT}" ]]; then
  echo "LASTCHAOS_ASSET_ROOT is not a directory: ${LASTCHAOS_ASSET_ROOT}" >&2
  exit 3
fi
if [[ ! -d "${LASTCHAOS_ASSET_ROOT}/Data" && ! -d "${LASTCHAOS_ASSET_ROOT}/data" ]]; then
  echo "LASTCHAOS_ASSET_ROOT must contain Data/ or data/: ${LASTCHAOS_ASSET_ROOT}" >&2
  exit 4
fi

if command -v lipo >/dev/null 2>&1; then
  BIN_ARCHS="$(lipo -archs "${LASTCHAOS_CLIENT_BINARY}" 2>/dev/null || true)"
  if [[ -n "${BIN_ARCHS}" ]]; then
    for arch in ${EXPECTED_ARCHS//;/ }; do
      if [[ " ${BIN_ARCHS} " != *" ${arch} "* ]]; then
        echo "Client binary arch check failed: expected '${arch}' in '${BIN_ARCHS}'." >&2
        echo "Set LASTCHAOS_EXPECT_CLIENT_ARCHS to adjust required arch list if needed." >&2
        exit 5
      fi
    done
    echo "Client binary archs: ${BIN_ARCHS} (required: ${EXPECTED_ARCHS})"
  else
    echo "Warning: unable to read client binary architectures with lipo: ${LASTCHAOS_CLIENT_BINARY}" >&2
  fi
fi

export LASTCHAOS_CLIENT_BINARY
export LASTCHAOS_ASSET_ROOT

"${ROOT_DIR}/scripts/package_full_macos_app.sh"

if [[ "${LASTCHAOS_CREATE_DMG:-0}" == "1" ]]; then
  "${ROOT_DIR}/scripts/create_macos_dmg.sh"
fi

echo ""
echo "Full macOS app ready:"
echo "  ${ROOT_DIR}/build/macos/LastChaos.app"
if [[ "${LASTCHAOS_CREATE_DMG:-0}" == "1" ]]; then
  echo "  ${ROOT_DIR}/build/macos/LastChaos.dmg"
fi
