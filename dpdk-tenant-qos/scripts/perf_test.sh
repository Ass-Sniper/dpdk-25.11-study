#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
RULES=${RULES:-128}
TENANTS=${TENANTS:-8}

python3 "$ROOT_DIR/scripts/gen_rules.py" --rules "$RULES" --tenants "$TENANTS" > "$ROOT_DIR/build/rules.csv"

echo "Generated rules: $ROOT_DIR/build/rules.csv"

echo "Run your traffic generator against dpdk-tenant-qos and capture perf metrics."
