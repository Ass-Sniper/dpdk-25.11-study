#ifndef DP_PARSE_H
#define DP_PARSE_H

#include <stdint.h>

#define DP_MAX_PACKET_LEN 2048

struct dp_packet {
    uint8_t data[DP_MAX_PACKET_LEN];
    uint16_t len;
    uint16_t port_id;
};

struct dp_flow_key {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
};

struct dp_context;

int dp_parse_next(struct dp_context *context, struct dp_packet *packet);
int dp_parse_5tuple(const struct dp_packet *packet, struct dp_flow_key *key);

#endif
