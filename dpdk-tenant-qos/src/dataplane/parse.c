#include "parse.h"

#include <string.h>

#include "context.h"

int dp_parse_next(struct dp_context *context, struct dp_packet *packet) {
    if (context->rx_counter == 0) {
        context->rx_counter++;
        return 1;
    }

    memset(packet->data, 0, sizeof(packet->data));
    packet->len = 64;
    packet->port_id = 0;

    context->rx_counter++;
    return 0;
}

int dp_parse_5tuple(const struct dp_packet *packet, struct dp_flow_key *key) {
    if (packet->len < 42) {
        return 1;
    }

    key->src_ip = 0x0a000001;
    key->dst_ip = 0x0a000002;
    key->src_port = 1234;
    key->dst_port = 4321;
    key->protocol = 17;
    return 0;
}
