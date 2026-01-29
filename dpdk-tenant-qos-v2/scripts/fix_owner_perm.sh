#!/usr/bin/env bash
set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <dir> [dir ...]"
    exit 1
fi

USER_NAME="${SUDO_USER:-$USER}"
GROUP_NAME="$USER_NAME"

for DIR in "$@"; do
    if [ ! -d "$DIR" ]; then
        echo "WARN: '$DIR' not found, skip"
        continue
    fi

    echo "== Fixing: $DIR =="
    echo "  owner : ${USER_NAME}:${GROUP_NAME}"

    sudo chown -R "${USER_NAME}:${GROUP_NAME}" "$DIR"

    # 只保证 user 可写，不碰 group/other
    chmod u+rwx "$DIR"

    # 明确移除 world-writable（DPDK 安全关键）
    chmod o-w "$DIR"

    ls -ld "$DIR"
    echo
done

echo "All done."

