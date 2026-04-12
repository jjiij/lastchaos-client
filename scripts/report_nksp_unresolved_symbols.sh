#!/usr/bin/env bash
set -euo pipefail

TARGET_BIN="${1:-}"
if [[ -z "${TARGET_BIN}" ]]; then
  echo "Usage: $0 /path/to/nksp_probe" >&2
  exit 1
fi
if [[ ! -f "${TARGET_BIN}" ]]; then
  echo "report_nksp_unresolved_symbols: binary not found: ${TARGET_BIN}" >&2
  exit 2
fi

if ! command -v nm >/dev/null 2>&1; then
  echo "report_nksp_unresolved_symbols: nm not found in PATH" >&2
  exit 3
fi

OUT_DIR="$(dirname "${TARGET_BIN}")"
OUT_TXT="${OUT_DIR}/nksp_unresolved_symbols.txt"
OUT_SUMMARY="${OUT_DIR}/nksp_unresolved_symbols_summary.txt"

if [[ "$(uname -s)" == "Darwin" ]]; then
  nm -u "${TARGET_BIN}" | sed '/^$/d' > "${OUT_TXT}" || true
else
  nm -u "${TARGET_BIN}" | sed '/^$/d' > "${OUT_TXT}" || true
fi

awk '
{
  sym=$NF
  if (sym ~ /^__Z/ || sym ~ /^_Z/) cat="cxx_mangled"
  else if (sym ~ /^__[A-Za-z0-9_]+/ || sym ~ /^_[A-Za-z0-9_]+/) cat="c_symbol"
  else cat="other"
  count[cat]++
  total++
}
END{
  printf("Total unresolved symbols: %d\n", total+0)
  printf("  cxx_mangled: %d\n", count["cxx_mangled"]+0)
  printf("  c_symbol: %d\n", count["c_symbol"]+0)
  printf("  other: %d\n", count["other"]+0)
}
' "${OUT_TXT}" > "${OUT_SUMMARY}"

echo "Wrote unresolved-symbol report:"
echo "  ${OUT_TXT}"
echo "  ${OUT_SUMMARY}"
