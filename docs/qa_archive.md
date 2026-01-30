# DPDK Graph 相关问答归档

> 归档范围：本轮对话中围绕 DPDK Graph/L3FWD-Graph 的全部问答内容。

## 1. Graph 机制数据处理流程 + Mermaid 流程图（含文件名/函数名）

以下流程图覆盖 **节点注册 → 图构建/校验 → 快路径内存构建 → 运行时 walk/处理/入队** 的关键路径。

```mermaid
flowchart TD
  A["节点注册\nnode.c::__rte_node_register"] --> B["图创建入口\ngraph.c::rte_graph_create"]
  B --> C["按模式加入节点\ngraph.c::expand_pattern_to_node"]
  C --> D["补齐边并加入节点\ngraph.c::graph_node_edges_add"]
  D --> E["构建邻接表\ngraph.c::graph_adjacency_list_update"]
  E --> F["图一致性检查\ngraph.c::graph_src_nodes_count / graph_has_isolated_node 等"]
  F --> G["分配/构建快路径内存\ngraph_populate.c::graph_fp_mem_create"]
  G --> H["填充图头与节点布局\ngraph_populate.c::graph_header_popluate / graph_nodes_populate"]
  H --> I["解析边到指针 & 源节点入队\ngraph_populate.c::graph_node_nexts_populate / graph_src_nodes_offset_populate"]
  I --> J["节点 init 回调\ngraph.c::graph_node_init"]

  J --> K["运行时主循环调用\nexamples/l3fwd-graph/main.c::graph_main_loop"]
  K --> L["图遍历入口\nrte_graph_worker.h::rte_graph_walk"]
  L --> M["RTC 模型遍历\nrte_graph_model_rtc.h::rte_graph_walk_rtc"]
  M --> N["节点处理\nrte_graph_worker_common.h::__rte_node_process -> node->process"]
  N --> O["将对象入队到后继节点\nrte_graph_worker_common.h::rte_node_enqueue / rte_node_enqueue_x1/x2/x4"]
  O --> P["更新待处理环形队列\nrte_graph_worker_common.h::__rte_node_enqueue_tail_update"]
  P --> M
```

---

## 2. 核心数据结构与图论概念对应

| 数据结构 | 位置 | 图论对应 | 要点 |
|---|---|---|---|
| `struct node` | `graph_private.h` | 顶点（vertex） | 注册态节点，含 `name/nb_edges/next_nodes[]`。
| `struct graph_node` | `graph_private.h` | 顶点 + 邻接表 | `adjacency_list[]` 为邻接表。
| `struct graph` | `graph_private.h` | 图（graph） | 记录节点列表、源节点数量、内存布局等。
| `struct rte_graph` | `rte_graph_worker_common.h` | 图的运行时状态 | 环形队列 `head/tail/cir_start` 负责调度。
| `struct rte_node` | `rte_graph_worker_common.h` | 顶点（运行态） | `nodes[]` 为下一跳节点指针数组。
| `rte_node_enqueue*` | `rte_graph_worker_common.h` | 沿边传播 | 将对象流送入后继节点并推进调度。

---

## 3. Graph 机制实际应用价值与落地项目概览

### 3.1 实际应用价值
- 可将包处理逻辑拆解为可复用节点，并以图形式拼装。
- 快路径布局与环形调度提升吞吐、降低调度开销。
- 支持 RTC 与跨核调度模型，适合多核场景。

### 3.2 公开资料项目（含链接）
- **Open vSwitch with DPDK (OVS-DPDK)**
  - 关联点：OVS 可使用 DPDK datapath 完全用户态运行，适合作为 Graph 节点化数据面的集成场景。
  - 文档：https://docs.openvswitch.org/en/latest/intro/install/dpdk/
- **DPDK Graph Library 文档**
  - 关联点：官方 Graph Library 设计与内建节点说明。
  - 文档：https://doc.dpdk.org/guides/prog_guide/graph_lib.html

---

## 4. 按模式加入节点：支持的模式类型、区别与适用场景

- **只支持 Shell Pattern（`fnmatch` 的 shell 风格通配符）**。
- 模式差异主要体现在是否使用通配符：
  - **精确匹配**：不含通配符，等价于严格字符串匹配。
  - **通配匹配**：含通配符，适合批量匹配一类节点。

---

## 5. `examples/l3fwd-graph` 数据包转发流程图（含用户空间/内核空间、模块/机制）

> 该示例以 DPDK 用户态数据面为主；RX/TX 通过 DPDK 驱动访问 NIC 队列。硬件不支持 ptype 时会启用软件回调做补齐。

```mermaid
flowchart TD
  subgraph UserSpace["用户空间 (DPDK)"]
    A["EAL 初始化 / 端口&队列配置\nmain.c: rte_eal_init / rte_eth_dev_configure / rte_eth_rx_queue_setup / rte_eth_tx_queue_setup"]
    B["Graph 创建 + 节点模式配置\nmain.c: graph_config_rtc / graph_config_mcore_dispatch"]
    C["Graph 运行主循环\nmain.c: graph_main_loop -> rte_graph_walk"]

    D["收包节点 (源节点)\nethdev_rx.c: ethdev_rx_node_process_inline -> rte_eth_rx_burst"]
    E["分类/路由查表\nip4_lookup.c: ip4_lookup_node_process_scalar -> rte_lpm_lookup"]
    F["重写节点(下一跳/L2改写)\nip4_rewrite.c: __ip4_rewrite_node_process"]
    G["发包节点\nethdev_tx.c: ethdev_tx_node_process -> rte_eth_tx_burst"]
    H["丢包节点\npkt_drop.c: pkt_drop_process"]
  end

  subgraph Hardware["硬件 (NIC)"]
    RX["RX 队列 + DMA"]
    TX["TX 队列 + DMA"]
  end

  A --> B --> C
  RX --> D
  D --> E
  E --> F
  F --> G --> TX
  E --> H
  G --> H

  note1["软硬件协同: ptype 不支持 -> software callback\nethdev_rx.c: ethdev_ptype_setup -> rte_eth_add_rx_callback"]:::note
  D -.-> note1
classDef note fill:#f5f5f5,stroke:#888,stroke-width:1px;
```

---

## 6. 完整数据包转发时序图（Mermaid，含文件名/函数名）

```mermaid
sequenceDiagram
  participant App as App(main.c)
  participant Graph as Graph Engine
  participant RxNode as ethdev_rx.c
  participant Lookup as ip4_lookup.c
  participant Rewrite as ip4_rewrite.c
  participant TxNode as ethdev_tx.c
  participant NIC as NIC(HW)

  App->>App: rte_eal_init() / 端口队列配置\nmain.c
  App->>App: rte_eth_dev_configure / rte_eth_rx_queue_setup / rte_eth_tx_queue_setup\nmain.c
  App->>App: rte_node_eth_config()\nmain.c
  App->>Graph: graph_config_rtc()/graph_config_mcore_dispatch()\nmain.c
  App->>Graph: rte_graph_create() / rte_graph_lookup()\nmain.c
  App->>Graph: rte_node_ip4_route_add() / rte_node_ip4_rewrite_add()\nmain.c
  App->>Graph: rte_eal_mp_remote_launch(graph_main_loop)\nmain.c

  loop 每个 worker lcore
    App->>Graph: graph_main_loop -> rte_graph_walk()\nmain.c
    Graph->>RxNode: ethdev_rx_node_process_inline()\nethdev_rx.c
    RxNode->>NIC: rte_eth_rx_burst()\nethdev_rx.c
    RxNode-->>Graph: enqueue next (pkt_cls / ip4_lookup)\nethdev_rx.c
    Graph->>Lookup: ip4_lookup_node_process_scalar()\nip4_lookup.c
    Lookup->>Lookup: rte_lpm_lookup()\nip4_lookup.c
    Lookup-->>Graph: enqueue next (rewrite / drop)\nip4_lookup.c
    Graph->>Rewrite: __ip4_rewrite_node_process()\nip4_rewrite.c
    Rewrite-->>Graph: next -> ethdev_tx-*\nip4_rewrite.c
    Graph->>TxNode: ethdev_tx_node_process()\nethdev_tx.c
    TxNode->>NIC: rte_eth_tx_burst()\nethdev_tx.c
  end
```

---

## 7. Graph walk 支持的模型遍历、差异与场景

- **RTC（Run-To-Completion）**：默认模型，单核内以环形队列调度节点，适合单核/低调度开销场景。
- **Mcore Dispatch**：跨核调度模型，依赖工作队列与内存池在多核之间分发节点流，适合多核负载均衡与跨核处理。

---

## 8. 公开资料清单（公司/组织 → 产品/项目 → DPDK/Graph 关联点）

| 公司/组织 | 产品/项目 | DPDK/Graph 关联点 | 外部来源 |
|---|---|---|---|
| Open vSwitch 社区 | Open vSwitch with DPDK | OVS 使用 DPDK datapath 完全用户态运行，适合 Graph 化数据面集成 | https://docs.openvswitch.org/en/latest/intro/install/dpdk/ |
| Linux Foundation DPDK 项目 | DPDK Graph Library 文档 | 官方 Graph library 机制/内建节点说明 | https://doc.dpdk.org/guides/prog_guide/graph_lib.html |

---

## 9. Graph 创建时节点模式支持说明（复述汇总）

- Graph 按模式加入节点只支持 **shell pattern**（`fnmatch` 风格通配符）。
- 常见用法：
  - 精确匹配：`"ethdev_tx-0"`
  - 通配匹配：`"ip4*"`、`"ethdev_tx-*"`

---

## 10. 备注

- 上述归档内容来源于对源码与公开文档的逐条分析。
- 如需扩展更多公司/产品/项目清单，请提供具体名单或方向。

