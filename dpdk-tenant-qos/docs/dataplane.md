# Dataplane Details

## 5-Tuple Classification

The dataplane parses Ethernet/IPv4/UDP/TCP headers, constructs a 5-tuple key, and uses
an in-memory table for lookups. In a full DPDK implementation, this would be backed by
`rte_hash` or `rte_fib` for exact match at scale.

## Tenant/Policy QoS

Each tenant maps to a token bucket. Tokens are replenished using the TSC (time stamp
counter). Policy overrides can adjust `rate_limit_bps` on a per-flow basis. Packets that
exceed the available tokens are dropped and logged in statistics.

## Statistics and TopK

The stats module keeps per-tenant counters and maintains a small TopK list of hot
flows. In production, this should be replaced with a heavy-hitters algorithm (e.g.
SpaceSaving) to scale to millions of flows.
