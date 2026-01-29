#!/usr/bin/env bash
set -euo pipefail

CONFIG_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

load_config() {
  local role="$1"
  source "${CONFIG_DIR}/common.conf"
  source "${CONFIG_DIR}/${role}.conf"
}

