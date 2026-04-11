#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLASSES_DIR="${ROOT_DIR}/source/Engine/Classes"

if [[ ! -d "${CLASSES_DIR}" ]]; then
  echo "Classes dir not found: ${CLASSES_DIR}" >&2
  exit 1
fi

for es in "${CLASSES_DIR}"/*.es; do
  [[ -f "${es}" ]] || continue
  base="$(basename "${es}" .es)"
  hdr="${CLASSES_DIR}/${base}.h"

  # Keep hand-maintained compatibility stubs.
  if [[ -f "${hdr}" ]]; then
    continue
  fi

  upper_base="$(printf '%s' "${base}" | tr '[:lower:]' '[:upper:]')"
  guard="SE_INCL_ENGINE_CLASSES_${upper_base}_H"

  cat > "${hdr}" <<EOF
#ifndef ${guard}
#define ${guard}

// Auto-generated temporary compatibility stub from ${base}.es
// Replace with real generated header from entity-class compiler pipeline.

class ${base} {
public:
  virtual ~${base}() {}
};

#endif
EOF

done

echo "Generated missing .es header stubs in: ${CLASSES_DIR}"
