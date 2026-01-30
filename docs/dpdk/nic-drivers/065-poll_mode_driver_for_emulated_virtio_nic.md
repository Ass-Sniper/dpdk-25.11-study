
# 65. 用于模拟 Virtio 网卡的轮询模式驱动（Poll Mode Driver）

Virtio 是一个由 IBM 发起、并由 KVM 虚拟化管理程序支持的**半虚拟化（para-virtualization）框架**。  
在数据平面开发套件（DPDK）中，我们提供了 **virtio 轮询模式驱动（PMD）** 作为一种软件解决方案，用于实现 **虚拟机到虚拟机（guest VM ↔ guest VM）** 以及 **虚拟机到宿主机（guest VM ↔ host）** 的高速通信，相比之下，SR-IOV 是一种硬件解决方案。

Vhost 是一个用于 virtio 的 **qemu 后端内核加速模块**。

关于基础的 qemu-KVM 安装，以及在虚拟机中使用其他 Intel EM 轮询模式驱动的内容，请参考章节 **“Driver for VM Emulated Devices”**。

在本章中，我们将演示 **virtio PMD** 与两种后端的使用方式，其中包括 **标准的 qemu vhost 后端**。

---

## 65.1. DPDK 中的 Virtio 实现

关于 virtio 规范的详细信息，请参考最新的 **VIRTIO（Virtual I/O）设备规范**。

作为一个 PMD，virtio 提供了用于 **报文接收（Rx）和发送（Tx）** 的回调函数。

在 **接收（Rx）** 路径中，由 vring 中 *used descriptors* 描述的报文对 virtio 可用，并可被成批（burst）取出。

在 **发送（Tx）** 路径中，由 vring 中 *used descriptors* 描述的报文对 virtio 可用并可被清理。  
Virtio 会将待发送的报文入队到 vring 中，使其对设备可见，并在必要时通知宿主机后端。

---

## 65.2. Virtio PMD 的特性与限制

在当前版本中，virtio PMD 提供了 **基础的报文接收和发送功能**。

在接收报文时，它支持 **每个报文使用可合并（merge-able）的缓冲区**；在发送报文时，支持 **每个报文使用分散缓冲区（scattered buffer）**。  
支持的报文大小范围为 **64 到 9728 字节**。

它支持 **多播报文** 以及 **混杂模式（promiscuous mode）**。

在 qemu 2.7 及以下版本中，Rx/Tx 队列的描述符数量被硬编码为 **256**。  
如果上层应用指定了不同的描述符数量，virtio PMD 会生成警告并回退到该硬编码值。  
自 qemu 2.8 及以上版本起，Rx 队列大小可以配置，最大可达 **1024**；默认 Rx 队列大小仍为 **256**。  
Tx 队列大小仍然被硬编码为 **256**。

支持 **MAC/VLAN 过滤功能**，但需要与 vhost/后端进行协商。  
当后端无法支持 VLAN 过滤时，虚拟机中的 virtio 应用不应启用 VLAN 过滤，以确保 virtio 端口能够正确配置。  
例如，在 testpmd 命令行中不要指定 `--enable-hw-vlan`。  
需要注意的是，MAC/VLAN 过滤是 **尽力而为（best effort）** 的：不期望的报文仍有可能到达。

`RTE_PKTMBUF_HEADROOM` 的定义应 **不小于**：
- 使用 hash report 时：`sizeof(struct virtio_net_hdr_hash_report)`，即 **20 字节**
- 使用可合并缓冲区或设置了 `VIRTIO_F_VERSION_1` 时：`sizeof(struct virtio_net_hdr_mrg_rxbuf)`，即 **12 字节**
- 使用不可合并缓冲区时：`sizeof(struct virtio_net_hdr)`，即 **10 字节**

Virtio **不支持运行时配置**。

Virtio 支持 **链路状态（Link State）中断**。

Virtio 支持 **Rx 中断**（目前仅支持 **队列与中断 1:1 映射**）。

Virtio 支持 **软件 VLAN 剥离与插入**。

Virtio 在 **packed virtqueue 模式** 下支持 **hash report 特性**。

当 UIO 模块不可用时，Virtio 支持使用 **端口 I/O（port IO）** 来获取 PCI 资源。

Virtio 支持 **RSS 接收模式**，包括：
- 40 字节可配置的 hash key 长度
- 128 个可配置的 RETA 表项
- 可配置的 hash 类型

---

## 65.3. 前置条件（Prerequisites）

需要满足以下条件：

- 在 BIOS 中开启 **VT-x 和 VT-d**
- Linux 内核启用 **KVM 模块**；加载 **vhost 模块** 并支持 **ioeventfd**
- 不带 vhost 支持的标准 qemu 后端未经过测试，且可能不被支持
- 使用 legacy 接口时，需要 **SYS_RAWIO capability**，以允许通过 `iopl()` 访问 PCI I/O 端口

---

## 65.4. 使用 qemu virtio 后端的 Virtio

（图 65.1：使用 qemu vhost 后端的 Host ↔ VM 通信示例）

![](https://doc.dpdk.org/guides/_images/host_vm_comms_qemu.png)

示例 qemu 启动命令：

```

qemu-system-x86_64 -enable-kvm -cpu host -m 2048 -smp 2 -mem-path /dev/hugepages -mem-prealloc 
-drive file=/data/DPDKVMS/dpdk-vm1 
-netdev tap,id=vm1_p1,ifname=tap0,script=no,vhost=on 
-device virtio-net-pci,netdev=vm1_p1,bus=pci.0,addr=0x3,ioeventfd=on 
-device pci-assign,host=04:10.1

```

在该示例中，**报文接收路径**为：

```

IXIA 报文发生器
→ 82599 PF
→ Linux Bridge
→ TAP0 的 socket 队列
→ Guest VM virtio 端口 0 Rx burst
→ Guest VM 82599 VF 端口 1 Tx burst
→ IXIA 报文发生器

```

**报文发送路径**为：

```

IXIA 报文发生器
→ Guest VM 82599 VF 端口 1 Rx burst
→ Guest VM virtio 端口 0 Tx burst
→ tap
→ Linux Bridge
→ 82599 PF
→ IXIA 报文发生器

```

---

## 65.5. Virtio PMD 的 Rx/Tx 回调函数

Virtio 驱动提供 **6 个 Rx 回调** 和 **3 个 Tx 回调**。

### Rx 回调

- `virtio_recv_pkts`  
  用于 split virtqueue，不支持可合并 Rx 缓冲区的普通版本

- `virtio_recv_mergeable_pkts`  
  用于 split virtqueue，支持可合并 Rx 缓冲区的普通版本

- `virtio_recv_pkts_vec`  
  用于 split virtqueue，不支持可合并 Rx 缓冲区的向量化版本，同时修正 available ring 索引并使用向量指令优化性能

- `virtio_recv_pkts_inorder`  
  用于 split virtqueue，支持可合并和不可合并 Rx 缓冲区的 in-order 版本

- `virtio_recv_pkts_packed`  
  用于 packed virtqueue，不支持可合并 Rx 缓冲区的普通及 in-order 版本

- `virtio_recv_mergeable_pkts_packed`  
  用于 packed virtqueue，支持可合并 Rx 缓冲区的普通及 in-order 版本

### Tx 回调

- `virtio_xmit_pkts`  
  用于 split virtqueue 的普通版本

- `virtio_xmit_pkts_inorder`  
  用于 split virtqueue 的 in-order 版本

- `virtio_xmit_pkts_packed`  
  用于 packed virtqueue 的普通及 in-order 版本

默认情况下，使用 **非向量化回调**：

- Rx：
  - 若禁用 mergeable Rx buffers，则使用 `virtio_recv_pkts` 或 `virtio_recv_pkts_packed`
  - 否则使用 `virtio_recv_mergeable_pkts` 或 `virtio_recv_mergeable_pkts_packed`
- Tx：
  - 使用 `virtio_xmit_pkts` 或 `virtio_xmit_pkts_packed`

### 向量化回调使用条件

当满足以下条件时使用向量化回调：

- 禁用 mergeable Rx buffers

对应回调为：

- Rx：`virtio_recv_pkts_vec`

目前 **packed virtqueue 尚不支持向量化 Rx 回调**。

示例（在 testpmd 中使用向量化 virtio PMD）：

```

dpdk-testpmd -l 0-2 -- -i --rxq=1 --txq=1 --nb-cores=1

```

**In-order 回调仅适用于模拟的 virtio user vdev**。

对于 split virtqueue：

- Rx：启用 in-order 时使用 `virtio_recv_pkts_inorder`
- Tx：启用 in-order 时使用 `virtio_xmit_pkts_inorder`

对于 packed virtqueue，默认回调已支持 in-order 特性。

---

## 65.6. 中断模式

通过 PCI 总线，virtio 设备有三种中断类型：

- 配置中断（config interrupt）
- Rx 中断
- Tx 中断

配置中断用于通知设备配置变化，尤其是 **链路状态（lsc）**。  
在 DPDK 语境下，中断模式被转换为 **Rx 中断**。

> 注意  
> Virtio PMD 已支持在链路状态变化时从 qemu 接收 lsc，尤其是在 vhost-user 断开连接时。  
> 但如果虚拟机由 qemu 2.6.2 或更低版本创建，则该功能失效，因为检测 vhost-user 断开的能力是在 qemu 2.7.0 中引入的。

### 65.6.1. Rx 中断的前置条件

要支持 Rx 中断：

1. 检查 guest 内核是否支持 **VFIO-NOIOMMU**  
   Linux 自 4.8.0 起支持 VFIO-NOIOMMU，需确保内核配置包含：

```

CONFIG_VFIO_NOIOMMU=y

```

2. 启动虚拟机时正确设置 **MSI-X 向量**  
启用多队列，并在 qemu 命令行中指定向量数量。  
最小值为 `(N+1)`，推荐值为 `(2N+2)`：

```

-device virtio-net-pci,mq=on,vectors=2N+2

```

3. 在虚拟机中以 NOIOMMU 模式加载 vfio 模块：

```

modprobe vfio enable_unsafe_noiommu_mode=1
modprobe vfio-pci

```

4. 在虚拟机中将 virtio 设备绑定到 vfio-pci：

```

./usertools/dpdk-devbind.py -b vfio-pci 00:03.0

```

### 65.6.2. 示例

使用 **l3fwd-power** 作为示例：

```

dpdk-l3fwd-power -l 0-1 -- -p 1 -P --config="(0,0,1)" 
--no-numa --parse-ptype

```

---

## 65.7. 运行时配置（Runtime Configuration）

### PCI virtio 驱动支持的 devargs

- `vdpa`  
  指定 virtio 设备以 **vDPA（vhost data path acceleration）** 模式运行，作为硬件 vhost 后端（默认：0，禁用）

- `speed`  
  指定 virtio 设备的链路速率（默认：0xffffffff，未知）

- `vectorized`  
  指定 virtio 是否优先使用向量化路径（默认：0，禁用）

### virtio-user vdev 支持的 devargs

- `path`：指定连接 vhost 后端的路径  
- `mac`：指定 MAC 地址  
- `cq`：启用控制队列（默认：0，禁用）  
- `queue_size`：指定队列大小（默认：256）  
- `queues`：指定队列数量（默认：1）  
- `iface`：指定 vhost-kernel 后端使用的宿主机接口名  
- `server`：启用 vhost-user 后端的 server 模式（默认：0，禁用）  
- `mrg_rxbuf`：启用可合并 Rx 缓冲区（默认：1，启用）  
- `in_order`：启用 in-order 特性（默认：1，启用）  
- `packed_vq`：启用 packed virtqueue（默认：0，禁用）  
- `speed`：指定链路速率（默认：0xffffffff，未知）  
- `vectorized`：指定是否优先使用向量化路径（默认：0，禁用）

---

## 65.8. Virtio 路径选择与使用

逻辑上，virtio PMD 根据 virtio 特性组合（Rx mergeable、In-order、Packed virtqueue）共有 **9 条路径**。

### 特性说明

- **Rx mergeable**：设备可通过合并多个描述符接收大报文  
- **In-order**：设备按描述符提供顺序使用描述符  
- **Packed virtqueue**：与 split virtqueue 不同，packed virtqueue 由描述符环、驱动事件抑制和设备事件抑制组成，其目标是减少 cache miss 并简化硬件实现

### 65.8.1. Virtio 路径选择

（以下路径选择规则保持与原文一致，略）

### 65.8.2. 各 Virtio 路径的 Rx/Tx 回调

（表 65.1：Virtio 路径与回调函数，对应关系与原文一致）

### 65.8.3. 各版本 Virtio 路径支持情况

- In-order 特性自 **DPDK 18.08** 起支持  
- Packed virtqueue 自 **DPDK 19.02** 起支持  

（表 65.2：Virtio 路径与 DPDK 版本支持情况）

### 65.8.4. QEMU 支持情况

- Qemu 支持 3 种 split virtqueue 路径  
- 自 qemu 4.2.0 起支持 packed virtqueue 路径

### 65.8.5. 调试方法

当升级驱动或配置后出现性能下降或其他问题时：

1. 运行 vhost/virtio 测试用例  
2. 运行 `perf top`，查看 virtio Rx/Tx 回调名称  
3. 参考上表确定所选 virtio 路径并定位根因

