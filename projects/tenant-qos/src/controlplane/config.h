#ifndef DP_CONFIG_H
#define DP_CONFIG_H

#include <stdint.h>

#include <rte_cycles.h>

#define DP_DEFAULT_PORTMASK 0x1
#define DP_DEFAULT_TENANTS 4
#define DP_DEFAULT_RULES 1024
#define DP_DEFAULT_MBUF_COUNT 8192
#define DP_DEFAULT_MBUF_CACHE 256
#define DP_DEFAULT_REPORT_INTERVAL_SEC 5

struct dp_config {
    uint32_t portmask;
    uint16_t rx_queues;
    uint16_t tx_queues;
    uint32_t tenant_count;
    uint32_t max_rules;
    uint32_t mbuf_count;
    uint32_t mbuf_cache_size;
    uint64_t tsc_hz;
    uint64_t report_interval_tsc;
    const char *rules_path;
};

int dp_config_parse(int argc, char **argv, struct dp_config *config);
void dp_register_signals(void (*handler)(int));

#endif
