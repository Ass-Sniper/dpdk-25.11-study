
# 1. Introduction  
(介绍)

本文档包含了**安装和配置 Data Plane Development Kit (DPDK) 软件的说明**。  
其目的在于让用户**快速启动并运行 DPDK 软件**。  
本指南描述了如何在 Linux 应用环境（linux）中**编译并运行 DPDK 应用程序**，但不会深入细节。

---

## 1.1 Documentation Roadmap  
(文档阅读顺序指南)

以下是建议的 **DPDK 文档阅读顺序** 列表：

- [**Release Notes**](https://doc.dpdk.org/guides/rel_notes/index.html)：提供发布版本特定的信息，包括支持的功能、限制、修复的问题、已知问题等。同时还以 FAQ 形式给出常见问题解答。

- [**Getting Started Guide for Linux**](https://doc.dpdk.org/guides/linux_gsg/index.html)（本指南）：描述如何安装和配置 DPDK；旨在让用户快速上手。

- [**Programmer’s Guide**](https://doc.dpdk.org/guides/prog\_guide/index.htmlhttps://doc.dpdk.org/guides/prog_guide/index.html)：描述：
  * 软件架构以及如何使用 DPDK（通过示例），特别是在 Linux 应用（linux）环境中使用；
  * DPDK 的内容、构建系统（包括构建开发套件和应用的命令）以及移植应用的指南；
  * 软件内部使用的优化以及新开发中应考虑的优化。

  同时还提供了术语表。

- [**API Reference**](https://doc.dpdk.org/api/html/index.html)：提供 DPDK 函数、数据结构和其他编程结构的详细信息。

- [**Sample Applications User Guides**](https://doc.dpdk.org/guides/sample_app_ug/index.html)：描述一组示例应用程序，每章描述一个示例应用程序，展示特定功能，并提供如何编译、运行和使用该示例应用程序的说明。

- **Driver Reference Guides**：提供各类别驱动程序的详细信息。对于每类驱动程序都有单独的指南，包括：
  * Baseband Device Drivers
  * Compression Device Drivers  
  * [Crypto Device Drivers](https://doc.dpdk.org/guides/cryptodevs/index.html)  
  * [DMA Device Drivers](https://doc.dpdk.org/guides/dmadevs/index.html)  
  * [Event Device Drivers](https://doc.dpdk.org/guides/eventdevs/index.html)  
  * General-Purpose GPU Drivers  
  * [Mempool Device Driver](https://doc.dpdk.org/guides/mempool/index.html)  
  * [Network Interface Controller Drivers](https://doc.dpdk.org/guides/nics/index.html)  
  * Rawdev Drivers  
  * [REGEX Device Drivers](https://doc.dpdk.org/guides/regexdevs/index.html)  
  * [vDPA Device Drivers](https://doc.dpdk.org/guides/vdpadevs/index.html) 
  * Platform Specific Guides  
  *(以上条目均来自官方文档内容)* 

