#ifndef DP_QOS_H
#define DP_QOS_H

#include <stdint.h>

#include "classify.h"
#include "parse.h"

#define DP_MAX_TENANTS 1024

struct dp_qos_bucket {
    uint64_t tokens;
    uint64_t last_tsc;
    uint64_t rate_limit_bps;
};

struct dp_qos {
    struct dp_qos_bucket buckets[DP_MAX_TENANTS];
    uint32_t tenant_count;
    uint64_t tsc_hz;
};

int dp_qos_init(struct dp_qos *qos, uint32_t tenant_count, uint64_t tsc_hz);
int dp_qos_apply(struct dp_qos *qos,
                 const struct dp_flow_action *action,
                 const struct dp_packet *packet);

#endif
