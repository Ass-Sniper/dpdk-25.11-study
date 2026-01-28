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
<<<<<<< ours
=======

## Local Script Checks (No DPDK Runtime)

The following checks validate rule generation and perf workflow scaffolding without
needing a NIC or DPDK runtime:

```bash
python3 scripts/gen_rules.py --rules 4 --tenants 2 --output rules.sample.csv
```

Result (file created):

- `rules.sample.csv` generated with 4 rules.

```bash
scripts/perf_test.sh
```

Result:

```
Generated rules: /workspace/dpdk-25.11-study/dpdk-tenant-qos-v2/rules.csv
Suggested perf workflow:
  1) Build dpdk-tenant-qos-v2 with DPDK 25.11
  2) Run scripts/run.sh with appropriate EAL flags
  3) Use a traffic generator (pktgen, moongen, trex) to push flows
  4) Capture throughput, drops, and TopK stats output
```
>>>>>>> theirs
