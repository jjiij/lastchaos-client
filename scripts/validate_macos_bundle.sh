#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_PATH="${1:-${ROOT_DIR}/build/macos/LastChaos.app}"
REPORT_DIR="${LASTCHAOS_BUNDLE_VALIDATION_DIR:-${ROOT_DIR}/build/macos}"
REPORT_FILE="${REPORT_DIR}/bundle_validation_$(uname -m).txt"
REQUIRED_ARCHS="${LASTCHAOS_EXPECT_CLIENT_ARCHS:-x86_64;arm64}"
RUN_LOGIN_SMOKE="${LASTCHAOS_RUN_LOGIN_SMOKE:-0}"
LOGIN_CMD="${LASTCHAOS_LOGIN_SMOKE_CMD:-}"

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "validate_macos_bundle.sh must run on macOS hosts." >&2
  exit 2
fi
if [[ ! -d "${APP_PATH}" ]]; then
  echo "App bundle not found: ${APP_PATH}" >&2
  exit 3
fi

mkdir -p "${REPORT_DIR}"
BIN_PATH="${APP_PATH}/Contents/MacOS/LastChaos"
if [[ ! -f "${BIN_PATH}" ]]; then
  BIN_PATH="${APP_PATH}/Contents/MacOS/LastChaosLauncher.bin"
fi
if [[ ! -f "${BIN_PATH}" ]]; then
  echo "No runnable Mach-O found in ${APP_PATH}/Contents/MacOS" >&2
  exit 4
fi

{
  echo "LastChaos macOS bundle validation"
  echo "Date (UTC): $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  echo "Host: $(sw_vers -productName) $(sw_vers -productVersion) ($(uname -m))"
  echo "Bundle: ${APP_PATH}"
  echo "Binary: ${BIN_PATH}"

  if command -v lipo >/dev/null 2>&1; then
    BIN_ARCHS="$(lipo -archs "${BIN_PATH}" 2>/dev/null || true)"
    echo "Binary archs: ${BIN_ARCHS:-<unknown>}"
    for arch in ${REQUIRED_ARCHS//;/ }; do
      if [[ " ${BIN_ARCHS} " != *" ${arch} "* ]]; then
        echo "ERROR: missing required binary arch ${arch}"
        exit 10
      fi
    done
  fi

  echo "-- codesign verify --"
  codesign --verify --verbose=2 "${APP_PATH}"

  echo "-- codesign display --"
  codesign --display --verbose=2 "${APP_PATH}" 2>&1

  if command -v spctl >/dev/null 2>&1; then
    echo "-- spctl assess --"
    spctl --assess --type execute --verbose=4 "${APP_PATH}"
  fi

  if [[ -f "${APP_PATH}/Contents/Resources/Game/sl.dta" ]]; then
    echo "Found login config: Contents/Resources/Game/sl.dta"
  else
    echo "ERROR: missing Contents/Resources/Game/sl.dta"
    exit 11
  fi

  if [[ -d "${APP_PATH}/Contents/Resources/Game/Data" || -d "${APP_PATH}/Contents/Resources/Game/data" ]]; then
    echo "Found bundled game data directory"
  else
    echo "ERROR: missing bundled Data/ directory"
    exit 12
  fi

  if [[ "${RUN_LOGIN_SMOKE}" == "1" ]]; then
    if [[ -z "${LOGIN_CMD}" ]]; then
      echo "ERROR: LASTCHAOS_RUN_LOGIN_SMOKE=1 requires LASTCHAOS_LOGIN_SMOKE_CMD"
      exit 13
    fi
    echo "-- login smoke --"
    echo "Command: ${LOGIN_CMD}"
    eval "${LOGIN_CMD}"
  else
    echo "Login smoke not executed (set LASTCHAOS_RUN_LOGIN_SMOKE=1 and LASTCHAOS_LOGIN_SMOKE_CMD)."
  fi

  echo "Validation: PASS"
} | tee "${REPORT_FILE}"

echo "Wrote validation report: ${REPORT_FILE}"
