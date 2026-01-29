# env/config

本目录用于集中管理 **QEMU 虚拟机（frontend）** 与 **DPDK vhost-user 后端（backend）**
在不同运行角色下的共享与角色专属配置，避免在脚本中硬编码参数。

## 配置分层说明

### 1. common.conf
通用配置，**frontend / backend 共享**，例如：

- 内存大小（MEM）
- CPU / lcore 范围（LCORES）
- NUMA / 内存通道数
- vhost-user 队列数（QUEUES）
- 文件前缀（FILE_PREFIX）

### 2. frontend.conf
仅供 **QEMU / VM 前端** 使用的配置，例如：

- VM 内存 / vCPU
- SSH 端口转发
- 管理网卡 MAC
- virtio-net 前端参数

### 3. backend.conf
仅供 **DPDK vhost-user 后端** 使用的配置，例如：

- vhost-user socket 路径
- DPDK EAL 参数
- backend dataplane 相关队列设置
- 规则文件路径（如 RULES_FILE）

### 4. parse.sh
配置解析与加载入口，提供统一接口：

```bash
load_config common
load_config frontend   # QEMU
load_config backend    # DPDK
```

脚本内部负责：

* 按角色加载对应 `.conf`
* 提供默认值
* 避免脚本中出现重复配置逻辑

## 设计原则

* **配置与启动逻辑分离**
* **frontend / backend 显式区分**
* **支持后续扩展多 VM / 多 backend 实例**
* **脚本保持最小职责，只负责 orchestration**

