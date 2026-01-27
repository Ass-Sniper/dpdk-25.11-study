#ifndef DP_CONFIG_H
#define DP_CONFIG_H

#include <stdint.h>

#define DP_DEFAULT_TENANTS 4

struct dp_config {
    uint32_t tenant_count;
    uint64_t tsc_hz;
};

int dp_config_load(int argc, char **argv, struct dp_config *config);

#endif
