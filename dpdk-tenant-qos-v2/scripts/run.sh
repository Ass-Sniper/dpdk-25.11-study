#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

APP=${APP:-$ROOT_DIR/build/dpdk-tenant-qos-v2}
RULES=${RULES:-$ROOT_DIR/rules.csv}
PORTMASK=${PORTMASK:-0x1}

if [[ ! -x "$APP" ]]; then
  echo "Binary not found: $APP" >&2
  exit 1
fi

sudo -E "$APP" -l 0-1 -n 4 \
  -- --portmask "$PORTMASK" \
  --tenants 8 \
  --rules 4096 \
  --rules-file "$RULES" \
  --stats-interval 5
