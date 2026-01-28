#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataplane/classify.h"
#include "dataplane/context.h"
#include "dataplane/parse.h"
#include "dataplane/qos.h"
#include "dataplane/stats.h"
#include "controlplane/cli.h"
#include "controlplane/config.h"

int main(int argc, char **argv) {
    struct dp_config config;
    struct dp_context context;

    if (dp_config_load(argc, argv, &config) != 0) {
        fprintf(stderr, "failed to load config\n");
        return 1;
    }

    if (dp_context_init(&context, &config) != 0) {
        fprintf(stderr, "failed to init dataplane context\n");
        return 1;
    }

    dp_cli_init(&context, &config);

    printf("Starting dataplane with %u tenants\n", config.tenant_count);

    while (!context.shutdown) {
        struct dp_packet packet;

        if (dp_parse_next(&context, &packet) != 0) {
            continue;
        }

        struct dp_flow_key key;
        if (dp_parse_5tuple(&packet, &key) != 0) {
            dp_stats_drop(&context.stats, DP_DROP_PARSE_ERROR);
            continue;
        }

        struct dp_flow_action action;
        if (dp_classify_lookup(&context.classifier, &key, &action) != 0) {
            dp_stats_drop(&context.stats, DP_DROP_UNMATCHED);
            continue;
        }

        if (dp_qos_apply(&context.qos, &action, &packet) != 0) {
            dp_stats_drop(&context.stats, DP_DROP_RATE_LIMIT);
            continue;
        }

        dp_stats_record(&context.stats, &key, &action, &packet);
    }

    dp_stats_report(&context.stats);
    dp_context_shutdown(&context);
    return 0;
}
