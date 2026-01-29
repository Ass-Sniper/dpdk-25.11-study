#!/usr/bin/env bash
set -euo pipefail

# run_min.sh 所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# dpdk-tenant-qos-v2 根目录
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT}"

# workspace 级 env.sh
ENV_SH="${ROOT}/../../scripts/env.sh"
echo "Loading env from ${ENV_SH}"
source "${ENV_SH}"

BIN="${ROOT}/build/dpdk-tenant-qos-v2"

EAL_ARGS=(
  -l 0-1
  -n 4
  --file-prefix tenantqos
  --proc-type auto
)

VDEV_ARGS=(
  --vdev=net_vhost0,iface=/tmp/tenantqos-vhost0,queues=1,client=0
)

APP_ARGS=(
  # --rules "${ROOT}/rules.csv"
)

SOCK="/tmp/tenantqos-vhost0"
if [[ -S "${SOCK}" ]]; then
  echo "[WARN] Removing stale vhost-user socket: ${SOCK}"
  rm -f "${SOCK}"
fi

echo "== RUN =="
echo "${BIN} ${EAL_ARGS[*]} ${VDEV_ARGS[*]} -- ${APP_ARGS[*]}"

exec "${BIN}" "${EAL_ARGS[@]}" "${VDEV_ARGS[@]}" -- "${APP_ARGS[@]}"

