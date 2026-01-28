# Dataplane Details

## 5-Tuple Classification

The parser mirrors DPDK example usage of `rte_mbuf`, `rte_ether_hdr`, and `rte_ipv4_hdr` to
extract the 5-tuple (src/dst IP, src/dst port, protocol). The flow key feeds `rte_hash`
for O(1) lookups.

## Tenant/Policy QoS

Each tenant maintains a token-bucket with rate in bps; policies can override the tenant
rate. Tokens are replenished via `rte_rdtsc()` and `rte_get_tsc_hz()`.

## Observability

`dp_stats` tracks per-tenant counters, drop reasons, and TopK heavy hitters for both
flows and tenants. Reporting can be periodic (`--stats-interval`) or final on exit.
