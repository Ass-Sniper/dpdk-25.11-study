#!/usr/bin/env bash
set -euo pipefail

# =============================================================================
# QEMU + KVM + vhost-user (tenant-qos dataplane)
#
# VM role:
#   - Control / Tenant
#   - virtio front-end only
#
# Dataplane:
#   - vhost-user
#   - DPDK runs on host
# =============================================================================

# ===== paths =====
QEMU_BIN="$HOME/opt/qemu-6.2/bin/qemu-system-x86_64"
IMAGE="$HOME/vm-images/ubuntu20-kvm.qcow2"

# ===== resources =====
MEM=2048
SMP=2

# ===== networking =====
# Management (SSH)
SSH_FWD_PORT=2222
MGMT_MAC="52:54:00:00:00:02"

# Dataplane (vhost-user)
VHOST_SOCK="/tmp/tenantqos-vhost0"
DATA_MAC="52:54:00:aa:bb:01"
QUEUES=2   # must match DPDK vhost-user queues

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
  -m "$MEM" \
  -smp "$SMP" \
  \
  -machine q35,accel=kvm \
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
  -nographic \
  -serial mon:stdio

