#ifndef DP_CLASSIFY_H
#define DP_CLASSIFY_H

#include <stdint.h>

#include "parse.h"

#define DP_MAX_RULES 4096

struct dp_flow_action {
    uint32_t tenant_id;
    uint32_t policy_id;
    uint64_t rate_limit_bps;
};

struct dp_classifier_entry {
    struct dp_flow_key key;
    struct dp_flow_action action;
};

struct dp_classifier {
    struct dp_classifier_entry entries[DP_MAX_RULES];
    uint32_t count;
};

int dp_classify_init(struct dp_classifier *classifier);
int dp_classify_add(struct dp_classifier *classifier,
                    const struct dp_flow_key *key,
                    const struct dp_flow_action *action);
int dp_classify_lookup(const struct dp_classifier *classifier,
                       const struct dp_flow_key *key,
                       struct dp_flow_action *action);

#endif
