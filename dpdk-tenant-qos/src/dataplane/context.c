#include "context.h"

#include <string.h>

#include "controlplane/config.h"

int dp_context_init(struct dp_context *context, const struct dp_config *config) {
    memset(context, 0, sizeof(*context));
    context->shutdown = 0;
    context->rx_counter = 0;

    if (dp_classify_init(&context->classifier) != 0) {
        return 1;
    }

    if (dp_qos_init(&context->qos, config->tenant_count, config->tsc_hz) != 0) {
        return 1;
    }

    dp_stats_init(&context->stats, config->tenant_count);
    return 0;
}

void dp_context_shutdown(struct dp_context *context) {
    context->shutdown = 1;
}
