# dpdk-tenant-qos Architecture

This reference implementation targets DPDK 25.11 and focuses on tenant-aware QoS on the
fast path.

## Components

- **Ingress Parsing**: Extracts the 5-tuple (IPv4 + L4 ports + protocol) from packets.
- **Classifier**: Matches the 5-tuple against a rule table and returns tenant + policy.
- **QoS Shaper**: Applies token-bucket policing per tenant or per policy.
- **Statistics**: Maintains real-time counters, drop reasons, and TopK flows/tenants.
- **Control Plane**: Loads configuration, rules, and provides a CLI hook for updates.

## Data Flow

1. RX burst is pulled from DPDK ports.
2. Packet is parsed into a 5-tuple key.
3. Classifier returns the matching tenant/policy action.
4. QoS checks tokens and decides pass/drop.
5. Statistics update per-flow and per-tenant counters.
