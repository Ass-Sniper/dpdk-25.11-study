#!/usr/bin/env bash
set -euo pipefail

# =============================================================================
# QEMU + KVM + vhost-user (tenant-qos dataplane)
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ===== load shared config =====
source "${SCRIPT_DIR}/../config/parse.sh"
load_config frontend

# ===== paths =====
QEMU_BIN="$HOME/opt/qemu-6.2/bin/qemu-system-x86_64"
IMAGE="$HOME/vm-images/ubuntu20-kvm.qcow2"

# ===== sanity checks =====
[ -x "$QEMU_BIN" ] || { echo "QEMU not found: $QEMU_BIN"; exit 1; }
[ -f "$IMAGE" ] || { echo "Disk not found: $IMAGE"; exit 1; }
[ -e /dev/kvm ] || { echo "/dev/kvm not available"; exit 1; }
[ -S "$VHOST_SOCK" ] || {
  echo "vhost-user socket not found: $VHOST_SOCK"
  echo "Start DPDK tenant-qos first."
  exit 1
}

echo "== QEMU vhost-user + DPDK (tenant-qos) =="
echo "IMAGE        : $IMAGE"
echo "MEM          : ${MEM}MB"
echo "SMP          : $SMP"
echo "SSH forward  : 127.0.0.1:${SSH_FWD_PORT} -> VM:22"
echo "VHOST socket : $VHOST_SOCK"
echo "Queues       : $QUEUES"
echo

exec "$QEMU_BIN" \
  -enable-kvm \
  -cpu host \
  \
  -object memory-backend-memfd,id=mem,size=${MEM}M,share=on \
  -machine q35,accel=kvm,memory-backend=mem \
  \
  -m "$MEM" \
  -smp "$SMP" \
  \
  -drive "file=$IMAGE,if=virtio,format=qcow2" \
  \
  -netdev "user,id=mgmt,hostfwd=tcp:127.0.0.1:${SSH_FWD_PORT}-:22" \
  -device "virtio-net-pci,netdev=mgmt,mac=${MGMT_MAC}" \
  \
  -chardev "socket,id=char0,path=${VHOST_SOCK}" \
  -netdev "vhost-user,id=net0,chardev=char0,queues=${QUEUES}" \
  -device "virtio-net-pci,netdev=net0,mac=${DATA_MAC},mq=on,vectors=$((QUEUES*2+2))" \
  \
  -device VGA \
  -display vnc=127.0.0.1:1  

