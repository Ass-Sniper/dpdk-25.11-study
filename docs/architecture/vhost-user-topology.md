# vhost-user Topology & Control/Data Plane Architecture

本文档描述 `tenant-qos` 项目中基于 **QEMU + virtio-net + vhost-user + DPDK**
构建的前后端分离网络数据面架构，重点说明组件职责、拓扑关系以及
控制面 / 数据面的边界划分。

---

## 1. 架构目标

该拓扑设计的目标是：

- 将 **Guest VM（frontend）** 与 **高性能数据面（backend）** 解耦
- 使用 vhost-user 作为 **virtio 后端传输通道**
- 在 Host 上运行 DPDK，实现可控、可扩展的高性能 dataplane
- 为多租户 QoS / 流量整形提供清晰的插入点

---

## 2. 组件角色划分

### 2.1 Frontend（Guest VM）

- 运行在 QEMU/KVM 中
- 使用 `virtio-net-pci` 作为网络前端
- **不包含任何 DPDK 代码**
- 主要职责：
  - 运行控制面或租户逻辑
  - 通过标准 Linux 网络栈收发数据
  - 对 dataplane 实现保持透明

### 2.2 Backend（Host / DPDK）

- 运行在 Host 用户态
- 基于 DPDK + vhost-user PMD
- 直接操作 hugepage / DMA / polling
- 主要职责：
  - 接管 virtio-net 后端
  - 执行 tenant-qos 数据面逻辑
  - 实现高性能转发、限速、统计等功能

### 2.3 QEMU / vhost-user

- QEMU 负责：
  - VM 生命周期管理
  - virtio 设备建模
  - vhost-user 协议转发
- vhost-user socket：
  - 作为 **frontend ↔ backend 的 IPC 边界**
  - 由 backend（DPDK）创建
  - 由 QEMU 作为 client 连接

---

## 3. 拓扑结构（逻辑视图）

```text
+---------------------------+
|        Guest VM           |
|                           |
|  +---------------------+  |
|  | Linux Network Stack |  |
|  +----------+----------+  |
|             |             |
|      virtio-net (PCI)     |
+-------------+-------------+
              |
              | virtio descriptors
              |
      +-------v------------------+
      |        QEMU              |
      |  virtio-net + vhost-user |
      +-------+------------------+
              |
              | vhost-user socket
              | (UNIX domain)
              |
+-------------v------------------+
|        DPDK Backend            |
|                                |
|  net_vhost PMD                 |
|  tenant-qos dataplane          |
|                                |
+--------------------------------+
````

---

## 4. 控制面 vs 数据面

### 4.1 控制面（Control Plane）

控制面主要发生在：

* QEMU 启动阶段
* virtio 特性协商（features negotiation）
* 队列数量 / vring 地址配置
* vhost-user socket 建立与管理

特点：

* 低频
* 以 `ioctl / socket message` 为主
* 对性能不敏感

### 4.2 数据面（Data Plane）

数据面路径为：

```text
Guest skb
  → virtio descriptor
    → vhost-user vring
      → DPDK poll loop
        → tenant-qos processing
```

特点：

* 高频、持续运行
* 完全由 DPDK polling 驱动
* 无 VM-exit / 无中断（典型 fast path）
* 性能主要受限于：

  * cache locality
  * memory bandwidth
  * queue / core 绑定策略

---

## 5. vhost-user Socket 生命周期

1. **Backend（DPDK）启动**

   * 创建 vhost-user socket
   * 进入监听状态

2. **QEMU 启动**

   * 作为 client 连接 socket
   * 完成 virtio / vring 初始化

3. **运行阶段**

   * QEMU 不再参与数据面
   * DPDK 独占 dataplane

4. **退出 / 重启**

   * backend 负责清理 stale socket
   * frontend 可重复连接

---

## 6. 多队列与多核设计

* virtio-net 启用 `mq=on`
* vhost-user PMD 使用多队列
* 队列数（QUEUES）需保持 frontend / backend 一致
* 常见绑定策略：

  * 1 queue : 1 lcore
  * NUMA 感知的 lcore / memory 绑定

---

## 7. 为什么选择 vhost-user

与其他方案对比：

| 方案             | 特点     | 取舍               |
| -------------- | ------ | ---------------- |
| tap / bridge   | 简单     | 性能低              |
| vhost-kernel   | 内核参与   | 不利于自定义 dataplane |
| **vhost-user** | 用户态、灵活 | ⭐ 本项目选择          |

vhost-user 使 dataplane：

* 完全用户态
* 易于调试 / profiling
* 适合 tenant-qos 这类自定义转发逻辑

---

## 8. 可扩展方向

该拓扑可自然扩展为：

* 多 VM → 单 backend（多租户）
* 多 backend → 多 socket（分片 dataplane）
* backend 替换为：

  * AF_XDP
  * vDPA
  * SmartNIC / DPU offload

---

## 9. 总结

该 vhost-user 拓扑实现了：

* 清晰的前后端职责分离
* 高性能、可控的数据面
* 与 QEMU / virtio 标准兼容
* 面向未来 offload 架构的良好演进路径

适合作为 **DPDK + 虚拟化数据面实验与工程化原型** 的基础架构。

