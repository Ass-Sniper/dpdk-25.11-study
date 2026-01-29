#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

APP=${APP:-$ROOT_DIR/build/dpdk-tenant-qos-v2}
RULES=${RULES:-$ROOT_DIR/rules.csv}
PORTMASK=${PORTMASK:-0x1}

# vdev 设备：默认使用 vhost-user（net_vhost PMD）。如需物理口运行，可设置 VDEV_ARGS 为空。
VDEV_ARGS=${VDEV_ARGS:---vdev=net_vhost0,iface=/tmp/tenantqos-vhost0,queues=1,client=0}
if [[ ! -x "$APP" ]]; then
  echo "Binary not found: $APP" >&2
  exit 1
fi

sudo -E "$APP" -l 0-1 -n 4 \
  "$VDEV_ARGS" \
  -- --portmask "$PORTMASK" \
  --tenants 8 \
  --rules 4096 \
  --rules-file "$RULES" \
  --stats-interval 5
