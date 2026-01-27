#ifndef DP_STATS_H
#define DP_STATS_H

#include <stdint.h>

#include "classify.h"
#include "parse.h"
#include "qos.h"

#define DP_MAX_TOPK 10

enum dp_drop_reason {
    DP_DROP_PARSE_ERROR = 0,
    DP_DROP_UNMATCHED = 1,
    DP_DROP_RATE_LIMIT = 2
};

struct dp_flow_stat {
    struct dp_flow_key key;
    uint64_t packets;
    uint64_t bytes;
};

struct dp_tenant_stat {
    uint64_t packets;
    uint64_t bytes;
    uint64_t drops;
};

struct dp_stats {
    struct dp_flow_stat topk_flows[DP_MAX_TOPK];
    struct dp_tenant_stat tenants[DP_MAX_TENANTS];
    uint64_t drops[3];
};

void dp_stats_init(struct dp_stats *stats, uint32_t tenant_count);
void dp_stats_record(struct dp_stats *stats,
                     const struct dp_flow_key *key,
                     const struct dp_flow_action *action,
                     const struct dp_packet *packet);
void dp_stats_drop(struct dp_stats *stats, enum dp_drop_reason reason);
void dp_stats_report(const struct dp_stats *stats);

#endif
