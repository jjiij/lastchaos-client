#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LEGACY_WORKFLOW="${ROOT_DIR}/.github/workflows/porting-matrix.yml"

if [[ -f "${LEGACY_WORKFLOW}" ]]; then
  echo "Legacy workflow must not exist: ${LEGACY_WORKFLOW}" >&2
  exit 1
fi

echo "OK: legacy workflow is absent (${LEGACY_WORKFLOW})"
