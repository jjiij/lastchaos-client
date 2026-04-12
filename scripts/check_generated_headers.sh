#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Full packaging should use real generated headers rather than temporary stubs.
# Set LASTCHAOS_ALLOW_PLACEHOLDER_HEADERS=1 to bypass this check for bring-up/debug.
ALLOW_PLACEHOLDERS="${LASTCHAOS_ALLOW_PLACEHOLDER_HEADERS:-0}"

if [[ "${ALLOW_PLACEHOLDERS}" == "1" ]]; then
  echo "check_generated_headers: skipping placeholder-header check (LASTCHAOS_ALLOW_PLACEHOLDER_HEADERS=1)"
  exit 0
fi

declare -a scan_dirs=(
  "${ROOT_DIR}/source/EntitiesMP"
)

declare -a marker_patterns=(
  "Temporary portability stub"
  "Auto-generated temporary compatibility stub"
  "Replace with real generated header"
)

missing=0
for required in "${ROOT_DIR}/source/EntitiesMP/Global.h" "${ROOT_DIR}/source/EntitiesMP/Player.h"; do
  if [[ ! -f "${required}" ]]; then
    echo "check_generated_headers: missing required generated header: ${required}" >&2
    missing=1
  fi
done
if [[ "${missing}" -ne 0 ]]; then
  exit 2
fi

found_markers=0
for dir in "${scan_dirs[@]}"; do
  [[ -d "${dir}" ]] || continue
  for marker in "${marker_patterns[@]}"; do
    while IFS= read -r line; do
      [[ -n "${line}" ]] || continue
      echo "check_generated_headers: placeholder marker found: ${line}" >&2
      found_markers=1
    done < <(rg -n --glob '*.h' --fixed-strings "${marker}" "${dir}" || true)
  done
done

if [[ "${found_markers}" -ne 0 ]]; then
  cat >&2 <<'EOF'
check_generated_headers: placeholder/generated-stub headers detected.
Packaging aborted because full macOS bundles should use real generated headers.
If you are intentionally doing compile-only bring-up, set:
  LASTCHAOS_ALLOW_PLACEHOLDER_HEADERS=1
EOF
  exit 3
fi

echo "check_generated_headers: OK (no placeholder markers detected)"
