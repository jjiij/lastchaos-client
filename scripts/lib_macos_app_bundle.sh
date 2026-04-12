#!/usr/bin/env bash
# Shared helpers for LastChaos.app packaging (build_macos_app.sh, package_full_macos_app.sh).

# Install Terminal-visible launcher: CFBundleExecutable runs a tiny script that opens
# LastChaosInner.command in Terminal (CLI binaries are invisible when double-clicked from Finder).

install_macos_terminal_launcher() {
  local app_dir="$1"
  local real_binary="$2"
  local macos="${app_dir}/Contents/MacOS"

  if [[ ! -f "${real_binary}" ]]; then
    echo "install_macos_terminal_launcher: missing binary: ${real_binary}" >&2
    return 1
  fi

  mkdir -p "${macos}"
  cp -f "${real_binary}" "${macos}/LastChaosLauncher.bin"
  chmod +x "${macos}/LastChaosLauncher.bin"

  cat > "${macos}/LastChaosInner.command" <<'EOS'
#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
GAME="${DIR}/../Resources/Game"
APP_ROOT="$(cd "${DIR}/.." && pwd)"
FRAMEWORKS="${APP_ROOT}/Frameworks"
printf '\033]0;LastChaos\007'
echo "LastChaos"
echo "----------------------------------------"
if [[ -d "${GAME}/Data" || -d "${GAME}/data" ]]; then
  echo "Bundled game data: yes"
  echo "  ${GAME}"
else
  echo "Bundled game data: not found."
  echo "Copy or symlink the client folder that contains Data/ to:"
  echo "  ${GAME}/"
  echo "(see client-source/game-assets/README.txt)"
fi
if [[ -f "${FRAMEWORKS}/libvulkan.1.dylib" ]]; then
  echo "Bundled Vulkan runtime: ${FRAMEWORKS}/libvulkan.1.dylib"
else
  echo "Bundled Vulkan runtime: not found in ${FRAMEWORKS}"
fi
echo "Requested gfx API hint (LC_GFX_API): ${LC_GFX_API:-auto}"
echo "DYLD_LIBRARY_PATH: ${DYLD_LIBRARY_PATH:-<unset>}"
echo "----------------------------------------"
"${DIR}/LastChaosLauncher.bin" 2>&1 || true
echo "----------------------------------------"
read -r -p "Press Enter to close this window... " _
EOS
  chmod +x "${macos}/LastChaosInner.command"

  cat > "${macos}/LastChaosLauncher" <<'EOS'
#!/usr/bin/env bash
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
exec open -a Terminal "${DIR}/LastChaosInner.command"
EOS
  chmod +x "${macos}/LastChaosLauncher"
}

# Copy sl.dta next to tools and under Resources/Game when that directory exists.
install_sl_dta_for_bundle() {
  local app_dir="$1"
  local sl_dta="$2"
  local macos="${app_dir}/Contents/MacOS"
  local game="${app_dir}/Contents/Resources/Game"

  cp -f "${sl_dta}" "${macos}/sl.dta"
  if [[ -d "${game}" ]]; then
    cp -f "${sl_dta}" "${game}/sl.dta"
  fi
}

# Optional: rsync full client payload (must contain Data/ or data/).
bundle_game_resources_if_available() {
  local app_dir="$1"
  local asset_root="$2"

  if [[ -z "${asset_root}" || ! -d "${asset_root}" ]]; then
    return 0
  fi
  if [[ ! -d "${asset_root}/Data" && ! -d "${asset_root}/data" ]]; then
    echo "bundle_game_resources_if_available: no Data/ under ${asset_root}, skipping." >&2
    return 0
  fi

  local game="${app_dir}/Contents/Resources/Game"
  mkdir -p "${game}"
  rsync -a --delete "${asset_root}/" "${game}/"
  echo "Bundled game resources into ${game}"
}

# Bundle Vulkan runtime for macOS so users do not need a separate install.
# Input can be either libMoltenVK.dylib or libvulkan*.dylib.
# If omitted, common Homebrew locations are probed.
install_macos_vulkan_runtime_if_available() {
  local app_dir="$1"
  local maybe_dylib="${2:-}"
  local frameworks="${app_dir}/Contents/Frameworks"
  local src="${maybe_dylib}"

  if [[ "$(uname -s)" != "Darwin" ]]; then
    return 0
  fi

  if [[ -z "${src}" ]]; then
    for candidate in \
      "/opt/homebrew/lib/libMoltenVK.dylib" \
      "/usr/local/lib/libMoltenVK.dylib" \
      "/opt/homebrew/lib/libvulkan.1.dylib" \
      "/usr/local/lib/libvulkan.1.dylib" \
      "/opt/homebrew/lib/libvulkan.dylib" \
      "/usr/local/lib/libvulkan.dylib"
    do
      if [[ -f "${candidate}" ]]; then
        src="${candidate}"
        break
      fi
    done
  fi

  if [[ -z "${src}" ]]; then
    echo "install_macos_vulkan_runtime_if_available: no MoltenVK/Vulkan dylib found; skipping."
    return 0
  fi

  if [[ ! -f "${src}" ]]; then
    echo "install_macos_vulkan_runtime_if_available: dylib not found: ${src}" >&2
    return 2
  fi

  mkdir -p "${frameworks}"

  local base
  base="$(basename "${src}")"
  cp -f "${src}" "${frameworks}/${base}"
  chmod +x "${frameworks}/${base}"

  if [[ "${base}" == "libMoltenVK.dylib" ]]; then
    ln -sfn "libMoltenVK.dylib" "${frameworks}/libvulkan.1.dylib"
    ln -sfn "libvulkan.1.dylib" "${frameworks}/libvulkan.dylib"
  else
    # Keep a canonical libvulkan*.dylib name for loaders.
    cp -f "${src}" "${frameworks}/libvulkan.1.dylib"
    chmod +x "${frameworks}/libvulkan.1.dylib"
    ln -sfn "libvulkan.1.dylib" "${frameworks}/libvulkan.dylib"
  fi

  echo "Bundled Vulkan runtime from ${src} into ${frameworks}"
}

# Ad-hoc or developer signing so Gatekeeper accepts the bundle when launched from Finder.
# - Default identity "-" is ad-hoc (no Apple account; fine for same-machine use).
# - Set CODESIGN_IDENTITY="Apple Development: Your Name (TEAMID)" or
#   "Developer ID Application: ..." for stricter local testing.
# - LASTCHAOS_SIGN=0 skips signing.
# - LASTCHAOS_CLEAR_QUARANTINE=0 skips stripping com.apple.quarantine (on by default).
sign_macos_app_bundle() {
  local app_dir="$1"

  if [[ "${LASTCHAOS_SIGN:-1}" == "0" ]]; then
    echo "sign_macos_app_bundle: skipped (LASTCHAOS_SIGN=0)"
    return 0
  fi
  if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "sign_macos_app_bundle: not macOS, skipping codesign"
    return 0
  fi
  if ! command -v codesign >/dev/null 2>&1; then
    echo "sign_macos_app_bundle: codesign not in PATH, skipping" >&2
    return 0
  fi

  local identity="${CODESIGN_IDENTITY:--}"
  echo "sign_macos_app_bundle: codesign (identity: ${identity})"

  if [[ "${LASTCHAOS_CLEAR_QUARANTINE:-1}" != "0" ]]; then
    xattr -dr com.apple.quarantine "${app_dir}" 2>/dev/null || true
  fi

  local -a args=(--force --sign "${identity}" --deep)
  if [[ "${identity}" == "-" ]]; then
    args+=(--timestamp=none)
  fi

  if ! codesign "${args[@]}" "${app_dir}"; then
    echo "sign_macos_app_bundle: codesign failed" >&2
    return 1
  fi

  codesign --verify --verbose=2 "${app_dir}" 2>/dev/null || codesign --verify "${app_dir}"
}

# Print post-package signing/Gatekeeper diagnostics.
# - codesign verification is required on macOS and will fail this function if invalid.
# - spctl assessment is optional and reported when available.
verify_macos_app_bundle_security() {
  local app_dir="$1"

  if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "verify_macos_app_bundle_security: not macOS, skipping bundle security checks"
    return 0
  fi

  if ! command -v codesign >/dev/null 2>&1; then
    echo "verify_macos_app_bundle_security: codesign not in PATH, skipping" >&2
    return 0
  fi

  echo "verify_macos_app_bundle_security: codesign --verify --verbose=2"
  codesign --verify --verbose=2 "${app_dir}"

  echo "verify_macos_app_bundle_security: codesign --display --verbose=2"
  codesign --display --verbose=2 "${app_dir}" 2>&1 | sed 's/^/  /'

  if command -v spctl >/dev/null 2>&1; then
    echo "verify_macos_app_bundle_security: spctl --assess --type execute --verbose=4"
    if ! spctl --assess --type execute --verbose=4 "${app_dir}" 2>&1 | sed 's/^/  /'; then
      echo "verify_macos_app_bundle_security: spctl assess reported non-accept (informational)." >&2
    fi
  else
    echo "verify_macos_app_bundle_security: spctl not in PATH, skipping Gatekeeper assessment"
  fi
}
