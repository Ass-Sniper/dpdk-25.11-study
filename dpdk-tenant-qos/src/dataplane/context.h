#ifndef DP_CONTEXT_H
#define DP_CONTEXT_H

#include <stdint.h>

#include "classify.h"
#include "qos.h"
#include "stats.h"

struct dp_config;

struct dp_context {
    struct dp_classifier classifier;
    struct dp_qos qos;
    struct dp_stats stats;
    uint8_t shutdown;
    uint64_t rx_counter;
};

int dp_context_init(struct dp_context *context, const struct dp_config *config);
void dp_context_shutdown(struct dp_context *context);

#endif
