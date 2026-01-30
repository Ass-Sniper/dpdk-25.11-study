
# 4. 针对 aarch64 和 aarch32 交叉编译 DPDK

本章节介绍了如何在 x86 编译机上为 aarch64 交叉编译 DPDK，以及在 aarch64 编译机上编译 32 位 aarch32 DPDK。

> **注意**
> 虽然建议在 aarch64 上原生构建 DPDK（就像在 x86 上一样），但也可以为 aarch64 进行交叉编译。交叉编译可以使用 aarch64 交叉编译器 GNU 工具链或 LLVM/Clang 工具链。

## 4.1. 前提条件

### 4.1.1. NUMA 库

大多数现代机器都需要 NUMA 支持，而非 NUMA 架构则不需要。

> **注意**
> 编译 NUMA 库时，请运行 `libtool --version` 以确保 libtool 版本 >= 2.2，否则编译会出错。

```bash
git clone https://github.com/numactl/numactl.git
cd numactl
git checkout v2.0.13 -b v2.0.13
./autogen.sh
autoconf -i
./configure --host=aarch64-linux-gnu CC=aarch64-none-linux-gnu-gcc --prefix=<numa_install_dir>
make install

```

> **注意**
> 如果你按照以下指南下载 GCC，编译器名称为 `aarch64-none-linux-gnu-gcc`。如果你使用不同的编译器，请确保使用正确的可执行文件名。

NUMA 头文件和库文件将分别生成在 `<numa_install_dir>` 下的 `include` 和 `lib` 文件夹中。

### 4.1.2. Meson 前提条件

Meson 依赖 `pkgconfig` 来查找依赖项。aarch64 需要 `pkg-config-aarch64-linux-gnu` 包。在 Ubuntu 中安装：

```bash
sudo apt install pkg-config-aarch64-linux-gnu

```

对于 aarch32，安装 `pkg-config-arm-linux-gnueabihf`：

```bash
sudo apt install pkg-config-arm-linux-gnueabihf

```

## 4.2. GNU 工具链

### 4.2.1. 获取交叉工具链

最新的 GNU 交叉编译器工具链可以从以下地址下载：[https://developer.arm.com/open-source/gnu-toolchain/gnu-a/downloads](https://developer.arm.com/open-source/gnu-toolchain/gnu-a/downloads)。

始终建议从该页面检查并获取最新的编译器工具，以生成更优的代码。截至本文撰写时，最新版本为 9.2-2019.12，以下说明以该版本为例。

**对于 aarch64：**

```bash
wget https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
tar -xvf gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
export PATH=$PATH:<cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin

```

**对于 aarch32：**

```bash
wget https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.tar.xz
tar -xvf gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.tar.xz
export PATH=$PATH:<cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/bin

```

> **注意**
> 有关宿主机要求和其他信息，请参考发行说明章节：[https://releases.linaro.org/components/toolchain/binaries/](https://releases.linaro.org/components/toolchain/binaries/)

### 4.2.2. 为 GNU 工具链扩充 NUMA 支持

将 NUMA 头文件和库拷贝到交叉编译器的目录中：

```bash
cp <numa_install_dir>/include/numa*.h <cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/usr/include/
cp <numa_install_dir>/lib/libnuma.a <cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/lib/gcc/aarch64-none-linux-gnu/9.2.1/
cp <numa_install_dir>/lib/libnuma.so <cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/lib/gcc/aarch64-none-linux-gnu/9.2.1/

```

> **注意**
> 使用 `LDFLAGS` 和 `CFLAGS` 并不是拷贝文件的可行替代方案。Meson 文档指出不推荐这样做，因为在 Meson 中使用它们有很多陷阱，尤其是在重新构建项目时。一个可行的替代方案是在 Meson 0.51.0 及更高版本中使用 `c_args` 和 `c_link_args` 选项：
> `-Dc_args=-I<numa_install_dir>/include -Dc_link_args=-L<numa_install_dir>/lib`

对于低于 0.51.0 的 Meson 版本，`c_args` 和 `c_link_args` 选项不适用于交叉编译。但是，编译器/链接器标志可以添加到交叉定义文件（cross file）的 `[properties]` 下：

```toml
c_args = ['-I<numa_install_dir>/include']
c_link_args = ['-L<numa_install_dir>/lib']

```

### 4.2.3. 使用 Meson 和 GNU 工具链交叉编译 DPDK

> **注意**
> 交叉定义文件中的 GCC 二进制文件名与下载的文件名不同，下载的文件名中多了一个 `-none-`。使用下载的交叉编译器时，请相应修改交叉定义文件中的二进制文件名。

一个修改了名称并添加了 NUMA 路径的交叉定义文件示例如下：

```toml
[binaries]
c = 'aarch64-none-linux-gnu-gcc'
cpp = 'aarch64-none-linux-gnu-cpp'
ar = 'aarch64-none-linux-gnu-gcc-ar'
strip = 'aarch64-none-linux-gnu-strip'
pkgconfig = 'aarch64-linux-gnu-pkg-config' # 下载的二进制文件中
# 不包含 pkgconfig 二进制文件，因此不修改
pcap-config = ''

[host_machine]
system = 'linux'
cpu_family = 'aarch64'
cpu = 'armv8-a'
endian = 'little'

[properties]
# 生成可在所有 Armv8 机器上移植的二进制文件
platform = 'generic'
c_args = ['-I<numa_install_dir>/include'] # 将 <numa_install_dir> 替换为你的路径
c_link_args = ['-L<numa_install_dir>/lib']

```

要交叉编译 DPDK 到目标机器，可以使用以下命令：

```bash
meson setup cross-build --cross-file <target_machine_configuration>
ninja -C cross-build

```

例如，如果目标机器是 aarch64，且交叉定义文件已相应修改，可以使用：

```bash
meson setup aarch64-build-gcc --cross-file config/arm/arm64_armv8_linux_gcc
ninja -C aarch64-build-gcc

```

如果目标机器是 aarch32：

```bash
meson setup aarch32-build --cross-file config/arm/arm32_armv8_linux_gcc
ninja -C aarch32-build

```

## 4.3. LLVM/Clang 工具链

### 4.3.1. 获取交叉工具链

最新的 LLVM/Clang 交叉编译器工具链可以从以下地址下载：[https://developer.arm.com/tools-and-software/open-source-software/developer-tools/llvm-toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/llvm-toolchain)。

```bash
# Ubuntu 二进制文件
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz

```

LLVM/Clang 工具链没有实现标准 C 库。我们可以使用 GNU 工具链提供的实现。请参考“获取 GNU 工具链”部分来获取 GNU 工具链。

### 4.3.2. 解压并添加到 PATH

```bash
tar -xvf clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
export PATH=$PATH:<cross_install_dir>/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04/bin

```

### 4.3.3. 使用 Meson 和 LLVM/Clang 工具链交叉编译 DPDK

> **注意**
> 若要使用 NUMA 库，请遵循与“为 GNU 工具链扩充 NUMA 支持”相同的步骤。

必须在交叉定义文件中指定 GNU 标准库（stdlib）的路径。扩充默认的交叉定义文件 `config/arm/arm64_armv8_linux_clang_ubuntu1804` 中的 `c_args` 和 `c_link_args` 如下：

```toml
...
c_args = ['-target', 'aarch64-linux-gnu', '--sysroot', '<cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc']
c_link_args = ['-target', 'aarch64-linux-gnu', '-fuse-ld=lld', '--sysroot', '<cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc', '--gcc-toolchain=<cross_install_dir>/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu']

```

假设扩充后的文件命名为 `arm64_armv8_linux_clang`，使用以下命令交叉编译 DPDK：

```bash
meson setup aarch64-build-clang --cross-file config/arm/arm64_armv8_linux_clang
ninja -C aarch64-build-clang

```

### 4.3.4. 在 Ubuntu 18.04 上使用 Meson 和 LLVM/Clang 工具链交叉编译

在大多数流行的 Linux 发行版上，不需要手动下载工具链，而是可以使用发行版提供的软件包。在 Ubuntu 18.04 上，需要以下包：

```bash
sudo apt-get install pkg-config-aarch64-linux-gnu clang llvm llvm-dev lld libc6-dev-arm64-cross libatomic1-arm64-cross libgcc-8-dev-arm64-cross

```

使用以下命令交叉编译：

```bash
meson setup aarch64-build-clang --cross-file config/arm/arm64_armv8_linux_clang_ubuntu1804
ninja -C aarch64-build-clang

```

## 4.4. 在 aarch64 编译机上为 aarch64 SoC 构建

如果你希望在 aarch64 编译机上为不同的 aarch64 SoC 进行构建，你不需要单独的交叉工具链，只需要一套不同的配置选项。要为特定的 aarch64 SoC 构建，请使用 `-Dplatform` meson 选项：

```bash
meson setup soc_build -Dplatform=<target_soc>

```

将 `<target_soc>` 替换为支持的 SoC 之一：

* `generic`: 针对 armv8 aarch64 执行模式的通用非优化构建。
* `generic_aarch32`: 针对 armv8 aarch32 执行模式的通用非优化构建。
* `altra`: Ampere Altra/AltraMax
* `ampereone`: Ampere AmpereOne
* `ampereoneac04`: Ampere AmpereOneAC04
* `armada`: Marvell ARMADA
* `bluefield`: NVIDIA BlueField
* `bluefield3`: NVIDIA BlueField-3
* `capri`: AMD Pensando Capri
* `cdx`: AMD CDX
* `centriq2400`: Qualcomm Centriq 2400
* `cn9k`: Marvell OCTEON 9
* `cn10k`: Marvell OCTEON 10
* `cobalt100`: Microsoft Azure Cobalt 100
* `dpaa`: NXP DPAA
* `elba`: AMD Pensando Elba
* `emag`: Ampere eMAG
* `ft2000plus`: Phytium FT-2000+
* `grace`: NVIDIA Grace
* `graviton2`: AWS Graviton2
* `graviton3`: AWS Graviton3
* `graviton4`: AWS Graviton4
* `hip10`: HiSilicon HIP10
* `hip12`: HiSilicon HIP12
* `imx`: NXP IMX
* `kunpeng920`: HiSilicon Kunpeng 920
* `kunpeng930`: HiSilicon Kunpeng 930
* `n1sdp`: Arm Neoverse N1SDP
* `n2`: Arm Neoverse N2
* `n3`: Arm Neoverse N3
* `odyssey`: Marvell Odyssey
* `stingray`: Broadcom Stingray
* `thunderx2`: Marvell ThunderX2 T99
* `thunderxt83`: Marvell ThunderX T83
* `thunderxt88`: Marvell ThunderX T88
* `tys2500`: Phytium TengYun S2500
* `tys5000c`: Phytium TengYun S5000c
* `v2`: Arm Neoverse V2
* `v3`: Arm Neoverse V3

这些 SoC 也用于交叉定义文件中，例如：

```toml
[properties]
# 生成可在所有 Armv8 机器上移植的二进制文件
platform = 'generic'

```

## 4.5. 支持的 SoC 配置

SoC 配置是实现者（implementer）ID、CPU 部件号（part number）配置以及 SoC 特定配置的结合：

```python
soc_<name> = {
    'description': 'SoC Description',      # 必填
    'implementer': <implementer_id>,        # 必填
    'part_number': <part_number>,          # 必填
    'numa': false,                         # 可选，为非 NUMA SoC 指定
    'enable_drivers': 'common/*,bus/*',     # 可选，要构建的驱动列表（逗号分隔），支持通配符
    'disable_drivers': 'crypto/*',         # 可选，要禁用的驱动列表（逗号分隔），支持通配符
    'flags': [
        ['RTE_MAX_LCORE', '16'],
        ['RTE_MAX_NUMA_NODES', '1']
    ]                                      # 可选，要添加或覆盖的 DPDK 选项列表
}

```

其中 `<implementer_id>` 是 `config/arm/meson.build` 中 `implementers` 字典里定义的键（例如 `0x41`），
而 `part_number` 是 `implementers[<implementer_id>]['part_number_config']` 字典里定义的键
（即部件号必须为该实现者定义过，例如对于 `0x41`，一个有效值是 `0xd49`，代表 neoverse-n2 SoC）。

