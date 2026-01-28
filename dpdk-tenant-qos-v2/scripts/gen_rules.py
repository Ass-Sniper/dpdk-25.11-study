#!/usr/bin/env python3
import argparse
import ipaddress
import random


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate CSV rules for dpdk-tenant-qos-v2")
    parser.add_argument("--rules", type=int, default=1024)
    parser.add_argument("--tenants", type=int, default=8)
    parser.add_argument("--output", default="rules.csv")
    args = parser.parse_args()

    base_src = ipaddress.ip_address("10.0.0.1")
    base_dst = ipaddress.ip_address("10.1.0.1")

    with open(args.output, "w", encoding="utf-8") as handle:
        handle.write("# src_ip,dst_ip,src_port,dst_port,proto,tenant_id,policy_id,rate_bps\n")
        for i in range(args.rules):
            src_ip = base_src + i
            dst_ip = base_dst + i
            src_port = 10000 + i
            dst_port = 20000 + i
            proto = random.choice([6, 17])
            tenant = i % args.tenants
            policy = i % (args.tenants * 4)
            rate = max(100000000, 1000000000 // max(args.tenants, 1))
            handle.write(f"{src_ip},{dst_ip},{src_port},{dst_port},{proto},{tenant},{policy},{rate}\n")


if __name__ == "__main__":
    main()
