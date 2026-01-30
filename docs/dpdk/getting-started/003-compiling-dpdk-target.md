
SPDX-License-Identifier: BSD-3-Clause  
Copyright (c) 2010–2014 Intel Corporation

# 3. Compiling the DPDK Target from Source

---

## 3.1 Uncompress DPDK and Browse Sources

首先，解压归档文件并进入解压后的 DPDK 源码目录：

```bash
tar xJf dpdk-<version>.tar.xz
cd dpdk-<version>
````

DPDK 由多个目录组成，包括：

* `doc`：DPDK 文档
* `license`：DPDK 许可证信息
* `lib`：DPDK 库的源代码
* `drivers`：DPDK poll-mode 驱动的源代码
* `app`：DPDK 应用程序（自动测试）的源代码
* `examples`：DPDK 应用程序示例的源代码
* `config`, `buildtools`：框架相关脚本和配置
* `usertools`：供 DPDK 应用最终用户使用的实用脚本
* `devtools`：供 DPDK 开发者使用的脚本
* `kernel`：某些操作系统所需的内核模块

---

## 3.2 Compiling and Installing DPDK System-wide

可以使用构建工具 `meson` 和 `ninja` 配置、构建并安装 DPDK 到你的系统中。([doc.dpdk.org][1])

---

### 3.2.1 DPDK Configuration

要配置 DPDK 构建，请使用：

```bash
meson setup <options> build
```

其中：

* `build` 是希望输出的构建目录
* `<options>` 可以为空，也可以是多个 meson 或 DPDK 特定的构建选项（本节稍后会描述）

配置过程结束后会输出将构建和安装哪些 DPDK 库和驱动，并为每个被禁用的项给出原因。
这些信息可以用于识别某个驱动缺失了哪些依赖包。([doc.dpdk.org][1])

---

配置完成后，要构建并在系统范围内安装 DPDK，请执行：

```bash
cd build
ninja
meson install
ldconfig
```

以上最后两个命令通常需要以 root 权限运行：

* `meson install` 会将构建好的对象复制到最终的系统路径
* `ldconfig` 使动态链接器 ld.so 更新缓存以纳入新对象([doc.dpdk.org][1])

---

> **注意**

在某些 Linux 发行版中（例如 Fedora 或 Red Hat），
`/usr/local` 下的路径并不在默认的链接器路径中。
因此在这些发行版中，在运行 `ldconfig` 之前，
应将 `/usr/local/lib` 和 `/usr/local/lib64` 添加到 `/etc/ld.so.conf.d/` 下的某个文件中。([doc.dpdk.org][1])

---

### 3.2.2 Adjusting Build Options

DPDK 在构建配置过程中有许多可调整选项。
这些选项可以通过在已配置的构建目录中运行：

```bash
meson configure
```

来列出。
其中很多选项来自于 meson 本身，可以在 Meson 官方文档中查看其说明。([doc.dpdk.org][1])

例如，要将默认的构建类型 `debugoptimized` 改为普通的 `debug` 构建，可以：

* 在初次配置构建时传入 `-Dbuildtype=debug` 或 `--buildtype=debug`
* 或者在构建目录中运行：

  ```bash
  meson configure -Dbuildtype=debug
  ```

其他选项也类似调整。
其中比较重要的是 `platform` 参数，它指定了一套将用于构建的配置参数：

* `-Dplatform=native`：将配置针对当前构建主机优化
* `-Dplatform=generic`：将使用适用于与构建主机相同架构所有机器的通用配置
* `-Dplatform=<SoC>`：使用针对特定 SoC 优化的配置。
  可在 `config/arm/meson.build` 中的 `socs` 字典查看支持的 SoC 列表。([doc.dpdk.org][1])

---

除了 `platform` 外，还可通过 `meson configure` 或 `-D<name>=<value>` 来覆盖其他选项值。例如：

```bash
meson configure -Dmax_lcores=256
```

设置最大逻辑核数为 256。([doc.dpdk.org][1])

---

你也可以在构建时让 meson 自动构建部分示例应用。例如：

```bash
meson setup -Dexamples=l2fwd,l3fwd build
```

同时也支持：

```bash
meson setup -Dexamples=all build
```

来构建所有在当前系统上其依赖满足条件的示例应用。([doc.dpdk.org][1])

---

### 3.2.3 Building 32-bit DPDK on 64-bit Systems

要在 64 位操作系统上构建 32 位的 DPDK 副本，
需要将 `-m32` 标志传递给编译器和链接器以强制生成 32 位对象和二进制。

这可以通过设置环境变量 `CFLAGS` 和 `LDFLAGS` 完成，
也可以通过向 meson 传递：

```bash
-Dc_args=-m32 -Dc_link_args=-m32
```

为了正确识别和使用任何依赖包，
还必须将 `pkg-config` 配置为在适当目录查找 32 位库的 `.pc` 文件。
可以通过设置 `PKG_CONFIG_LIBDIR` 实现。([doc.dpdk.org][1])

---

在 RHEL/Fedora 系统上，可通过如下命令配置 32 位构建（假设已安装相关 32 位开发包）：

```bash
PKG_CONFIG_LIBDIR=/usr/lib/pkgconfig \
    meson setup -Dc_args='-m32' -Dc_link_args='-m32' build
```

对于 Debian/Ubuntu 系统，等价命令为：

```bash
PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu/pkgconfig \
    meson setup -Dc_args='-m32' -Dc_link_args='-m32' build
```

32 位构建配置好之后，可以像前文一样用 `ninja` 进行编译。([doc.dpdk.org][1])

---


### 3.2.4. 为新的 ARM SoC 构建 DPDK  
(Building DPDK for a New ARM SoC)

---

#### 3.2.4.1. ARM 的构建选项  
(Build options for ARM)

自 DPDK 25.03 起，ARM 平台的构建系统发生了变化，  
以提升 **清晰度、可用性以及性能优化能力**。

`-march` 选项用于定义 **通用架构**，  
而 `-mtune` 用于针对 **特定 CPU 优化性能**，  
但它 **不允许编译器假设可用的指令集**。

在 DPDK 25.03 之前，ARM 的构建流程混合使用了多种编译器参数  
（`-mcpu`、`-march` 和 `-mtune`），  
这可能会在无意中导致编译器回退到较旧的指令集，  
从而产生 **次优的性能结果**。

遵循 Arm 官方指南，推荐的做法是：  
**只要编译器支持目标 CPU，就应优先使用 `-mcpu` 参数**。

`-mcpu` 选项用于指定 **精确的 CPU 型号**，  
使编译器能够：

- 针对指定处理器优化代码生成
- 选择合适的指令集
- 明确地对性能特性进行微调

---

自 DPDK 25.03 起：

- 对于 **编译器 `-mcpu` 选项直接支持的 CPU**，  
  构建配置中会移除对 `-march` 及相关特性的引用，  
  以简化并改进构建配置。

- 对于 **编译器缺乏直接支持的 CPU**，  
  会使用 **伪 CPU（pseudo-CPU）定义**，  
  显式指定架构（`march`）和扩展（`march_extensions`），  
  以确保在不发生非预期降级的情况下获得最优性能。

- 当指定了 **不受支持的 `-mcpu`、`march` 或扩展** 时，  
  构建过程将 **显式失败**，  
  并提供解决问题的指导，  
  而不是无提示地回退到低性能架构。

---

#### 3.2.4.2. 为新的 SoC 添加支持  
(Adding Support for a New SoC)

如果需要为 **当前尚未支持的 ARM SoC** 构建 DPDK，  
请根据编译器支持情况，按照以下指南添加新的 SoC 支持。

---

##### 编译器 `-mcpu` 选项支持该 SoC

在 `config/arm/meson.build` 中的  
`part_number_config` 字典的相应位置，  
将 `mcpu` 设置为 **编译器 `-mcpu` 选项所支持的 SoC**。

以下示例展示了编译器支持 `-mcpu=foo` 的 SoC `foo`：

```python
'<Part_Number>': {
    'mcpu': 'foo',
    'flags': [
        ['RTE_MACHINE', '"Foo"'],
        # 根据需要添加其他标志
    ]
},
````

---

##### 编译器缺乏特定 `-mcpu` 支持或功能（需要使用伪 CPU）

如果编译器 **未能完全支持你的 SoC**，
请按以下步骤操作：

###### 1) 指定一个伪 CPU 名称

在 `config/arm/meson.build` 中的
`part_number_config` 字典中，
将 `mcpu` 设置为一个 **唯一的伪 CPU 名称**，
并使用 `mcpu_` 作为前缀。

该名称应能够清晰表示你的 SoC。
以下示例以 SoC `foo` 为例：

```python
'<Part_Number>': {
    'mcpu': 'mcpu_foo',
    'flags': [
        ['RTE_MACHINE', '"Foo"'],
        # 根据需要添加其他标志
    ]
},
```

---

###### 2) 定义伪 CPU 的详细信息

在 `mcpu_defs` 字典中，
添加该伪 CPU 的定义。

需要 **明确指定**：

* 架构（`march`）
* 编译器支持的扩展列表（`march_extensions`）

例如，`sve` 或 `crypto` 等扩展。

如果不需要特定扩展，可以将 `march_extensions` 留空。

```python
'mcpu_foo': {
    'march': 'armv8.x-a',
    'march_extensions': ['sve', 'crypto']
},
```

请将 `armv8.x-a` 及所列扩展替换为
与你的 SoC 实际 ISA 和特性相匹配的值。

---

##### 较旧的编译器不支持特定 `-mcpu`

如果使用的编译器较旧，且不支持所需的 CPU：

* 升级到支持目标 CPU 的新版本编译器

或者：

* 使用通用构建配置：

```bash
meson setup -Dplatform=generic build
```

通过遵循上述指南，
可以确保为基于 ARM 的 DPDK 目标生成 **最优性能的构建结果**。


---

### 3.2.5 Building Applications Using Installed DPDK

当 DPDK 在系统范围内安装之后，
会提供一个 `pkg-config` 文件 `libdpdk.pc` 供应用查询用于构建。
建议应用构建时**使用 pkg-config 文件**，而不是硬编码 DPDK 的编译器/链接器参数。([doc.dpdk.org][1])

示例 Makefile 配置片段如下：

```make
PKGCONF = pkg-config

CFLAGS += -O3 $(shell $(PKGCONF) --cflags libdpdk)
LDFLAGS += $(shell $(PKGCONF) --libs libdpdk)
$(APP): $(SRCS-y) Makefile
    $(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS)
```


[1]: https://doc.dpdk.org/guides/linux_gsg/build_dpdk.html "3. Compiling the DPDK Target from Source — Data Plane Development Kit 25.11.0 documentation"

