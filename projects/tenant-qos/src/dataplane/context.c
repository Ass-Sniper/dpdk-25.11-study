#include "context.h"

#include <string.h>

#include <rte_ethdev.h>
#include <rte_malloc.h>

#include "controlplane/config.h"

int dp_context_init(struct dp_context *context, const struct dp_config *config) {
    memset(context, 0, sizeof(*context));

    uint16_t port_id;
    RTE_ETH_FOREACH_DEV(port_id) {
        if ((config->portmask & (1u << port_id)) == 0) {
            continue;
        }
        context->ports[context->nb_ports++] = port_id;
    }

    if (context->nb_ports == 0) {
        return -1;
    }

    context->mbuf_pool = rte_pktmbuf_pool_create("dp_mbuf_pool",
                                                 config->mbuf_count,
                                                 config->mbuf_cache_size,
                                                 0,
                                                 RTE_MBUF_DEFAULT_BUF_SIZE,
                                                 rte_socket_id());
    if (context->mbuf_pool == NULL) {
        return -1;
    }

    if (dp_classify_init(&context->classifier, config->max_rules) != 0) {
        return -1;
    }

    if (dp_qos_init(&context->qos, config->tenant_count, config->tsc_hz) != 0) {
        return -1;
    }

    dp_stats_init(&context->stats, config->tenant_count, config->report_interval_tsc);
    return 0;
}

void dp_context_close(struct dp_context *context) {
    dp_classify_close(&context->classifier);
    context->mbuf_pool = NULL;
}
