#include "stats.h"

#include <stdio.h>
#include <string.h>

void dp_stats_init(struct dp_stats *stats, uint32_t tenant_count) {
    memset(stats, 0, sizeof(*stats));
    for (uint32_t i = 0; i < tenant_count && i < DP_MAX_TENANTS; i++) {
        stats->tenants[i].packets = 0;
    }
}

static void dp_stats_update_topk(struct dp_stats *stats,
                                 const struct dp_flow_key *key,
                                 uint64_t bytes) {
    for (uint32_t i = 0; i < DP_MAX_TOPK; i++) {
        if (stats->topk_flows[i].packets == 0) {
            stats->topk_flows[i].key = *key;
            stats->topk_flows[i].packets = 1;
            stats->topk_flows[i].bytes = bytes;
            return;
        }
        if (memcmp(&stats->topk_flows[i].key, key, sizeof(*key)) == 0) {
            stats->topk_flows[i].packets++;
            stats->topk_flows[i].bytes += bytes;
            return;
        }
    }

    uint32_t min_index = 0;
    uint64_t min_packets = stats->topk_flows[0].packets;
    for (uint32_t i = 1; i < DP_MAX_TOPK; i++) {
        if (stats->topk_flows[i].packets < min_packets) {
            min_packets = stats->topk_flows[i].packets;
            min_index = i;
        }
    }

    if (bytes > stats->topk_flows[min_index].bytes) {
        stats->topk_flows[min_index].key = *key;
        stats->topk_flows[min_index].packets = 1;
        stats->topk_flows[min_index].bytes = bytes;
    }
}

void dp_stats_record(struct dp_stats *stats,
                     const struct dp_flow_key *key,
                     const struct dp_flow_action *action,
                     const struct dp_packet *packet) {
    if (action->tenant_id < DP_MAX_TENANTS) {
        stats->tenants[action->tenant_id].packets++;
        stats->tenants[action->tenant_id].bytes += packet->len;
    }

    dp_stats_update_topk(stats, key, packet->len);
}

void dp_stats_drop(struct dp_stats *stats, enum dp_drop_reason reason) {
    if (reason <= DP_DROP_RATE_LIMIT) {
        stats->drops[reason]++;
    }
}

void dp_stats_report(const struct dp_stats *stats) {
    printf("Drops: parse=%lu unmatched=%lu rate_limit=%lu\n",
           stats->drops[DP_DROP_PARSE_ERROR],
           stats->drops[DP_DROP_UNMATCHED],
           stats->drops[DP_DROP_RATE_LIMIT]);

    for (uint32_t i = 0; i < DP_MAX_TOPK; i++) {
        if (stats->topk_flows[i].packets == 0) {
            continue;
        }
        printf("TopK[%u]: packets=%lu bytes=%lu\n",
               i,
               stats->topk_flows[i].packets,
               stats->topk_flows[i].bytes);
    }
}
