#include "classify.h"

#include <stdio.h>

#include <rte_hash_crc.h>
#include <rte_malloc.h>

int dp_classify_init(struct dp_classifier *classifier, uint32_t max_rules) {
    struct rte_hash_parameters params = {
        .name = "dp_flow_hash",
        .entries = max_rules,
        .key_len = sizeof(struct dp_flow_key),
        .hash_func = rte_hash_crc,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };

    classifier->hash = rte_hash_create(&params);
    if (classifier->hash == NULL) {
        return -1;
    }

    classifier->actions = rte_zmalloc("dp_actions",
                                      max_rules * sizeof(struct dp_flow_action),
                                      RTE_CACHE_LINE_SIZE);
    if (classifier->actions == NULL) {
        rte_hash_free(classifier->hash);
        classifier->hash = NULL;
        return -1;
    }

    classifier->max_rules = max_rules;
    return 0;
}

void dp_classify_close(struct dp_classifier *classifier) {
    if (classifier->hash != NULL) {
        rte_hash_free(classifier->hash);
        classifier->hash = NULL;
    }
    rte_free(classifier->actions);
    classifier->actions = NULL;
}

int dp_classify_add(struct dp_classifier *classifier,
                    const struct dp_flow_key *key,
                    const struct dp_flow_action *action) {
    int32_t idx = rte_hash_add_key(classifier->hash, key);
    if (idx < 0 || (uint32_t)idx >= classifier->max_rules) {
        return -1;
    }

    classifier->actions[idx] = *action;
    return 0;
}

int dp_classify_lookup(const struct dp_classifier *classifier,
                       const struct dp_flow_key *key,
                       struct dp_flow_action *action) {
    int32_t idx = rte_hash_lookup(classifier->hash, key);
    if (idx < 0 || (uint32_t)idx >= classifier->max_rules) {
        return -1;
    }

    *action = classifier->actions[idx];
    return 0;
}
