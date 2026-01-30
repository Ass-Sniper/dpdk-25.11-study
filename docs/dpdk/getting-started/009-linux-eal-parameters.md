
# 9. EAL 参数

本文档包含所有 EAL（环境抽象层）参数的列表。这些参数可用于在 Linux 上运行的任何 DPDK 应用程序。

## 9.1. 通用 EAL 参数

以下 EAL 参数对 DPDK 支持的所有平台都是通用的。

### 9.1.1. 逻辑核心（Lcore）相关选项

* **`-l, --lcores <核心列表>`**
设置运行的核心列表。
最简单的参数格式是 `<c1>[-c2][,c3[-c4],...]`，其中 `c1`、`c2` 等是 0 到 `RTE_MAX_LCORE`（默认 128）之间的核心索引。
此参数也可用于将逻辑核心（lcore）集映射到物理 CPU 集。
参数格式为：`<lcores[@cpus]>[<,lcores[@cpus]>...]`
lcore 和 CPU 列表通过 `(` 和 `)` 分组。在组内，`-` 字符用作范围分隔符，`,` 用作单个数字分隔符。对于单元素组，可以省略分组括号 `()`。如果 cpus 和 lcores 的值相同，可以省略 `@`。
**示例：**
* `--lcores=1-3`：在物理 CPU 1、2 和 3 上运行线程，每个线程的 lcore ID 与物理 CPU ID 相同。
* `--lcores=1@(1,2)`：运行一个 lcore ID 为 1 的单线程，但该线程绑定到物理 CPU 1 和 2，由操作系统决定运行在其中哪一个上。
* `--lcores=1@31,2@32,3@33`：运行内部 lcore ID 分别为 1、2 和 3 的线程，但这些线程分别绑定到物理 CPU 31、32 和 33。
* `--lcores='(1-3)@(31-33)'`：运行三个 lcore ID 为 1、2 和 3 的线程。与上例不同，每个线程不绑定到特定的物理 CPU，而是所有三个线程都绑定到三个物理 CPU 31、32 和 33。这意味着这三个线程可以在物理 CPU 31-33 之间移动，由 OS 在运行期间决定。
* `--lcores='(1-3)@20'`：运行三个线程（lcore ID 为 1、2 和 3），所有三个线程都绑定到（只能运行在）物理 CPU 20 上。


> **注意**：将多个 DPDK lcore 绑定到单个物理 CPU 可能会导致性能不佳，或在使用 DPDK 环（ring）、内存池或自旋锁时引起死锁。此类配置应谨慎使用。
> **注意**：如示例所示，取决于所使用的 shell，有时需要将 lcores 参数值用引号括起来，例如当参数值以 `(` 字符开头时。
> **注意**：在给定实例中，只能使用 `--lcores`、`-l` 或 `-c` 其中一种核心选项。


* **`-R, --remap-lcore-ids [<起始 lcore id>]`**
启用将 lcore-id 自动重新映射为从 0（或用户提供的值）开始的连续集合。
当传递此标志时，由核心掩码或核心列表选项指定的核心被视为应用程序运行的物理核心，每个核心将启动一个线程，并具有顺序递增的 lcore-id。
例如：`dpdk-test -l 20-24 -R` 将在物理核心 20 到 24 上启动 5 个线程，lcore-id 分别为 0 到 4。
* **`--main-lcore <核心 ID>`**
指定用作主核心的核心 ID。
* **`-S, --service-corelist <服务核心列表>`**
指定用作服务核心的核心列表。

### 9.1.2. 设备相关选项

* **`-b, --block <[domain:]bus:devid.func>`**
跳过对 PCI 设备的探测，防止 EAL 使用该设备。允许使用多个 `-b` 选项。
> **注意**：阻止列表（Block list）不能与允许列表 `-a` 选项同时使用。


* **`-a, --allow <[domain:]bus:devid.func>`**
将 PCI 设备添加到探测列表。
* **`--vdev <设备参数>`**
使用以下格式添加虚拟设备：`<驱动名称><ID>[,key=val, ...]`
例如：`--vdev 'net_pcap0,rx_pcap=input.pcap,tx_pcap=output.pcap'`
* **`-d, --driver-path <共享对象或目录路径>`**
加载外部驱动程序。参数可以是单个共享对象文件，也可以是包含多个驱动程序共享对象的目录。
* **`--no-pci`**
禁用 PCI 总线。

### 9.1.3. 多进程相关选项

* **`--proc-type <primary|secondary|auto>`**
设置当前进程的类型（主进程、从进程或自动）。
* **`--base-virtaddr <地址>`**
尝试为 DPDK 主进程的所有内存映射使用不同的起始地址。这有助于解决从进程由于地址空间冲突而无法启动的问题。

### 9.1.4. 内存相关选项

* **`-n, --memory-channels <通道数>`**
设置使用的内存通道数量。
* **`-r, --memory-ranks <Rank数>`**
设置内存 Rank 数量（默认自动检测）。
* **`-m, --memory-size <MB>`**
启动时预分配的内存量。
* **`--in-memory`**
不创建任何共享数据结构，完全在内存中运行。隐含 `--no-shconf` 和（如果适用）`--huge-unlink`。
* **`--iova-mode <pa|va>`**
强制 IOVA 模式为特定值（物理地址 PA 或虚拟地址 VA）。
* **`--huge-worker-stack[=size]`**
从大页内存分配工作线程栈。

### 9.1.5. 调试选项

* **`--no-shconf`**
不创建共享文件（隐含不支持多进程）。
* **`--no-huge`**
使用匿名内存代替大页（隐含不支持多进程）。
* **`--log-level <类型:值>`**
为特定组件指定日志级别。例如：`--log-level lib.eal:debug`。
* **`--trace=<正则匹配>`**
基于正则表达式启用追踪。
* **`--trace-dir=<目录路径>`**
指定追踪输出的目录。
* **`--trace-bufsz=<值>`**
指定每个线程分配用于追踪输出的最大内存。支持 `B`、`K`、`M` 单位。
* **`--trace-mode=<o[verwrite] | d[iscard] >`**
指定追踪输出文件的更新模式：覆盖（overwrite）或丢弃（discard）。

### 9.1.6. 其他选项

* **`-h, --help`**：显示帮助信息。
* **`-v, --version`**：启动时显示版本信息。
* **`--telemetry`**：启用遥测（默认启用）。
* **`--force-max-simd-bitwidth=<值>`**：指定允许的最大 SIMD 位宽。例如 `512`（支持 AVX-512）或 `64`（禁用所有向量代码）。

---

## 9.2. Linux 特有的 EAL 参数

### 9.2.1. 设备相关选项

* **`--create-uio-dev`**：为绑定到 `igb_uio` 驱动的设备创建 `/dev/uioX` 文件。
* **`--vmware-tsc-map`**：使用 VMware TSC 映射代替原生 RDTSC。
* **`--no-hpet`**：不使用 HPET 定时器。
* **`--vfio-intr <legacy|msi|msix>`**：指定 VFIO 驱动设备的中断模式。
* **`--vfio-vf-token <uuid>`**：指定 VFIO 驱动设备的 VF Token。

### 9.2.2. 多进程相关选项

* **`--file-prefix <前缀名称>`**
为 DPDK 进程使用不同的共享数据文件前缀。允许在不同前缀下运行多个独立的 DPDK 主/从进程。

### 9.2.3. 内存相关选项

* **`--legacy-mem`**：使用旧版 DPDK 内存分配模式。
* **`--numa-mem <每个 NUMA 节点的内存量>`**
预分配指定的每个 NUMA 节点的内存量（逗号分隔）。例如：`--numa-mem 1024,2048`。
* **`--numa-limit <限额>`**：设置每个 NUMA 节点的内存使用上限。
* **`--single-file-segments`**：在 hugetlbfs 中创建更少的文件。
* **`--huge-dir <hugetlbfs 目录路径>`**：使用指定的大页挂载点。
* **`--huge-unlink[=existing|always|never]`**
* `existing` (默认)：移除并重新创建现有的大页文件，以确保内核清理内存。
* `always`：映射前移除文件，使应用程序在 hugetlbfs 中不留文件（不支持多进程）。
* `never`：从不移除，而是重新映射，允许大页重用以加快重启。


* **`--match-allocations`**：将大页释放回系统时，保持与最初分配时完全一致。

### 9.2.4. 其他选项

* **`--syslog <syslog 设施>`**
设置 syslog 设施。有效值包括：`auth`、`cron`、`daemon`、`ftp`、`kern`、`lpr`、`mail`、`news`、`syslog`、`user`、`uucp` 以及 `local0` 到 `local7`。

