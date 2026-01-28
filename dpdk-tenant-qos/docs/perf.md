# Performance Notes

## Target

- DPDK 25.11
- Multi-queue RX/TX
- NUMA-aware mempools

## Benchmark Checklist

1. Build with `-O3` and enable `-march=native`.
2. Pin lcores to NIC NUMA node.
3. Enable hardware RX offloads for checksum parsing.
4. Use `scripts/perf_test.sh` to generate rules and drive a traffic generator.
5. Capture per-tenant throughput, drops, and latency.

## Observability

TopK flows and tenant stats can be exported via telemetry or a metrics sidecar. Use
DPDK telemetry or shared memory to expose real-time counters to the control plane.
