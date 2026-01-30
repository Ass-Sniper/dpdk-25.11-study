
SPDX-License-Identifier: BSD-3-Clause  
Copyright (c) 2010–2014 Intel Corporation

# 7. Linux Drivers

不同的 PMD（Poll Mode Driver）可能需要不同的内核驱动程序才能正常工作。  
根据所使用的 PMD，应加载相应的内核驱动程序，并将网络端口绑定到该驱动程序。

---

## 7.1. Binding and Unbinding Network Ports to/from the Kernel Modules

> ⚠️ 注意 使用 **分叉驱动（bifurcated driver）** 的 PMD **不应** 与其内核驱动程序解除绑定。  
本节适用于使用 **UIO 或 VFIO 驱动程序的 PMD**。  
更多详情请参阅 **7.4. Bifurcated Driver** 章节。

> ℹ️ 注意 建议在 **所有情况下** 使用 `vfio-pci` 作为 DPDK 绑定端口的内核模块。  
如果系统中没有 IOMMU，可在 **no-iommu 模式下** 使用 `vfio-pci`。  
如果由于某种原因无法使用 vfio，则可使用基于 UIO 的模块 `igb_uio` 和 `uio_pci_generic`。  
有关详细信息请参阅 **7.5. UIO**。

大多数设备要求 DPDK 使用的硬件在运行应用程序之前，**要从其原内核驱动解绑**，并绑定到 `vfio-pci` 内核模块。  
对于这类 PMD，Linux 控制下的任何网络端口或硬件都会被忽略，并且不能被应用程序使用。:contentReference[oaicite:4]{index=4}

DPDK 提供了一个名为 `dpdk-devbind.py` 的实用脚本（位于 `usertools` 子目录），  
可用于显示系统当前网络端口状态，并将这些端口从不同的内核模块（包括 VFIO 和 UIO）进行绑定和解绑。  
以下示例展示了如何使用该脚本。调用脚本的 `--help` 或 `--usage` 参数可以获取完整说明。  
注意：在运行 `dpdk-devbind.py` 之前，应先将要使用的 UIO 或 VFIO 内核模块加载到内核中。:contentReference[oaicite:5]{index=5}

> ℹ️ 注意 由于 VFIO 的工作方式，某些设备能否与 VFIO 一起使用存在限制，这主要取决于 **IOMMU group** 的机制。  
通常情况下，任何虚拟功能（VF）设备可以单独与 VFIO 一起使用，  
但对于物理设备，可能需要所有端口绑定到 VFIO，或有些端口绑定到 VFIO，而其它端口未绑定到任何驱动上。
>
> 如果设备位于 PCI-to-PCI 桥之后，则该桥会成为设备所属的 IOMMU 组的一部分。  
因此，为了让 VFIO 在桥下设备上正常工作，还需要将桥的驱动从桥 PCI 设备解绑。

> ℹ️ 注意 任何用户都可以运行 `dpdk-devbind.py` 脚本查看网络端口状态，  
但 **绑定或解绑网络端口需要 root 权限**。

示例 — 查看系统上所有网络端口状态：

```bash
./usertools/dpdk-devbind.py --status
````

输出可能类似：

```
Network devices using DPDK-compatible driver
============================================
0000:82:00.0 '82599EB 10-GbE NIC' drv=vfio-pci unused=ixgbe
0000:82:00.1 '82599EB 10-GbE NIC' drv=vfio-pci unused=ixgbe

Network devices using kernel driver
===================================
0000:04:00.0 'I350 1-GbE NIC' if=em0  drv=igb unused=vfio-pci *Active*
0000:04:00.1 'I350 1-GbE NIC' if=eth1 drv=igb unused=vfio-pci
0000:04:00.2 'I350 1-GbE NIC' if=eth2 drv=igb unused=vfio-pci
0000:04:00.3 'I350 1-GbE NIC' if=eth3 drv=igb unused=vfio-pci

Other network devices
=====================
<none>
```

([doc.dpdk.org][1])

示例 — 将设备 `eth1`（04:00.1）绑定到 `vfio-pci` 驱动：

```bash
./usertools/dpdk-devbind.py --bind=vfio-pci 04:00.1
```

或：

```bash
./usertools/dpdk-devbind.py --bind=vfio-pci eth1
```

当指定设备 ID 时，可对地址最后一部分使用通配符。例如要恢复设备 `82:00.0` 和 `82:00.1` 的原内核绑定：

```bash
./usertools/dpdk-devbind.py --bind=ixgbe 82:00.*
```

([doc.dpdk.org][1])

---

## 7.2. VFIO

VFIO 是一个依赖 **IOMMU 保护** 的强健且安全的驱动。
要使用 VFIO，必须加载 `vfio-pci` 模块：

```bash
sudo modprobe vfio-pci
```

([doc.dpdk.org][1])

VFIO 内核模块通常在所有发行版默认存在，但请查阅你所用发行版的文档确认情况。([doc.dpdk.org][1])

要使用完整的 VFIO 功能，**内核和 BIOS 必须支持并配置 IO 虚拟化**（例如 Intel® VT-d）。([doc.dpdk.org][1])

> ℹ️ 注意
> 在大多数情况下，将内核参数指定为 `"iommu=on"` 即足以将 Linux 内核配置为使用 IOMMU。([doc.dpdk.org][1])

为了在**非特权用户**下运行 DPDK 应用时正确使用 VFIO，还需要设置合适的权限。
更多信息请参阅：“Running DPDK Applications Without Root Privileges”。([doc.dpdk.org][1])

---

### 7.2.1. VFIO no-IOMMU mode

如果系统上没有 IOMMU，仍然可以使用 VFIO，但必须使用额外的模块参数加载：

```bash
modprobe vfio enable_unsafe_noiommu_mode=1
```

或者，在已经加载的内核模块上启用该选项：

```bash
echo 1 > /sys/module/vfio/parameters/enable_unsafe_noiommu_mode
```

之后，VFIO 可以像平常一样与硬件设备一起使用。([doc.dpdk.org][1])

> ⚠️ 警告
> 由于 no-IOMMU 模式放弃了 IOMMU 保护，因此本质上不安全。
> 但在 IOMMU 不可用的情况下，该模式可以让用户保留 VFIO 提供的设备访问与编程能力。([doc.dpdk.org][1])

---

### 7.2.2. VFIO Memory Mapping Limits

对于外部内存或 hugepages 的 DMA 映射，VFIO 接口被使用。
VFIO 不支持已映射内存的部分取消映射。
因此，DPDK 的内存以 **hugepage 粒度或系统页面粒度映射**。
DMA 映射数量受内核以进程锁定内存（rlimit）为界限。([doc.dpdk.org][1])

另一个适用于外部内存和系统内存的整体限额（也称容器限额）在内核 5.1 中被增加，
由 vfio 模块参数 `dma_entry_limit` 定义，默认值为 64K。
当应用超出 DMA 条目限制时，需要调整此限额以增加允许的 DMA 映射数。([doc.dpdk.org][1])

如果使用 `--no-huge` 选项，则使用较小的页面大小（如 `4K` 或 `64K`），
在这种情况下也需增加 `dma_entry_limit`。([doc.dpdk.org][1])

要更新 `dma_entry_limit`，必须使用额外模块参数加载 `vfio_iommu_type1`：

```bash
modprobe vfio_iommu_type1 dma_entry_limit=512000
```

或者修改已加载模块的参数：

```bash
echo 512000 > /sys/module/vfio_iommu_type1/parameters/dma_entry_limit
```

([doc.dpdk.org][1])

---

### 7.2.3. Creating Virtual Functions using vfio-pci

自 Linux 5.7 起，`vfio-pci` 模块支持创建虚拟功能（VFs）。
当 PF 绑定到 `vfio-pci` 模块之后，用户可以通过 sysfs 接口创建 VF，
这些 VF 将自动绑定到 vfio-pci 模块。([doc.dpdk.org][1])

当 PF 绑定到 vfio-pci 时，默认会生成一个随机的 VF token。
为安全起见，该 token 是只写的，因此用户不能直接从内核读取。
要访问这些 VF，用户需要创建一个新 token，并使用该 token 初始化 VF 和 PF 设备。
上述 token 为 UUID 格式，因此可以使用任意 UUID 生成工具生成。([doc.dpdk.org][1])

通过 EAL 参数 `--vfio-vf-token` 可以将该 VF token 传递给 DPDK。
该 token 将用于应用程序中的所有 PF 和 VF 端口。([doc.dpdk.org][1])

创建流程包括：

1. 通过 uuid 命令生成 VF token：

   ```
   14d63f20-8445-11ea-8900-1f9ce7d5650d
   ```
2. 以带 `enable_sriov` 参数方式加载 vfio-pci：

   ```bash
   sudo modprobe vfio-pci enable_sriov=1
   ```

   或在 sysfs 中启用该参数：

   ```bash
   echo 1 | sudo tee /sys/module/vfio_pci/parameters/enable_sriov
   ```
3. 将 PCI 设备绑定到 vfio-pci：

   ```bash
   ./usertools/dpdk-devbind.py -b vfio-pci 0000:86:00.0
   ```
4. 创建所需数量的 VF 设备

   ```bash
   echo 2 > /sys/bus/pci/devices/0000:86:00.0/sriov_numvfs
   ```
5. 启动管理 PF 设备的 DPDK 应用

   ```bash
   <build_dir>/app/dpdk-testpmd … --vfio-vf-token=14d63f20-8445-11ea-8900-1f9ce7d5650d …
   ```
6. 启动管理 VF 设备的 DPDK 应用

   ```bash
   <build_dir>/app/dpdk-testpmd …
   ```

([doc.dpdk.org][1])

> ℹ️ 注意
> Linux 版本早于 5.7 的内核不支持 VFIO 框架下的虚拟功能创建。([doc.dpdk.org][1])

---

## 7.3. VFIO Platform

VFIO Platform 是一个内核驱动，通过添加对**IOMMU 后平台设备**的支持来扩展 VFIO 的功能。
Linux 通常在系统启动期间从设备树中识别平台设备，而不像 PCI 设备一样具有内建信息。([doc.dpdk.org][1])

要使用 VFIO Platform，必须先加载 `vfio-platform` 模块：

```bash
sudo modprobe vfio-platform
```

([doc.dpdk.org][1])

> ℹ️ 注意
> 默认情况下，`vfio-platform` 假定平台设备具有专用的 reset 驱动。
> 如果此驱动缺失或设备不需要 reset，则可以通过参数 `reset_required=0` 关闭该要求。([doc.dpdk.org][1])

之后，需要将平台设备绑定到 vfio-platform。
这是标准流程，需要两个步骤：

1. 设置 driver_override：

   ```bash
   sudo echo vfio-platform > /sys/bus/platform/devices/DEV/driver_override
   ```
2. 绑定设备：

   ```bash
   sudo echo DEV > /sys/bus/platform/drivers/vfio-platform/bind
   ```

([doc.dpdk.org][1])

在应用程序启动时，DPDK 平台总线驱动程序（platform bus driver）会扫描 /sys/bus/platform/devices 目录，寻找那些其 driver 符号链接指向 vfio-platform 驱动的设备。
最后，将扫描到的设备与可用的 PMD（轮询模式驱动）进行匹配。
如果满足以下任一条件，则匹配成功：PMD 名称或 PMD 别名与内核驱动程序名称匹配，或者 PMD 名称与平台设备名称匹配（按上述顺序依次匹配）。

VFIO Platform 依赖于 ARM/ARM64 架构，并且通常在运行于这些系统的发行版中处于启用状态。请查阅您的发行版文档以确认是否如此。

---

## 7.4. Bifurcated Driver

使用分叉驱动的 PMD 与设备内核驱动共存。
在这种模式下，NIC 由内核控制，而数据平面由 PMD 直接在设备上执行。([doc.dpdk.org][1])

这种模型具有以下优势：([doc.dpdk.org][1])

* 内存管理和隔离由内核完成，因此安全性和健壮性更高
* 在运行 DPDK 应用的同时，可使用传统 Linux 工具（如 ethtool 或 ifconfig）查看 NIC 状态
* 能使部分流量由 DPDK 应用处理，其余流量由内核驱动处理
  这些流量分叉由 NIC 硬件执行，例如使用 “Flow isolated mode” 能选择性地指定接收哪些数据包到 DPDK。([doc.dpdk.org][1])

更多关于分叉驱动的信息可参考 NVIDIA 的 bifurcated PMD 演示资料。([doc.dpdk.org][1])

---

## 7.5. UIO

> ⚠️ 警告

使用 UIO 驱动本质上不安全，因为此方法缺乏 IOMMU 保护，并且只能由 root 用户执行。([doc.dpdk.org][1])

当无法使用 VFIO 时，可使用替代驱动。
在许多情况下，Linux 内核中包含的标准模块 `uio_pci_generic` 可作为 VFIO 的替代方案：

```bash
sudo modprobe uio_pci_generic
```

([doc.dpdk.org][1])

> ℹ️ 注意
> `uio_pci_generic` 模块不支持创建虚拟功能（VFs）。([doc.dpdk.org][1])

作为 `uio_pci_generic` 的替代，DPDK 还提供 `igb_uio` 模块，可在 DPDK 的 kmod 仓库中找到。
加载方式如下：

```bash
sudo modprobe uio
sudo insmod igb_uio.ko
```

([doc.dpdk.org][1])

> ℹ️ 注意
> 对于某些不支持传统中断的设备（例如 VF 设备），可能需要使用 `igb_uio` 模块代替 `uio_pci_generic`。([doc.dpdk.org][1])

> ℹ️ 注意
> 如果启用了 UEFI 安全 Boot，则 Linux 内核可能禁止使用 UIO。
> 因此建议将要用于 DPDK 的设备绑定到 `vfio-pci` 而不是任何基于 UIO 的模块。([doc.dpdk.org][1])

> ℹ️ 注意
> 如果设备已绑定到基于 UIO 的内核模块，请确保 IOMMU 被禁用或处于透传模式。
> 可在 x86_64 系统的 GRUB 命令行中添加例如 `intel_iommu=off`、`amd_iommu=off` 或 `intel_iommu=on iommu=pt`；
> 或在 aarch64 系统中添加 `iommu.passthrough=1`。([doc.dpdk.org][1])


[1]: https://doc.dpdk.org/guides/linux_gsg/linux_drivers.html "7. Linux Drivers — Data Plane Development Kit 25.11.0 documentation"

