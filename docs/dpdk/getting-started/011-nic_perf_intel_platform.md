
# 11. 如何在 Intel 平台上获得最佳网卡性能

本文档是针对在 Intel 平台上提升 DPDK 应用程序性能的逐步指南。

## 11.1. 硬件和内存要求

为了获得最佳性能，请使用 Intel Xeon（至强）级服务器系统，例如 Ivy Bridge、Haswell 或更新架构。

确保每个内存通道至少插入了一个内存 DIMM（内存条），且每个内存条的大小至少为 4GB。**注意：** 这是对性能影响最直接的因素之一。

您可以使用 `dmidecode` 检查内存配置，如下所示：

```bash
dmidecode -t memory | grep Locator

Locator: DIMM_A1
Bank Locator: NODE 1
Locator: DIMM_A2
Bank Locator: NODE 1
...
Locator: DIMM_G1
Bank Locator: NODE 2
Locator: DIMM_G2
Bank Locator: NODE 2

```

上述示例输出显示共有 8 个通道（从 A 到 H），每个通道有 2 个 DIMM 插槽。

您还可以使用 `dmidecode` 确定内存频率：

```bash
dmidecode -t memory | grep Speed

Speed: 2133 MHz
Configured Clock Speed: 2134 MHz
Speed: Unknown
...

```

输出显示速度为 2133 MHz (DDR4)，“Unknown”表示不存在。这与之前的输出一致，即每个通道插有一根内存条。

### 11.1.1. 网络接口卡（NIC）要求

* 使用 [DPDK 支持](http://core.dpdk.org/supported/) 的高端网卡，例如 Intel XL710 40GbE。
* 确保每张网卡都已刷入最新版本的 NVM/固件。
* 使用 PCIe Gen3 插槽（如 Gen3 x8 或 Gen3 x16），因为 PCIe Gen2 插槽无法为 2 x 10GbE 及更高速率提供足够的带宽。您可以使用 `lspci` 检查 PCI 插槽的速度：

```bash
lspci -s 03:00.1 -vv | grep LnkSta

LnkSta: Speed 8GT/s, Width x8, TrErr- Train- SlotClk+ DLActive- ...

```

* 将网卡插入 PCI 插槽时，务必检查标识（如 CPU0 或 CPU1），以确认其连接到了哪个插槽（Socket）。
* **需特别注意 NUMA：** 如果您使用来自不同网卡的 2 个或更多端口，最好确保这些网卡位于同一个 CPU 插槽上。下文将展示如何确定这一点。

### 11.1.2. BIOS 设置

以下是一些关于 BIOS 设置的建议。不同平台的 BIOS 命名可能不同，以下内容仅供参考：

* 建立系统的稳定状态，考虑审查以获得最佳性能特征所需的 BIOS 设置，例如：针对性能优化或针对能效优化。
* 使 BIOS 设置匹配您测试的应用程序需求。
* 通常，将 **Performance**（高性能）作为 CPU 电源和性能策略是一个合理的起点。
* 考虑使用 **Turbo Boost**（睿频）来提高核心频率。
* 在测试网卡的物理功能（PF）时，禁用所有虚拟化选项；如果想使用 VFIO，请开启 **VT-d**。

### 11.1.3. Linux 启动命令行

以下是一些关于 GRUB 启动设置的建议：

* 以默认的 grub 文件作为起点。
* 通过 grub 配置保留 **1G 大页**。例如，保留 8 个 1G 大页：
`default_hugepagesz=1G hugepagesz=1G hugepages=8`
* **隔离** 将用于 DPDK 的 CPU 核心。例如：
`isolcpus=2,3,4,5,6,7,8`
* 如果要使用 VFIO，请添加以下额外的 grub 参数：
`iommu=pt intel_iommu=on`

## 11.2. 运行 DPDK 前的配置

1. **保留大页：** 更多详情请参阅之前关于“在 Linux 环境中使用大页”的章节。
2. **获取大页大小：** `awk '/Hugepagesize/ {print $2}' /proc/meminfo`
3. **获取大页总数：** `awk '/HugePages_Total/ {print $2} ' /proc/meminfo`
4. **卸载大页挂载点：** `umount `awk '/hugetlbfs/ {print $2}' /proc/mounts``
5. **创建大页挂载目录：** `mkdir -p /mnt/huge`
6. **挂载到特定目录：** `mount -t hugetlbfs nodev /mnt/huge`

* 使用 DPDK 的 `cpu_layout` 实用程序检查 CPU 布局：
```bash
cd dpdk_folder
usertools/cpu_layout.py

```


或者运行 `lscpu` 查看每个插槽上的核心。
* **检查您的网卡 ID 及其相关的插槽 ID：**
列出所有带 PCI 地址和设备 ID 的网卡：`lspci -nn | grep Eth`
示例输出：
`82:00.0 Ethernet [0200]: Intel XL710 for 40GbE QSFP+ [8086:1583]`
`85:00.0 Ethernet [0200]: Intel XL710 for 40GbE QSFP+ [8086:1583]`
* **检查 PCI 设备相关的 NUMA 节点 ID：**
`cat /sys/bus/pci/devices/0000\:xx\:00.x/numa_node`
通常 `0x:00.x` 在插槽 0 上，`8x:00.x` 在插槽 1 上。**注意：** 为获得最佳性能，请确保 **核心（Core）和网卡（NIC）位于同一个插槽中**。在上面的例子中，`85:00.0` 位于插槽 1，应由插槽 1 上的核心使用，以获得最佳性能。
* 检查需要加载哪些内核驱动程序，以及是否需要从内核驱动程序解绑定网络端口。有关 DPDK 设置和 Linux 内核要求的更多详细信息，请参阅“从源码编译 DPDK 目标”和“Linux 驱动程序”章节。

