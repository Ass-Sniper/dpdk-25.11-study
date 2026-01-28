# dpdk-tenant-qos-v2 Architecture

This implementation follows patterns from DPDK 25.11 examples (e.g., `l2fwd`, `l3fwd`,
`ip_pipeline`) to build a tenant-aware QoS dataplane.

## Components

- **Ingress Parsing**: Parse Ethernet/IPv4/TCP/UDP headers and extract 5-tuple keys.
- **Classifier**: Use `rte_hash` for exact-match 5-tuple lookups and map flows to tenant/policy.
- **QoS Shaper**: Token-bucket per tenant with policy-level overrides.
- **Statistics**: Per-tenant counters, drop reasons, and TopK flow/tenant visibility.
- **Control Plane**: CSV rule loader + runtime flags for deployment.

## Packet Walk

1. RX burst via `rte_eth_rx_burst`.
2. Parse 5-tuple.
3. Lookup rule in `rte_hash`.
4. Apply tenant/policy QoS.
5. Update stats and report TopK.
