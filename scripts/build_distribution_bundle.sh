#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage:
  build_distribution_bundle.sh --platform <windows|linux|macos> --assets-root <dir> [--runtime-root <dir>] [--out-dir <dir>] [--archive]

Description:
  Creates a distribution-ready directory that always includes full client assets from
  the external assets repository, then applies platform-specific runtime replacement rules.

Rules:
  - windows: keeps Windows launchers/runtime files from assets unless --runtime-root overlays replacements.
  - linux/macos: strips Windows-only launchers/runtime payloads and injects native replacements from --runtime-root.

Env overrides:
  LASTCHAOS_DIST_PLATFORM
  LASTCHAOS_DIST_ASSETS_ROOT
  LASTCHAOS_DIST_RUNTIME_ROOT
  LASTCHAOS_DIST_OUT_DIR
  LASTCHAOS_DIST_ARCHIVE=1
USAGE
}

PLATFORM="${LASTCHAOS_DIST_PLATFORM:-}"
ASSETS_ROOT="${LASTCHAOS_DIST_ASSETS_ROOT:-}"
RUNTIME_ROOT="${LASTCHAOS_DIST_RUNTIME_ROOT:-}"
OUT_DIR="${LASTCHAOS_DIST_OUT_DIR:-}"
MAKE_ARCHIVE="${LASTCHAOS_DIST_ARCHIVE:-0}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --platform) PLATFORM="$2"; shift 2 ;;
    --assets-root) ASSETS_ROOT="$2"; shift 2 ;;
    --runtime-root) RUNTIME_ROOT="$2"; shift 2 ;;
    --out-dir) OUT_DIR="$2"; shift 2 ;;
    --archive) MAKE_ARCHIVE=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown arg: $1" >&2; usage; exit 1 ;;
  esac
done

if [[ -z "${PLATFORM}" || -z "${ASSETS_ROOT}" ]]; then
  usage
  exit 1
fi
if [[ ! -d "${ASSETS_ROOT}" ]]; then
  echo "Assets root not found: ${ASSETS_ROOT}" >&2
  exit 2
fi
if [[ -n "${RUNTIME_ROOT}" && ! -d "${RUNTIME_ROOT}" ]]; then
  echo "Runtime root not found: ${RUNTIME_ROOT}" >&2
  exit 3
fi

case "${PLATFORM}" in
  windows|linux|macos) ;;
  *) echo "Unsupported platform: ${PLATFORM}" >&2; exit 4 ;;
esac

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${OUT_DIR:-${ROOT_DIR}/build/dist}"
STAGE_DIR="${OUT_DIR}/${PLATFORM}/LastChaos"
mkdir -p "${STAGE_DIR}"

# clean stage
find "${STAGE_DIR}" -mindepth 1 -maxdepth 1 -exec rm -rf {} +

# copy full assets payload into stage
cp -a "${ASSETS_ROOT}/." "${STAGE_DIR}/"

strip_windows_payload() {
  local target="$1"
  rm -f \
    "${target}/FastList4.exe" \
    "${target}/IP-change.exe" \
    "${target}/launcher.bat"

  if [[ -d "${target}/Bin" ]]; then
    find "${target}/Bin" -maxdepth 1 -type f \
      \( -iname '*.dll' -o -iname '*.exe' -o -iname '*.lib' -o -iname '*.exp' -o -iname '*.pdb' \) \
      -delete
  fi
}

if [[ "${PLATFORM}" == "windows" ]]; then
  if [[ -n "${RUNTIME_ROOT}" ]]; then
    if [[ -d "${RUNTIME_ROOT}/Bin" ]]; then
      mkdir -p "${STAGE_DIR}/Bin"
      cp -a "${RUNTIME_ROOT}/Bin/." "${STAGE_DIR}/Bin/"
    fi
    for f in FastList4.exe IP-change.exe launcher.bat; do
      if [[ -f "${RUNTIME_ROOT}/${f}" ]]; then
        cp -f "${RUNTIME_ROOT}/${f}" "${STAGE_DIR}/${f}"
      fi
    done
  fi
elif [[ "${PLATFORM}" == "linux" ]]; then
  strip_windows_payload "${STAGE_DIR}"
  mkdir -p "${STAGE_DIR}/Bin"

  if [[ -n "${RUNTIME_ROOT}" ]]; then
    # Prefer explicit native client names if present.
    for cand in LastChaos lastchaos lastchaos_client nksp; do
      if [[ -x "${RUNTIME_ROOT}/${cand}" ]]; then
        cp -f "${RUNTIME_ROOT}/${cand}" "${STAGE_DIR}/Bin/LastChaos"
        break
      fi
    done
    if [[ -x "${RUNTIME_ROOT}/lastchaos_login_check" && ! -f "${STAGE_DIR}/Bin/LastChaos" ]]; then
      cp -f "${RUNTIME_ROOT}/lastchaos_login_check" "${STAGE_DIR}/Bin/LastChaos"
    fi
  fi

  cat > "${STAGE_DIR}/launcher.sh" <<'LAUNCH'
#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "${DIR}/Bin/LastChaos" "$@"
LAUNCH
  chmod +x "${STAGE_DIR}/launcher.sh"
elif [[ "${PLATFORM}" == "macos" ]]; then
  strip_windows_payload "${STAGE_DIR}"

  if [[ -n "${RUNTIME_ROOT}" ]]; then
    if [[ -d "${RUNTIME_ROOT}/LastChaos.app" ]]; then
      cp -a "${RUNTIME_ROOT}/LastChaos.app" "${STAGE_DIR}/LastChaos.app"
    elif [[ -d "${RUNTIME_ROOT}" && "$(basename "${RUNTIME_ROOT}")" == "LastChaos.app" ]]; then
      cp -a "${RUNTIME_ROOT}" "${STAGE_DIR}/LastChaos.app"
    fi
  fi

  cat > "${STAGE_DIR}/open_lastchaos.command" <<'MACLAUNCH'
#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
open "${DIR}/LastChaos.app"
MACLAUNCH
  chmod +x "${STAGE_DIR}/open_lastchaos.command"
fi

if [[ "${MAKE_ARCHIVE}" == "1" ]]; then
  mkdir -p "${OUT_DIR}/archives"
  if command -v zip >/dev/null 2>&1; then
    (
      cd "${OUT_DIR}/${PLATFORM}"
      zip -qry "${OUT_DIR}/archives/lastchaos-${PLATFORM}.zip" "LastChaos"
    )
    echo "Archive: ${OUT_DIR}/archives/lastchaos-${PLATFORM}.zip"
  else
    tar -C "${OUT_DIR}/${PLATFORM}" -czf "${OUT_DIR}/archives/lastchaos-${PLATFORM}.tar.gz" "LastChaos"
    echo "Archive: ${OUT_DIR}/archives/lastchaos-${PLATFORM}.tar.gz"
  fi
fi

echo "Staged distribution: ${STAGE_DIR}"
