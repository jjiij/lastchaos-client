#!/usr/bin/env bash
set -euo pipefail

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew not found. Install Homebrew first." >&2
  exit 1
fi

brew update
brew install cmake go boost sdl2 pkg-config

cat <<EOF
Dependency bootstrap complete.
Installed/ensured: cmake, go, boost, sdl2, pkg-config
EOF
