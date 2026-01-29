#include "config.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void dp_config_defaults(struct dp_config *config) {
    config->portmask = DP_DEFAULT_PORTMASK;
    config->rx_queues = 1;
    config->tx_queues = 1;
    config->tenant_count = DP_DEFAULT_TENANTS;
    config->max_rules = DP_DEFAULT_RULES;
    config->mbuf_count = DP_DEFAULT_MBUF_COUNT;
    config->mbuf_cache_size = DP_DEFAULT_MBUF_CACHE;
    config->tsc_hz = rte_get_tsc_hz();
    config->report_interval_tsc = DP_DEFAULT_REPORT_INTERVAL_SEC * config->tsc_hz;
    config->rules_path = "rules.csv";
}

int dp_config_parse(int argc, char **argv, struct dp_config *config) {
    dp_config_defaults(config);

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--portmask") == 0 && i + 1 < argc) {
            config->portmask = (uint32_t)strtoul(argv[++i], NULL, 16);
        } else if (strcmp(argv[i], "--tenants") == 0 && i + 1 < argc) {
            config->tenant_count = (uint32_t)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--rules") == 0 && i + 1 < argc) {
            config->max_rules = (uint32_t)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--rxq") == 0 && i + 1 < argc) {
            config->rx_queues = (uint16_t)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--txq") == 0 && i + 1 < argc) {
            config->tx_queues = (uint16_t)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--rules-file") == 0 && i + 1 < argc) {
            config->rules_path = argv[++i];
        } else if (strcmp(argv[i], "--stats-interval") == 0 && i + 1 < argc) {
            uint64_t seconds = (uint64_t)strtoul(argv[++i], NULL, 10);
            config->report_interval_tsc = seconds * config->tsc_hz;
        }
    }

    if (config->tenant_count == 0 || config->max_rules == 0) {
        fprintf(stderr, "invalid tenants or rules count\n");
        return -1;
    }

    return 0;
}

void dp_register_signals(void (*handler)(int)) {
    signal(SIGINT, handler);
    signal(SIGTERM, handler);
}
