#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

cat <<'BANNER'
Launching dpdk-tenant-qos (stub runner)
BANNER

if [[ ! -x "$ROOT_DIR/build/dpdk-tenant-qos" ]]; then
  echo "Binary not found at $ROOT_DIR/build/dpdk-tenant-qos" >&2
  echo "Hint: build the sample with your DPDK toolchain" >&2
  exit 1
fi

"$ROOT_DIR/build/dpdk-tenant-qos" --tenants 4 --tsc-hz 1000000000
