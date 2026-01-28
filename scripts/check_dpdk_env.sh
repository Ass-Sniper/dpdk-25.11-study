#!/usr/bin/env bash
set -e


TARGET_PATH="$(pwd)"
HUGEPAGES_PATH="/dev/hugepages"


red() { echo -e "\033[31m$*\033[0m"; }
green() { echo -e "\033[32m$*\033[0m"; }
yellow(){ echo -e "\033[33m$*\033[0m"; }


check_permissions() {
echo "[1] Checking directory permissions..."
if find "$TARGET_PATH" -type d -perm -0002 | grep -q .; then
red "World-writable directories found:"
find "$TARGET_PATH" -type d -perm -0002
exit 1
fi
green "Directory permissions OK"
}


check_hugepages() {
echo "[2] Checking hugepages..."
grep -i huge /proc/meminfo
}


check_mount() {
echo "[3] Checking hugetlbfs mount..."
if mount | grep -q "$HUGEPAGES_PATH"; then
green "hugetlbfs mounted at $HUGEPAGES_PATH"
else
red "hugetlbfs not mounted"
exit 1
fi
}


check_permissions
check_hugepages
check_mount


green "DPDK environment basic checks passed"

