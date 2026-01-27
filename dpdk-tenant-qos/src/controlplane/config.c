#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int dp_config_load(int argc, char **argv, struct dp_config *config) {
    config->tenant_count = DP_DEFAULT_TENANTS;
    config->tsc_hz = 1000000000ULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tenants") == 0 && i + 1 < argc) {
            config->tenant_count = (uint32_t)atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--tsc-hz") == 0 && i + 1 < argc) {
            config->tsc_hz = (uint64_t)atoll(argv[i + 1]);
            i++;
        }
    }

    if (config->tenant_count == 0) {
        fprintf(stderr, "tenant count must be > 0\n");
        return 1;
    }

    return 0;
}
