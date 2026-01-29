#ifndef DP_CLI_H
#define DP_CLI_H

#include "dataplane/context.h"
#include "controlplane/config.h"

void dp_cli_init(struct dp_context *context, const struct dp_config *config);

#endif
