#ifndef DP_CLI_H
#define DP_CLI_H

#include "dataplane/context.h"

int dp_cli_load_rules(struct dp_context *context, const char *path);

#endif
