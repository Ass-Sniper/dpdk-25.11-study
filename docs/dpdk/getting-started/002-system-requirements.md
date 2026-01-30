
SPDX-License-Identifier: BSD-3-Clause  
Copyright (c) 2010–2014 Intel Corporation

# System Requirements

本章节描述了 **D​​PDK 编译所需的软件和环境要求**。:contentReference[oaicite:1]{index=1}

---

## BIOS 设置前提（x86 平台）

对于大多数平台来说，**使用基本 DPDK 功能不需要额外的 BIOS 设置**。  
但是，为了启用 HPET 计时器及电源管理功能，并获得对小包的高性能支持，可能需要更改 BIOS 设置。  
有关所需更改的更多信息，请参见 “Enabling Additional Functionality” 章节。:contentReference[oaicite:2]{index=2}

---

## DPDK 编译要求

### 必需的工具和库

> ⚠️ 注  
> 各系统的安装命令和软件包名称可能不同。有关不同 Linux 发行版及其测试版本的详细信息，请参阅 DPDK Release Notes。

以下是构建 DPDK 所需的基本工具和库：

- 通用开发工具，包括支持 C11 标准（包含标准原子操作）的 C 编译器，例如：
  - GCC（建议版本 8.0+）
  - Clang（建议版本 7+）
  - 用于构建用户程序时链接 DPDK 的 `pkg-config` 或 `pkgconf`。
    
    在 RHEL 系统中，可使用以下命令安装开发工具：

    ```bash
    dnf groupinstall "Development Tools"
    ```
    
    在 Fedora 系统中：
    
    ```bash
    dnf group install development-tools
    ```
    
    在 Ubuntu 或 Debian 系统中：
    
    ```bash
    apt install build-essential
    ```
    
    在 Alpine Linux 中：
    
    ```bash
    apk add alpine-sdk bsd-compat-headers
    ```

* Python 3.6 或更高版本。([doc.dpdk.org][1])

* Meson（版本 0.57+）和 Ninja 构建工具。

  * 大多数 Linux 发行版可以通过软件包管理器安装 `meson` 和 `ninja-build`
  * 如果软件包版本低于最低版本，可使用 pip 安装：

    ```bash
    pip3 install meson ninja
    ```

* pyelftools（版本 0.22+）

  * Fedora 系统：

    ```bash
    dnf install python-pyelftools
    ```

  * RHEL/CentOS 系统：

    ```bash
    pip3 install pyelftools
    ```

  * Ubuntu/Debian 系统：

    ```bash
    apt install python3-pyelftools
    ```

  * Alpine Linux：

    ```bash
    apk add py3-elftools
    ```

* NUMA（Non Uniform Memory Access）支持库

  * RHEL/Fedora：

    ```bash
    dnf install numactl-devel
    ```

  * Debian/Ubuntu：

    ```bash
    apt install libnuma-dev
    ```

  * Alpine Linux：

    ```bash
    apk add numactl-dev
    ```

([doc.dpdk.org][1])

> ⚠️ 注
> 请确保对第三方库和软件应用了最新补丁，以避免已知的安全漏洞。([doc.dpdk.org][1])

---

### 可选工具

以下是一些可选编译工具：

* Intel® oneAPI DPC++/C++ Compiler
* IBM® Advance ToolChain for Powerlinux：一套开源开发工具和运行时库，用于在 Linux 上利用 IBM 最新 POWER 硬件特性。有关安装说明，请参阅 IBM 官方安装文档。([doc.dpdk.org][1])

---

### 附加库依赖

一些 DPDK 组件（如库和 Poll Mode Driver）具有额外的依赖项。
构建 DPDK 时，会自动检测这些依赖是否存在，并启用或禁用相关组件。
对于这些库，需要安装相应的开发包（`-devel` 或 `-dev`）。([doc.dpdk.org][1])

额外依赖示例：

* **libarchive**：用于某些单元测试，以 tar 获取资源。
* **libelf**：用于 BPF 库的编译和使用。
* 各 Poll Mode Driver 的依赖可以参见各自文档，如 NIC 驱动器部分。([doc.dpdk.org][1])

---

## 运行 DPDK 应用程序

运行 DPDK 应用程序时，可能需要对目标机器做一些定制配置。([doc.dpdk.org][1])

### 系统软件要求

以下是运行 DPDK 应用所需的系统软件配置：

* **Kernel 版本 >= 5.4**

  Linux userspace API 在不同内核之间兼容，但某些驱动和硬件支持可能依赖更高版本内核。
  当前 DPDK 测试基础设施测试的最老内核是发布时维护的最老 LTS。
  使用以下命令检查内核版本：

  ```bash
  uname -r
  ```

* **glibc >= 2.7**（用于 cpuset 支持）
  可使用以下命令检查 glibc 版本：

  ```bash
  ldd --version
  ```

* **内核配置**

  在 Fedora、Ubuntu、Red Hat Enterprise Linux 等常见发行版中，默认内核配置通常已启用运行大多数 DPDK 应用所需的选项。

  对于其他内核构建，建议启用以下内核选项：

  * HUGETLBFS
  * PROC_PAGE_MONITOR 支持
  * 如果需要 HPET 支持，还需启用 HPET 和 HPET_MMAP 配置选项

  这些选项有助于提供高精度定时和稳定内存支持。([doc.dpdk.org][1])

---

## 在 Linux 环境中使用 Hugepage

Hugepage 支持对于分配用于包缓冲的大内存池是必需的。
内核必须启用 HUGETLBFS 选项，如前文所述。
使用 hugepage 可减少页数量，从而减少 TLB（高速翻译缓存）缺失，提高性能。
如果不使用 hugepage，系统可能因标准 4k 页面导致高 TLB 缺失率而降低性能。([doc.dpdk.org][1])

### 为 DPDK 保留 Hugepage 页面

#### 1) 运行时保留

可以在运行时将所需 hugepages 数字写入对应页面大小的 nr_hugepages 文件，例如（假设需要 1024 个 2 MB 页面）：

```bash
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
```

在 NUMA 系统中，该命令一般会将 hugepages 平均分配给所有 NUMA 节点，前提是每个节点均有足够内存。
也可以通过指定 NUMA 节点来单独保留 hugepages，例如：

```bash
echo 1024 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 1024 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
```

可以使用 `dpdk-hugepages.py` 工具来管理 hugepages。([doc.dpdk.org][1])

> ⚠️ 注
> 某些内核版本可能不允许在运行时保留 1 GB hugepage，此时可能只能在引导时保留。([doc.dpdk.org][1])

---

#### 2) 引导时保留

在有些场景下，为了获得更多的物理连续内存，建议在系统引导时通过 kernel command line 参数保留 hugepages。例如：

```bash
hugepages=1024
```

对于 1 GB 页面大小：

```bash
default_hugepagesz=1G hugepagesz=1G hugepages=4
```

更多内核参数信息可参见内核源码中的 `Documentation/admin-guide/kernel-parameters.txt`。([doc.dpdk.org][1])

---

## 在 DPDK 中使用 Hugepage

* 如果不需要 secondary process（即多进程）支持，DPDK 可以通过 “in-memory” 模式使用 hugepages，无需任何额外配置。参见 EAL 参数文档了解更多详情。([doc.dpdk.org][1])

* 如果需要 secondary process 支持，则需要创建 Hugepage mount 点。
  现代 Linux 发行版通常在 `/dev/hugepages` 提供默认 mount 点，用于默认 page 大小。

* 若要使用非默认大小的 hugepages（例如 1 GB），则需要手动创建挂载点，例如：

  ```bash
  mkdir /mnt/huge
  mount -t hugetlbfs pagesize=1GB /mnt/huge
  ```


