#ifndef DP_QOS_H
#define DP_QOS_H

#include <stdint.h>

#include "classify.h"

#define DP_MAX_TENANTS 4096
#define DP_MAX_POLICIES 16384

struct dp_qos_bucket {
    uint64_t tokens_bytes;
    uint64_t last_tsc;
    uint64_t rate_limit_bps;
};

struct dp_qos_policy {
    uint32_t policy_id;
    uint64_t rate_limit_bps;
};

struct dp_qos {
    struct dp_qos_bucket tenants[DP_MAX_TENANTS];
    struct dp_qos_policy policies[DP_MAX_POLICIES];
    uint32_t tenant_count;
    uint32_t policy_count;
    uint64_t tsc_hz;
};

int dp_qos_init(struct dp_qos *qos, uint32_t tenant_count, uint64_t tsc_hz);
void dp_qos_update_policy(struct dp_qos *qos, uint32_t policy_id, uint64_t rate_bps);
int dp_qos_apply(struct dp_qos *qos, const struct dp_flow_action *action, uint32_t pkt_len);

#endif
