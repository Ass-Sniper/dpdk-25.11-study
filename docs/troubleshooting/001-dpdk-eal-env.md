# DPDK 运行环境故障排查与修复指南（EAL 初始化常见问题）

> 归档路径建议：`docs/troubleshooting/dpdk-eal-env.md`

本文件用于**长期归档** DPDK 在 Linux 环境下运行时最常见、最容易踩坑的两类问题：

* **目录权限安全校验失败（world-writable）**
* **Hugepages / hugetlbfs 配置缺失**

目标是做到：**一次排查，长期复用；换机器、换用户、换环境也能快速恢复。**

---

## 1. 目录安全权限问题（World-writable and Insecure）

### 1.1 现象描述

运行 DPDK 程序（示例、pktgen、自研 app）时报错：

```
EAL: Error, directory path [...] is world-writable and insecure
EAL: Cannot init plugins
EAL init failed
```

### 1.2 原因分析

DPDK 的 EAL（Environment Abstraction Layer）在初始化阶段会：

* 递归检查 **driver / plugin 路径** 及其 **父目录链**
* 若任意目录对 *other* 用户可写（如 `777`、`o+w`）
* 认为存在 **恶意插件注入风险**（shared library injection）
* **直接拒绝加载并终止初始化**

这一步在 **DPDK 新版本** 中尤为严格，是很多“能编译但跑不起来”的根因。

### 1.3 修复方案

核心原则：**移除 other 的写权限（`o-w`）**，一般修复为 `755` 即可。

```bash
# 示例：对报错路径及其父目录逐级修复
chmod o-w /home/kay/codebase
chmod o-w /home/kay/codebase/test
chmod o-w /home/kay/codebase/test/dpdk
```

#### 辅助排查命令（强烈建议）

```bash
# 查看完整父目录链权限
namei -l /home/kay/codebase

# 查找 home 目录下仍然 world-writable 的目录
find /home/kay -type d -perm -0002 -print
```

> 经验法则：**DPDK 报错的路径，99% 不是“那一级目录”的问题，而是“上游某一级”。**

---

## 2. Hugepages 大页内存配置问题

### 2.1 现象描述

目录权限修复后，继续报错：

```
EAL: No free 2048 kB hugepages reported on node 0
EAL: Cannot get hugepage information.
```

### 2.2 原因分析

DPDK 默认依赖 Hugepages：

* 减少 TLB miss
* 提供大块、连续、可控的物理内存
* 支撑 mempool / mbuf / ring / memzone 等核心对象

若系统：

* 未预留 hugepages
* 或未挂载 `hugetlbfs`

EAL 将无法完成内存子系统初始化。

---

### 2.3 修复步骤

#### Step 1：预留 Hugepages（动态方式）

示例：分配 **512 × 2MB = 1GB**

```bash
sudo sysctl -w vm.nr_hugepages=512
```

验证：

```bash
grep -i huge /proc/meminfo
```

关注字段：

* `HugePages_Total`
* `HugePages_Free`
* `Hugepagesize`

---

#### Step 2：挂载 hugetlbfs

```bash
sudo mkdir -p /dev/hugepages
sudo mount -t hugetlbfs nodev /dev/hugepages
```

验证：

```bash
mount | grep -i huge
```

---

#### Step 3：允许非 root 用户运行（推荐方式）

**不推荐** 直接 `chmod 777`，更推荐用用户组隔离。

```bash
# 创建用户组（若已存在可忽略）
sudo groupadd -f hugetlb

# 将当前用户加入组
sudo usermod -aG hugetlb kay

# 设置挂载点权限
sudo chown root:hugetlb /dev/hugepages
sudo chmod 770 /dev/hugepages
```

> ⚠️ 重新登录 shell 后权限才会生效。

---

## 3. 成功验证标志

以下日志基本可判定 **EAL 初始化成功**：

```
EAL: Selected IOVA mode 'VA'
EAL: VFIO support initialized
DPDK EAL init OK
```

建议在应用代码中额外自检：

* `rte_eal_init()` 返回值
* `rte_lcore_count()`
* `rte_socket_id()`
* 创建一个最小 `rte_pktmbuf_pool_create()`

---

## 4. 持久化配置（可选，但强烈建议）

### 4.1 持久化 Hugepages 数量

`/etc/sysctl.conf`

```conf
vm.nr_hugepages=512
```

```bash
sudo sysctl -p
```

---

### 4.2 持久化 hugetlbfs 挂载

`/etc/fstab`

```fstab
nodev  /dev/hugepages  hugetlbfs  defaults  0  0
```

---

## 5. 自动化检查脚本（强烈推荐）

> 建议保存为：`scripts/check_dpdk_env.sh`

```bash
#!/usr/bin/env bash
set -e

TARGET_PATH="$(pwd)"
HUGEPAGES_PATH="/dev/hugepages"

red()   { echo -e "\033[31m$*\033[0m"; }
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
```

---

## 6. 下一步建议（进入真正的 DPDK 开发）

1. 创建最小 mbuf pool（`rte_pktmbuf_pool_create`）
2. 使用 `net_tap` / `net_null` 做最小收发闭环
3. 再接入真实 NIC（VFIO / IOMMU / 驱动）
4. 将本文件作为 **新环境 bring-up checklist** 固化使用

---

> **结论一句话版**：
>
> DPDK 跑不起来，80% 是环境问题；环境问题里，80% 又是 **权限 + Hugepages**。这份文档就是为这 64% 的问题准备的。

