#!/usr/bin/env python3
import argparse
import ipaddress
import random


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--rules", type=int, default=16)
    parser.add_argument("--tenants", type=int, default=4)
    args = parser.parse_args()

    base_src = ipaddress.ip_address("10.0.0.1")
    base_dst = ipaddress.ip_address("10.0.1.1")

    print("# src_ip,dst_ip,src_port,dst_port,proto,tenant_id,policy_id,rate_bps")
    for rule_id in range(args.rules):
        src_ip = base_src + rule_id
        dst_ip = base_dst + rule_id
        src_port = 10000 + rule_id
        dst_port = 20000 + rule_id
        proto = random.choice([6, 17])
        tenant = rule_id % args.tenants
        policy_id = rule_id
        rate = 1000000000 // max(args.tenants, 1)
        print(f"{src_ip},{dst_ip},{src_port},{dst_port},{proto},{tenant},{policy_id},{rate}")


if __name__ == "__main__":
    main()
