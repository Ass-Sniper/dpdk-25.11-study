#!/usr/bin/env bash
set -euo pipefail

# =============================================================================
# tenant-qos minimal dataplane (vhost-user backend)
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT}"

# ===== load shared config =====
source "${ROOT}/../../env/config/parse.sh"
load_config backend

# ===== workspace env =====
ENV_SH="${ROOT}/../../../scripts/env.sh"
echo "Loading env from ${ENV_SH}"
source "${ENV_SH}"

BIN="${ROOT}/build/tenant-qos"

EAL_ARGS=(
  -l "${LCORES}"
  -n "${MEM_CHANNELS}"
  --file-prefix "${FILE_PREFIX}"
  --proc-type auto
)

VDEV_ARGS=(
  --vdev=net_vhost0,iface="${VHOST_SOCK}",queues="${QUEUES}",client=0
)

APP_ARGS=(
  # --rules "${RULES_FILE}"
)

# ===== vhost socket =====
if [[ -S "${VHOST_SOCK}" ]]; then
  echo "[WARN] Removing stale vhost-user socket: ${VHOST_SOCK}"
  rm -f "${VHOST_SOCK}"
fi

echo "== RUN tenant-qos (DPDK backend) =="
echo "${BIN} ${EAL_ARGS[*]} ${VDEV_ARGS[*]} -- ${APP_ARGS[*]}"

exec "${BIN}" \
  "${EAL_ARGS[@]}" \
  "${VDEV_ARGS[@]}" \
  -- "${APP_ARGS[@]}"

