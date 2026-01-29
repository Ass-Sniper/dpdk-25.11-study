#include "classify.h"

#include <string.h>

int dp_classify_init(struct dp_classifier *classifier) {
    classifier->count = 0;
    return 0;
}

int dp_classify_add(struct dp_classifier *classifier,
                    const struct dp_flow_key *key,
                    const struct dp_flow_action *action) {
    if (classifier->count >= DP_MAX_RULES) {
        return 1;
    }

    classifier->entries[classifier->count].key = *key;
    classifier->entries[classifier->count].action = *action;
    classifier->count++;
    return 0;
}

int dp_classify_lookup(const struct dp_classifier *classifier,
                       const struct dp_flow_key *key,
                       struct dp_flow_action *action) {
    for (uint32_t i = 0; i < classifier->count; i++) {
        const struct dp_classifier_entry *entry = &classifier->entries[i];
        if (memcmp(&entry->key, key, sizeof(*key)) == 0) {
            *action = entry->action;
            return 0;
        }
    }
    return 1;
}
