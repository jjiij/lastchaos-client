#!/usr/bin/env bash
set -euo pipefail

TARGET_BIN="${1:-}"
if [[ -z "${TARGET_BIN}" ]]; then
  echo "Usage: $0 /path/to/nksp_probe" >&2
  exit 1
fi
if [[ ! -f "${TARGET_BIN}" ]]; then
  echo "classify_nksp_unresolved_symbols: binary not found: ${TARGET_BIN}" >&2
  exit 2
fi

if ! command -v nm >/dev/null 2>&1; then
  echo "classify_nksp_unresolved_symbols: nm not found in PATH" >&2
  exit 3
fi

OUT_DIR="$(dirname "${TARGET_BIN}")"
OUT_TXT="${OUT_DIR}/nksp_unresolved_symbols.txt"
OUT_BY_SUBSYSTEM="${OUT_DIR}/nksp_unresolved_symbols_by_subsystem.txt"
OUT_SUMMARY="${OUT_DIR}/nksp_unresolved_symbols_subsystem_summary.txt"

nm -u "${TARGET_BIN}" | sed '/^$/d' > "${OUT_TXT}" || true

awk '
function classify(sym) {
  if (sym ~ /(Vulkan|vk[A-Z]|MoltenVK|Metal|Render|Shader|Texture|D3D|d3d|GL)/) return "rendering"
  if (sym ~ /(IPC|ipc|SharedMemory|Ext_ipc|ToolHelp|CreateProcess|ShellExecute|Dialog|MessageBox|Process)/) return "ipc_process_launcher"
  if (sym ~ /(input|Input|Mouse|Keyboard|Controller|Joystick|Window|Wnd|CWnd)/) return "input_windowing"
  if (sym ~ /(File|Path|Stream|Directory|Asset|Data|sl\\.dta|pack|archive|resource)/) return "filesystem_assets"
  if (sym ~ /(Engine|Game|Client|Nksp|Entity|Character|World|__bClientApp|TitleData|EntitiesMP)/) return "engine_gameplay"
  return "uncategorized"
}
{
  sym=$NF
  cat=classify(sym)
  count[cat]++
  total++
  print cat "\t" sym
}
END{
  printf("Total unresolved symbols: %d\n", total+0)
  for (cat in count)
    printf("  %s: %d\n", cat, count[cat]+0)
}
' "${OUT_TXT}" | sort > "${OUT_BY_SUBSYSTEM}"

awk '
{
  if ($1 == "Total") next
  if ($1 == "") next
  if ($1 ~ /^uncategorized|^rendering|^ipc_process_launcher|^input_windowing|^filesystem_assets|^engine_gameplay/)
    count[$1]++
}
END{
  total=0
  for (cat in count) total += count[cat]
  printf("Total classified unresolved symbols: %d\n", total+0)
  printf("  engine_gameplay: %d\n", count["engine_gameplay"]+0)
  printf("  rendering: %d\n", count["rendering"]+0)
  printf("  ipc_process_launcher: %d\n", count["ipc_process_launcher"]+0)
  printf("  input_windowing: %d\n", count["input_windowing"]+0)
  printf("  filesystem_assets: %d\n", count["filesystem_assets"]+0)
  printf("  uncategorized: %d\n", count["uncategorized"]+0)
}
' "${OUT_BY_SUBSYSTEM}" > "${OUT_SUMMARY}"

echo "Wrote subsystem classification:"
echo "  ${OUT_BY_SUBSYSTEM}"
echo "  ${OUT_SUMMARY}"
