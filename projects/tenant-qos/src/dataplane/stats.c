#include "stats.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static void dp_stats_update_topk(struct dp_flow_stat *topk,
                                 const struct dp_flow_key *key,
                                 uint64_t packets,
                                 uint64_t bytes) {
    for (uint32_t i = 0; i < DP_MAX_TOPK; i++) {
        if (topk[i].packets == 0) {
            topk[i].key = *key;
            topk[i].packets = packets;
            topk[i].bytes = bytes;
            return;
        }

        if (memcmp(&topk[i].key, key, sizeof(*key)) == 0) {
            topk[i].packets += packets;
            topk[i].bytes += bytes;
            return;
        }
    }

    uint32_t min_idx = 0;
    uint64_t min_packets = topk[0].packets;
    for (uint32_t i = 1; i < DP_MAX_TOPK; i++) {
        if (topk[i].packets < min_packets) {
            min_packets = topk[i].packets;
            min_idx = i;
        }
    }

    if (packets > min_packets) {
        topk[min_idx].key = *key;
        topk[min_idx].packets = packets;
        topk[min_idx].bytes = bytes;
    }
}

void dp_stats_init(struct dp_stats *stats, uint32_t tenant_count, uint64_t report_interval_tsc) {
    memset(stats, 0, sizeof(*stats));
    stats->report_interval_tsc = report_interval_tsc;
    stats->last_report_tsc = 0;
    rte_spinlock_init(&stats->lock);

    for (uint32_t i = 0; i < tenant_count && i < DP_MAX_TENANTS; i++) {
        stats->tenants[i].packets = 0;
    }
}

void dp_stats_record(struct dp_stats *stats,
                     const struct dp_flow_key *key,
                     const struct dp_flow_action *action,
                     uint64_t packets,
                     uint64_t bytes) {
    if (action->tenant_id >= DP_MAX_TENANTS) {
        return;
    }

    rte_spinlock_lock(&stats->lock);
    stats->tenants[action->tenant_id].packets += packets;
    stats->tenants[action->tenant_id].bytes += bytes;
    dp_stats_update_topk(stats->topk_flows, key, packets, bytes);

    struct dp_flow_key tenant_key = {
        .src_ip = action->tenant_id,
        .dst_ip = 0,
        .src_port = 0,
        .dst_port = 0,
        .protocol = 0,
    };
    dp_stats_update_topk(stats->topk_tenants, &tenant_key, packets, bytes);
    rte_spinlock_unlock(&stats->lock);
}

void dp_stats_drop(struct dp_stats *stats,
                   enum dp_drop_reason reason,
                   uint64_t packets,
                   uint64_t bytes) {
    if (reason >= DP_DROP_COUNT) {
        return;
    }

    rte_spinlock_lock(&stats->lock);
    stats->drops[reason] += packets;
    (void)bytes;
    rte_spinlock_unlock(&stats->lock);
}

void dp_stats_maybe_report(struct dp_stats *stats, uint64_t now_tsc) {
    if (stats->report_interval_tsc == 0) {
        return;
    }

    if (stats->last_report_tsc == 0) {
        stats->last_report_tsc = now_tsc;
        return;
    }

    if (now_tsc - stats->last_report_tsc >= stats->report_interval_tsc) {
        dp_stats_report(stats, false);
        stats->last_report_tsc = now_tsc;
    }
}

void dp_stats_report(struct dp_stats *stats, bool final_report) {
    rte_spinlock_lock(&stats->lock);
    printf("%s stats report\n", final_report ? "Final" : "Interval");
    printf("Drops: parse=%" PRIu64 " unmatched=%" PRIu64 " rate_limit=%" PRIu64 "\n",
           stats->drops[DP_DROP_PARSE_ERROR],
           stats->drops[DP_DROP_UNMATCHED],
           stats->drops[DP_DROP_RATE_LIMIT]);

    for (uint32_t i = 0; i < DP_MAX_TOPK; i++) {
        if (stats->topk_flows[i].packets == 0) {
            continue;
        }
        printf("TopK flow[%u]: packets=%" PRIu64 " bytes=%" PRIu64 "\n",
               i,
               stats->topk_flows[i].packets,
               stats->topk_flows[i].bytes);
    }

    for (uint32_t i = 0; i < DP_MAX_TOPK; i++) {
        if (stats->topk_tenants[i].packets == 0) {
            continue;
        }
        printf("TopK tenant[%u]: tenant_id=%" PRIu32 " packets=%" PRIu64 " bytes=%" PRIu64 "\n",
               i,
               stats->topk_tenants[i].key.src_ip,
               stats->topk_tenants[i].packets,
               stats->topk_tenants[i].bytes);
    }
    rte_spinlock_unlock(&stats->lock);
}
