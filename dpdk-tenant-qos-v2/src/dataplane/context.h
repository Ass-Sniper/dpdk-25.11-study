#ifndef DP_CONTEXT_H
#define DP_CONTEXT_H

#include <stdint.h>

#include <rte_mempool.h>

#include "classify.h"
#include "qos.h"
#include "stats.h"

struct dp_config;

struct dp_context {
    struct dp_classifier classifier;
    struct dp_qos qos;
    struct dp_stats stats;
    struct rte_mempool *mbuf_pool;
    uint16_t ports[RTE_MAX_ETHPORTS];
    uint16_t nb_ports;
};

int dp_context_init(struct dp_context *context, const struct dp_config *config);
void dp_context_close(struct dp_context *context);

#endif
