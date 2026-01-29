#!/usr/bin/env bash
set -euo pipefail

# ===== paths =====
QEMU_BIN="$HOME/opt/qemu-6.2/bin/qemu-system-x86_64"
ISO="$HOME/Downloads/ubuntu-20.04.6-live-server-amd64.iso"
IMAGE="$HOME/vm-images/ubuntu20-kvm.qcow2"

# ===== resources =====
MEM=2048
SMP=2

# ===== sanity checks =====
[ -x "$QEMU_BIN" ] || { echo "QEMU not found: $QEMU_BIN"; exit 1; }
[ -f "$ISO" ] || { echo "ISO not found: $ISO"; exit 1; }
[ -f "$IMAGE" ] || { echo "Disk not found: $IMAGE"; exit 1; }
[ -e /dev/kvm ] || { echo "/dev/kvm not available"; exit 1; }

echo "== Ubuntu 20.04 installer (VGA + VNC) =="
echo "QEMU : $QEMU_BIN"
echo "ISO  : $ISO"
echo "DISK : $IMAGE"
echo "MEM  : ${MEM}MB"
echo "SMP  : $SMP"
echo "VNC  : 127.0.0.1:1 (tcp/5901)"
echo

exec "$QEMU_BIN" \
  -enable-kvm \
  -cpu host \
  -m "$MEM" \
  -smp "$SMP" \
  \
  -machine q35,accel=kvm \
  \
  -cdrom "$ISO" \
  -drive "file=$IMAGE,if=virtio,format=qcow2" \
  \
  -netdev "user,id=mgmt,hostfwd=tcp:127.0.0.1:2222-:22" \
  -device "virtio-net-pci,netdev=mgmt" \
  \
  -device VGA \
  -display "vnc=127.0.0.1:1" \
  \
  -monitor stdio

