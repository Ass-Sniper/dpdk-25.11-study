#!/usr/bin/env bash
# =============================================================================
# pktgen → virtio-user → vhost-user → tenant-qos
#
# Verified with:
#   - Pktgen 25.08.2
#   - DPDK   25.11.0
#
# Topology:
#   pktgen (virtio-user frontend)
#        |
#        |  vhost-user socket
#        v
#   tenant-qos (DPDK backend)
# =============================================================================

set -euo pipefail

# -----------------------------------------------------------------------------
# paths
# -----------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# pktgen binary (adjust if needed)
PKTGEN_BIN="$(command -v pktgen)"

if [[ -z "${PKTGEN_BIN}" ]]; then
  echo "pktgen not found in PATH"
  exit 1
fi

# -----------------------------------------------------------------------------
# runtime config (keep aligned with tenant-qos backend)
# -----------------------------------------------------------------------------

# CPU
LCORES="0-1"
MAIN_LCORE=1

# Memory
MEM_CHANNELS=4

# vhost / virtio
VHOST_SOCK="/tmp/tenantqos-vhost0"
QUEUES=1

# MAC for pktgen virtio-user device
PKTGEN_MAC="52:54:00:aa:bb:02"

# DPDK
FILE_PREFIX="pktgen"

# -----------------------------------------------------------------------------
# sanity checks
# -----------------------------------------------------------------------------
if [[ ! -S "${VHOST_SOCK}" ]]; then
  echo "vhost-user socket not found: ${VHOST_SOCK}"
  echo "Did you start tenant-qos backend?"
  exit 1
fi

# -----------------------------------------------------------------------------
# run pktgen
# -----------------------------------------------------------------------------
echo "== RUN pktgen (virtio-user frontend) =="
echo "  pktgen bin : ${PKTGEN_BIN}"
echo "  lcores     : ${LCORES} (main=${MAIN_LCORE})"
echo "  vhost sock : ${VHOST_SOCK}"
echo

exec "${PKTGEN_BIN}" \
  -l "${LCORES}" \
  --main-lcore "${MAIN_LCORE}" \
  -n "${MEM_CHANNELS}" \
  --proc-type auto \
  --file-prefix "${FILE_PREFIX}" \
  --single-file-segments \
  --vdev="net_virtio_user0,path=${VHOST_SOCK},queues=${QUEUES},server=0,mac=${PKTGEN_MAC}" \
  -- \
  -P \
  -m "[0:0].0"

