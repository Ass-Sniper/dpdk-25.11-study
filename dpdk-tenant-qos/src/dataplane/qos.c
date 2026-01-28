#include "qos.h"

#include <string.h>

static uint64_t dp_qos_now_tsc(void) {
    static uint64_t tsc = 0;
    tsc += 100;
    return tsc;
}

int dp_qos_init(struct dp_qos *qos, uint32_t tenant_count, uint64_t tsc_hz) {
    memset(qos, 0, sizeof(*qos));
    qos->tenant_count = tenant_count;
    qos->tsc_hz = tsc_hz;

    for (uint32_t i = 0; i < tenant_count; i++) {
        qos->buckets[i].rate_limit_bps = 1000000000ULL;
        qos->buckets[i].tokens = qos->buckets[i].rate_limit_bps / 8;
    }
    return 0;
}

int dp_qos_apply(struct dp_qos *qos,
                 const struct dp_flow_action *action,
                 const struct dp_packet *packet) {
    if (action->tenant_id >= qos->tenant_count) {
        return 1;
    }

    struct dp_qos_bucket *bucket = &qos->buckets[action->tenant_id];
    uint64_t now = dp_qos_now_tsc();
    uint64_t delta = now - bucket->last_tsc;
    bucket->last_tsc = now;

    uint64_t add_tokens = (delta * bucket->rate_limit_bps) / qos->tsc_hz / 8;
    bucket->tokens += add_tokens;

    if (action->rate_limit_bps != 0) {
        bucket->rate_limit_bps = action->rate_limit_bps;
    }

    uint64_t needed = packet->len;
    if (bucket->tokens < needed) {
        return 1;
    }

    bucket->tokens -= needed;
    return 0;
}
