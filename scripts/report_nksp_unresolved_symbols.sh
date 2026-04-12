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
OUT_SUBSYSTEM="${OUT_DIR}/nksp_unresolved_symbols_by_subsystem.txt"

nm -u "${TARGET_BIN}" | sed '/^$/d' > "${OUT_TXT}" || true

if command -v c++filt >/dev/null 2>&1; then
  DEMANGLE_TOOL="c++filt"
else
  DEMANGLE_TOOL=""
fi

if [[ -n "${DEMANGLE_TOOL}" ]]; then
  awk '{print $NF}' "${OUT_TXT}" | c++filt > "${OUT_TXT}.demangled"
else
  awk '{print $NF}' "${OUT_TXT}" > "${OUT_TXT}.demangled"
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

awk '
function classify(sym) {
  l=tolower(sym)
  if (l ~ /ipc|sharedmemory|xextipcevent|queue/) return "ipc_process"
  if (l ~ /vulkan|render|gfx|d3d|gl|moltenvk/) return "rendering"
  if (l ~ /launcher|cmdline|winmain|submain|updatenewlauncher/) return "launcher_entry"
  if (l ~ /entity|world|game|engine|clientapp|_pnetwork|_pgame/) return "engine_gameplay"
  return "other"
}
{
  subsystem=classify($0)
  count[subsystem]++
  total++
}
END{
  printf("Total unresolved symbols (demangled): %d\n", total+0)
  printf("  engine_gameplay: %d\n", count["engine_gameplay"]+0)
  printf("  rendering: %d\n", count["rendering"]+0)
  printf("  ipc_process: %d\n", count["ipc_process"]+0)
  printf("  launcher_entry: %d\n", count["launcher_entry"]+0)
  printf("  other: %d\n", count["other"]+0)
}
' "${OUT_TXT}.demangled" > "${OUT_SUBSYSTEM}"

echo "Wrote unresolved-symbol report:"
echo "  ${OUT_TXT}"
echo "  ${OUT_SUMMARY}"
echo "  ${OUT_SUBSYSTEM}"
