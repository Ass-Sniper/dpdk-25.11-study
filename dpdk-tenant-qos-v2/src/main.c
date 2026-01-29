#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <rte_cycles.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_lcore.h>
#include <rte_log.h>
#include <rte_mbuf.h>

#include "controlplane/cli.h"
#include "controlplane/config.h"
#include "dataplane/classify.h"
#include "dataplane/context.h"
#include "dataplane/parse.h"
#include "dataplane/qos.h"
#include "dataplane/stats.h"

#define DP_RX_BURST 32

static volatile bool dp_force_quit = false;

static void dp_signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        dp_force_quit = true;
    }
}

static int dp_port_init(uint16_t port_id, uint16_t rx_queues, uint16_t tx_queues,
                        struct rte_mempool *mbuf_pool) {
    struct rte_eth_conf port_conf = {
        .rxmode = {
            .mq_mode = RTE_ETH_MQ_RX_NONE,
        },
    };

    int ret = rte_eth_dev_configure(port_id, rx_queues, tx_queues, &port_conf);
    if (ret < 0) {
        return ret;
    }

    for (uint16_t q = 0; q < rx_queues; q++) {
        ret = rte_eth_rx_queue_setup(port_id, q, 1024,
                                     rte_eth_dev_socket_id(port_id), NULL,
                                     mbuf_pool);
        if (ret < 0) {
            return ret;
        }
    }

    for (uint16_t q = 0; q < tx_queues; q++) {
        ret = rte_eth_tx_queue_setup(port_id, q, 1024,
                                     rte_eth_dev_socket_id(port_id), NULL);
        if (ret < 0) {
            return ret;
        }
    }

    struct rte_eth_dev_info dev_info;
    memset(&dev_info, 0, sizeof(dev_info));

    ret = rte_eth_dev_info_get(port_id, &dev_info);
    if (ret < 0) {
        RTE_LOG(WARNING, USER1,
            "dev_info_get(%u) failed: %s (continue)\n",
                port_id, rte_strerror(-ret));
    }

    /* Some virtual devices (e.g. net_vhost) may not support MTU/promisc; treat as best-effort. */    
    ret = rte_eth_dev_set_mtu(port_id, RTE_ETHER_MTU);
    if (ret < 0) {
        RTE_LOG(WARNING, USER1, "set_mtu(%u) not supported: %s\n",
            port_id, rte_strerror(-ret));
    }   

    ret = rte_eth_dev_start(port_id);
    if (ret < 0) {
        return ret;
    }

    ret = rte_eth_promiscuous_enable(port_id);
    if (ret < 0) {
        RTE_LOG(WARNING, USER1, "promiscuous_enable(%u) not supported: %s\n",
            port_id, rte_strerror(-ret));
    }
    return 0;
}

static int dp_lcore_main(void *arg) {
    struct dp_context *context = arg;
    const uint16_t port_id = context->ports[0];
    const uint16_t queue_id = 0;
    struct rte_mbuf *pkts[DP_RX_BURST];

    uint32_t idle_loops = 0;

    while (!dp_force_quit) {
        uint16_t nb_rx = rte_eth_rx_burst(port_id, queue_id, pkts, DP_RX_BURST);
        if (nb_rx == 0) {

            /*
            * Idle backoff:
            * - rte_pause(): reduce power / HT contention
            * - occasional sleep to drop CPU usage
            */
            rte_pause();

            if (++idle_loops > 1024) {
                rte_delay_us_sleep(50); /* 50us */
                idle_loops = 0;
            }

            continue;
        }

        idle_loops = 0;

        for (uint16_t i = 0; i < nb_rx; i++) {
            struct rte_mbuf *mbuf = pkts[i];
            struct dp_flow_key key;
            struct dp_flow_action action;

            if (dp_parse_5tuple(mbuf, &key) != 0) {
                dp_stats_drop(&context->stats, DP_DROP_PARSE_ERROR, 1, mbuf->pkt_len);
                rte_pktmbuf_free(mbuf);
                continue;
            }

            if (dp_classify_lookup(&context->classifier, &key, &action) != 0) {
                dp_stats_drop(&context->stats, DP_DROP_UNMATCHED, 1, mbuf->pkt_len);
                rte_pktmbuf_free(mbuf);
                continue;
            }

            if (dp_qos_apply(&context->qos, &action, mbuf->pkt_len) != 0) {
                dp_stats_drop(&context->stats, DP_DROP_RATE_LIMIT, 1, mbuf->pkt_len);
                rte_pktmbuf_free(mbuf);
                continue;
            }

            dp_stats_record(&context->stats, &key, &action, 1, mbuf->pkt_len);
            rte_pktmbuf_free(mbuf);
        }

        dp_stats_maybe_report(&context->stats, rte_rdtsc());
    }

    return 0;
}

int main(int argc, char **argv) {
    struct dp_config config;
    struct dp_context context;

    int ret = rte_eal_init(argc, argv);
    if (ret < 0) {
        rte_exit(EXIT_FAILURE, "failed to init EAL\n");
    }

    argc -= ret;
    argv += ret;

    if (dp_config_parse(argc, argv, &config) != 0) {
        rte_exit(EXIT_FAILURE, "invalid application config\n");
    }

    dp_register_signals(dp_signal_handler);

    if (dp_context_init(&context, &config) != 0) {
        rte_exit(EXIT_FAILURE, "failed to init dataplane\n");
    }

    if (dp_cli_load_rules(&context, config.rules_path) != 0) {
        rte_exit(EXIT_FAILURE, "failed to load rules\n");
    }

    for (uint16_t i = 0; i < context.nb_ports; i++) {
        if (dp_port_init(context.ports[i], config.rx_queues, config.tx_queues,
                         context.mbuf_pool) != 0) {
            rte_exit(EXIT_FAILURE, "failed to init port %u\n", context.ports[i]);
        }
    }

    rte_eal_mp_remote_launch(dp_lcore_main, &context, CALL_MAIN);
    RTE_LCORE_FOREACH_WORKER(ret) {
        rte_eal_wait_lcore(ret);
    }

    dp_stats_report(&context.stats, true);
    dp_context_close(&context);
    rte_eal_cleanup();
    return 0;
}
