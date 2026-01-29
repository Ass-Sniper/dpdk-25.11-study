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

## QEMU Two-Node Test (Ubuntu 20.04)

The following steps describe a minimal QEMU setup with two VMs connected by a virtual
bridge: one VM runs `pktgen` as a traffic generator, the other runs `dpdk-tenant-qos-v2`.
The steps assume you have DPDK 25.11 sources and `dpdk-pktgen` built for aarch64
inside the VMs.

### Host prerequisites (aarch64)

```bash
sudo apt-get update
sudo apt-get install -y qemu-system-arm bridge-utils cloud-image-utils
```

Create a Linux bridge for the two VMs:

```bash
sudo ip link add name br0 type bridge
sudo ip link set br0 up
```

### Prepare Ubuntu 20.04 cloud images (aarch64)

```bash
wget https://cloud-images.ubuntu.com/focal/current/focal-server-cloudimg-arm64.img
qemu-img create -f qcow2 -b focal-server-cloudimg-arm64.img vm-pktgen.qcow2 20G
qemu-img create -f qcow2 -b focal-server-cloudimg-arm64.img vm-dp.qcow2 20G
```

Create cloud-init user-data for each VM (passwordless sudo + SSH key):

```bash
cat <<'EOF' > user-data.yaml
#cloud-config
users:
  - name: ubuntu
    sudo: ALL=(ALL) NOPASSWD:ALL
    ssh_authorized_keys:
      - <your-ssh-public-key>
EOF
cloud-localds -v seed.img user-data.yaml
```

### Launch QEMU VMs (aarch64)

Run pktgen VM:

```bash
sudo qemu-system-aarch64 \\
  -machine virt,gic-version=3 -cpu cortex-a72 -smp 4 -m 4096 \\
  -bios /usr/share/AAVMF/AAVMF_CODE.fd \\
  -drive file=vm-pktgen.qcow2,if=virtio \\
  -drive file=seed.img,if=virtio \\
  -netdev bridge,id=net0,br=br0 \\
  -device virtio-net-pci,netdev=net0 \\
  -nographic
```

Run dataplane VM:

```bash
sudo qemu-system-aarch64 \\
  -machine virt,gic-version=3 -cpu cortex-a72 -smp 4 -m 4096 \\
  -bios /usr/share/AAVMF/AAVMF_CODE.fd \\
  -drive file=vm-dp.qcow2,if=virtio \\
  -drive file=seed.img,if=virtio \\
  -netdev bridge,id=net0,br=br0 \\
  -device virtio-net-pci,netdev=net0 \\
  -nographic
```

### Inside each VM

Install dependencies and build DPDK:

```bash
sudo apt-get update
sudo apt-get install -y build-essential meson ninja-build libnuma-dev pkg-config
git clone https://github.com/DPDK/dpdk.git
cd dpdk
git checkout v25.11
meson build
ninja -C build
sudo ninja -C build install
sudo ldconfig
```

Configure hugepages:

```bash
echo 1024 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
sudo mkdir -p /dev/hugepages
sudo mount -t hugetlbfs nodev /dev/hugepages
```

### pktgen VM

Install pktgen (example uses dpdk-pktgen):

```bash
git clone https://github.com/pktgen/Pktgen-DPDK.git
cd Pktgen-DPDK
make
```

Run pktgen (adjust core mask and port for virtio, aarch64 build output path may differ):

```bash
sudo ./app/app/aarch64-native-linux-gcc/pktgen \\
  -l 0-2 -n 4 -- \\
  -m \"[1:2].0\" \\
  -P
```

### dataplane VM

Build and run dpdk-tenant-qos-v2:

```bash
git clone <your-repo-url>
cd dpdk-tenant-qos-v2
make
python3 scripts/gen_rules.py --rules 1024 --tenants 8 --output rules.csv
sudo ./build/dpdk-tenant-qos-v2 -l 0-2 -n 4 -- \\
  --portmask 0x1 --tenants 8 --rules 4096 --rules-file rules.csv --stats-interval 5
```

### Validation

- Use pktgen to transmit packets to the dataplane VM.
- Observe periodic stats output from `dpdk-tenant-qos-v2` (drops + TopK).
- Capture throughput using pktgen counters and compare with expected tenant limits.
