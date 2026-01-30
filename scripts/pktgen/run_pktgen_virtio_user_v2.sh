#!/usr/bin/env bash
set -euo pipefail

# =============================================================================
# Pktgen (virtio_user frontend) -> tenant-qos (vhost-user backend) test runner
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"   # dpdk-25.11-study
cd "${ROOT}"

# ----- load shared config -----
source "${ROOT}/env/config/parse.sh"
load_config backend   # provides LCORES/MEM_CHANNELS/RULES_FILE (backend.conf)
load_config frontend  # provides DATA_MAC (frontend.conf)
# common.conf provides VHOST_SOCK/QUEUES/MEM/SMP/FILE_PREFIX

# ----- workspace env (dpdk/pktgen paths, LD_LIBRARY_PATH, etc.) -----
ENV_SH="${ROOT}/../scripts/env.sh"
echo "Loading env from ${ENV_SH}"
source "${ENV_SH}"

# ----- pktgen binary -----
PKTGEN_BIN="${PKTGEN_BIN:-pktgen}"  # allow override
command -v "${PKTGEN_BIN}" >/dev/null 2>&1 || {
  echo "[ERR] pktgen not found in PATH. Try: export PATH=\$PKTGEN/bin:\$PATH"
  exit 1
}

# ----- cpu layout (2 lcores minimal) -----
# If you only have 0-1, use master=0, port threads on 1
PKTGEN_LCORES="${PKTGEN_LCORES:-0-1}"
PKTGEN_MAP="${PKTGEN_MAP:-[1:1].0}"   # rx:tx on lcore1 for port0

# ----- EAL args -----
EAL_ARGS=(
  -l "${PKTGEN_LCORES}"
  -n "${MEM_CHANNELS}"
  --proc-type auto
  --file-prefix "pktgen"
  --single-file-segments
)

# ----- virtio_user devargs -----
# Use packed_vq=0 if your backend negotiates split ring only;
# If you later enable packed, set packed_vq=1 on BOTH sides consistently.
VDEV_ARGS=(
  --vdev="net_virtio_user0,path=${VHOST_SOCK},queues=${QUEUES},packed_vq=0,mac=${DATA_MAC}"
)

# ----- pktgen app args -----
APP_ARGS=(
  -P
  -m "${PKTGEN_MAP}"
)

echo "== RUN pktgen (virtio_user -> vhost-user socket) =="
echo "${PKTGEN_BIN} ${EAL_ARGS[*]} ${VDEV_ARGS[*]} -- ${APP_ARGS[*]}"

exec "${PKTGEN_BIN}" \
  "${EAL_ARGS[@]}" \
  "${VDEV_ARGS[@]}" \
  -- \
  "${APP_ARGS[@]}"

