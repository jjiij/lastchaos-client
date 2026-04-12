#!/usr/bin/env bash
set -euo pipefail

# Cross-platform validation helper for macOS-first porting changes.
# - Runs native probe checks on the current host (Linux/macOS).
# - Prints explicit Windows x64 baseline commands for CI/manual runners.
#
# Optional env:
#   LASTCHAOS_VALIDATE_BUILD_DIR            (default: build/validate)
#   LASTCHAOS_VALIDATE_GENERATOR            (default: Unix Makefiles)
#   LASTCHAOS_VALIDATE_ARCHS               (default: x86_64;arm64 on macOS)
#   LASTCHAOS_VALIDATE_NKSP_PROBE          (default: 1 on macOS, 0 elsewhere)
#   LASTCHAOS_VALIDATE_NKSP_UNRESOLVED     (default: 1 on macOS when nksp probe is enabled)
#   LASTCHAOS_VALIDATE_NKSP_STRICT_LINK    (default: 0; set 1 to fail on unresolved symbols)
#   LASTCHAOS_VALIDATE_NKSP_NATIVE_BLUEPRINT (default: 0; set 1 on macOS to build native blueprint target)

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${LASTCHAOS_VALIDATE_BUILD_DIR:-${ROOT_DIR}/build/validate}"
GENERATOR="${LASTCHAOS_VALIDATE_GENERATOR:-Unix Makefiles}"
HOST_OS="$(uname -s)"

if [[ "${HOST_OS}" == "Darwin" ]]; then
  DEFAULT_ARCHS="x86_64;arm64"
  DEFAULT_VALIDATE_NKSP=1
else
  DEFAULT_ARCHS=""
  DEFAULT_VALIDATE_NKSP=0
fi

MACOS_ARCHS="${LASTCHAOS_VALIDATE_ARCHS:-${DEFAULT_ARCHS}}"
VALIDATE_NKSP="${LASTCHAOS_VALIDATE_NKSP_PROBE:-${DEFAULT_VALIDATE_NKSP}}"
VALIDATE_UNRESOLVED="${LASTCHAOS_VALIDATE_NKSP_UNRESOLVED:-1}"
VALIDATE_STRICT_LINK="${LASTCHAOS_VALIDATE_NKSP_STRICT_LINK:-0}"
VALIDATE_NATIVE_BLUEPRINT="${LASTCHAOS_VALIDATE_NKSP_NATIVE_BLUEPRINT:-0}"

mkdir -p "${BUILD_DIR}"

declare -a configure_args=(
  -S "${ROOT_DIR}"
  -B "${BUILD_DIR}"
  -G "${GENERATOR}"
  -DLASTCHAOS_ENABLE_NKSP_PROBE=ON
  -DLASTCHAOS_NKSP_STRICT_LINK=${VALIDATE_STRICT_LINK}
  -DLASTCHAOS_ENABLE_NKSP_NATIVE_BLUEPRINT=${VALIDATE_NATIVE_BLUEPRINT}
)

if [[ "${VALIDATE_NATIVE_BLUEPRINT}" == "1" ]]; then
  configure_args+=("-DLASTCHAOS_NKSP_NATIVE_STRICT_LINK=${VALIDATE_STRICT_LINK}")
fi

if [[ "${HOST_OS}" == "Darwin" && -n "${MACOS_ARCHS}" ]]; then
  configure_args+=("-DCMAKE_OSX_ARCHITECTURES=${MACOS_ARCHS}")
fi

echo "== Configure (${HOST_OS}) =="
cmake "${configure_args[@]}"

JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
echo "== Build portability probes =="
cmake --build "${BUILD_DIR}" --target lastchaos_porting_probe lastchaos_login_check -j "${JOBS}"

if [[ "${HOST_OS}" == "Darwin" && "${VALIDATE_NKSP}" == "1" ]]; then
  echo "== Build nksp_probe (macOS) =="
  cmake --build "${BUILD_DIR}" --target nksp_probe -j "${JOBS}"

  if [[ "${VALIDATE_UNRESOLVED}" == "1" ]]; then
    echo "== Report nksp_probe unresolved symbols =="
    "${ROOT_DIR}/scripts/report_nksp_unresolved_symbols.sh" "${BUILD_DIR}/nksp_probe"
  fi
fi

if [[ "${HOST_OS}" == "Darwin" && "${VALIDATE_NATIVE_BLUEPRINT}" == "1" ]]; then
  echo "== Build lastchaos_nksp_native_blueprint (macOS) =="
  cmake --build "${BUILD_DIR}" --target lastchaos_nksp_native_blueprint -j "${JOBS}"
fi

cat <<'EOF'
== Windows x64 baseline commands (run on Windows host/CI) ==
cmake -S . -B build\win64 -G "Visual Studio 17 2022" -A x64
cmake --build build\win64 --config Release --target GameMP
EOF
