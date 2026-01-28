#ifndef DP_PARSE_H
#define DP_PARSE_H

#include <stdint.h>

#include <rte_mbuf.h>

struct dp_flow_key {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
} __attribute__((packed));

int dp_parse_5tuple(const struct rte_mbuf *mbuf, struct dp_flow_key *key);

#endif
