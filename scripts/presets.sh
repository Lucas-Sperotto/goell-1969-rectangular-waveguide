#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

FIGURE="${1:-fig16}"
REBUILD="${2:-}"

if [ "$FIGURE" = "--rebuild" ] || [ "$REBUILD" = "--rebuild" ]; then
  "$REPO_ROOT/run.sh" build
  if [ "$FIGURE" = "--rebuild" ]; then
    FIGURE="fig16"
  fi
fi

exec "$REPO_ROOT/run.sh" "$FIGURE"
