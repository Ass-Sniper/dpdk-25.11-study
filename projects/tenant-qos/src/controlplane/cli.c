#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rte_byteorder.h>

static int dp_parse_rule_line(const char *line,
                              struct dp_flow_key *key,
                              struct dp_flow_action *action) {
    char src_ip[32];
    char dst_ip[32];
    unsigned int src_port = 0;
    unsigned int dst_port = 0;
    unsigned int proto = 0;
    unsigned int tenant = 0;
    unsigned int policy = 0;
    unsigned long rate = 0;

    if (sscanf(line, "%31[^,],%31[^,],%u,%u,%u,%u,%u,%lu",
               src_ip, dst_ip, &src_port, &dst_port, &proto,
               &tenant, &policy, &rate) != 8) {
        return -1;
    }

    uint8_t bytes[4];
    if (sscanf(src_ip, "%hhu.%hhu.%hhu.%hhu", &bytes[0], &bytes[1], &bytes[2], &bytes[3]) != 4) {
        return -1;
    }
    key->src_ip = rte_cpu_to_be_32((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);

    if (sscanf(dst_ip, "%hhu.%hhu.%hhu.%hhu", &bytes[0], &bytes[1], &bytes[2], &bytes[3]) != 4) {
        return -1;
    }
    key->dst_ip = rte_cpu_to_be_32((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);

    key->src_port = rte_cpu_to_be_16((uint16_t)src_port);
    key->dst_port = rte_cpu_to_be_16((uint16_t)dst_port);
    key->protocol = (uint8_t)proto;

    action->tenant_id = tenant;
    action->policy_id = policy;
    action->rate_limit_bps = rate;
    return 0;
}

int dp_cli_load_rules(struct dp_context *context, const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "failed to open rules file: %s\n", path);
        return -1;
    }

    char line[256];
    uint32_t loaded = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        struct dp_flow_key key;
        struct dp_flow_action action;
        if (dp_parse_rule_line(line, &key, &action) != 0) {
            continue;
        }
        if (dp_classify_add(&context->classifier, &key, &action) != 0) {
            break;
        }
        dp_qos_update_policy(&context->qos, action.policy_id, action.rate_limit_bps);
        loaded++;
    }

    fclose(fp);
    printf("Loaded %u rules from %s\n", loaded, path);
    return 0;
}
