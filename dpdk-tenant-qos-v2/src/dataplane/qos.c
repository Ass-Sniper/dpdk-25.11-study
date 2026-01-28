#include "qos.h"

#include <string.h>

#include <rte_cycles.h>

static uint64_t dp_qos_rate_to_tokens(uint64_t rate_bps, uint64_t tsc_hz, uint64_t delta_tsc) {
    uint64_t bytes_per_sec = rate_bps / 8;
    return (bytes_per_sec * delta_tsc) / tsc_hz;
}

int dp_qos_init(struct dp_qos *qos, uint32_t tenant_count, uint64_t tsc_hz) {
    memset(qos, 0, sizeof(*qos));
    qos->tenant_count = tenant_count;
    qos->tsc_hz = tsc_hz;

    for (uint32_t i = 0; i < tenant_count && i < DP_MAX_TENANTS; i++) {
        qos->tenants[i].rate_limit_bps = 1000000000ULL;
        qos->tenants[i].tokens_bytes = qos->tenants[i].rate_limit_bps / 8;
    }

    return 0;
}

void dp_qos_update_policy(struct dp_qos *qos, uint32_t policy_id, uint64_t rate_bps) {
    if (policy_id >= DP_MAX_POLICIES) {
        return;
    }

    qos->policies[policy_id].policy_id = policy_id;
    qos->policies[policy_id].rate_limit_bps = rate_bps;
    if (policy_id >= qos->policy_count) {
        qos->policy_count = policy_id + 1;
    }
}

int dp_qos_apply(struct dp_qos *qos, const struct dp_flow_action *action, uint32_t pkt_len) {
    if (action->tenant_id >= qos->tenant_count) {
        return -1;
    }

    struct dp_qos_bucket *bucket = &qos->tenants[action->tenant_id];
    uint64_t now = rte_rdtsc();
    uint64_t delta = now - bucket->last_tsc;

    if (bucket->last_tsc != 0) {
        bucket->tokens_bytes += dp_qos_rate_to_tokens(bucket->rate_limit_bps,
                                                      qos->tsc_hz,
                                                      delta);
    }

    bucket->last_tsc = now;

    uint64_t policy_rate = 0;
    if (action->policy_id < qos->policy_count) {
        policy_rate = qos->policies[action->policy_id].rate_limit_bps;
    }

    uint64_t effective_rate = action->rate_limit_bps != 0 ? action->rate_limit_bps :
                              (policy_rate != 0 ? policy_rate : bucket->rate_limit_bps);

    bucket->rate_limit_bps = effective_rate;

    if (bucket->tokens_bytes < pkt_len) {
        return -1;
    }

    bucket->tokens_bytes -= pkt_len;
    return 0;
}
