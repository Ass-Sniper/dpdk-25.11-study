#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
RULES=${RULES:-4096}
TENANTS=${TENANTS:-16}
OUTPUT=${OUTPUT:-$ROOT_DIR/rules.csv}

python3 "$ROOT_DIR/scripts/gen_rules.py" --rules "$RULES" --tenants "$TENANTS" --output "$OUTPUT"

echo "Generated rules: $OUTPUT"

cat <<'NOTE'
Suggested perf workflow:
  1) Build dpdk-tenant-qos-v2 with DPDK 25.11
  2) Run scripts/run.sh with appropriate EAL flags
  3) Use a traffic generator (pktgen, moongen, trex) to push flows
  4) Capture throughput, drops, and TopK stats output
NOTE
