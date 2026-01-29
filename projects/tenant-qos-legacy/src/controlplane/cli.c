#include "cli.h"

#include <stdio.h>

static void dp_cli_load_defaults(struct dp_context *context) {
    struct dp_flow_key key = {
        .src_ip = 0x0a000001,
        .dst_ip = 0x0a000002,
        .src_port = 1234,
        .dst_port = 4321,
        .protocol = 17
    };
    struct dp_flow_action action = {
        .tenant_id = 0,
        .policy_id = 1,
        .rate_limit_bps = 500000000ULL
    };

    dp_classify_add(&context->classifier, &key, &action);
}

void dp_cli_init(struct dp_context *context, const struct dp_config *config) {
    (void)config;
    printf("Control-plane CLI initialized (stub)\n");
    dp_cli_load_defaults(context);
}
