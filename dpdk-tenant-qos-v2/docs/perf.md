# Performance Checklist

## Build

- Enable `-O3` and `-march=native`.
- Ensure hugepages are configured for DPDK.

## Runtime

- Pin RX/TX lcores and match NIC NUMA node.
- Increase RX/TX descriptors for high throughput.
- Use `scripts/gen_rules.py` to generate realistic 5-tuple rules.

## Measurements

- Throughput per tenant/policy.
- Drop reasons (parse, unmatched, rate-limit).
- TopK flows and tenants for hot-spot analysis.
