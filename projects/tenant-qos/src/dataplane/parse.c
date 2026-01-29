#include "parse.h"

#include <rte_byteorder.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>

int dp_parse_5tuple(const struct rte_mbuf *mbuf, struct dp_flow_key *key) {
    const uint8_t *data = rte_pktmbuf_mtod(mbuf, const uint8_t *);
    uint32_t offset = 0;

    if (mbuf->pkt_len < sizeof(struct rte_ether_hdr)) {
        return -1;
    }

    const struct rte_ether_hdr *eth = (const struct rte_ether_hdr *)(data + offset);
    offset += sizeof(struct rte_ether_hdr);

    if (eth->ether_type != rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4)) {
        return -1;
    }

    if (mbuf->pkt_len < offset + sizeof(struct rte_ipv4_hdr)) {
        return -1;
    }

    const struct rte_ipv4_hdr *ip = (const struct rte_ipv4_hdr *)(data + offset);
    uint8_t ihl = (ip->version_ihl & 0x0f) * 4;
    if (ihl < sizeof(struct rte_ipv4_hdr)) {
        return -1;
    }

    if (mbuf->pkt_len < offset + ihl) {
        return -1;
    }

    offset += ihl;

    key->src_ip = ip->src_addr;
    key->dst_ip = ip->dst_addr;
    key->protocol = ip->next_proto_id;

    if (key->protocol == IPPROTO_UDP) {
        if (mbuf->pkt_len < offset + sizeof(struct rte_udp_hdr)) {
            return -1;
        }
        const struct rte_udp_hdr *udp = (const struct rte_udp_hdr *)(data + offset);
        key->src_port = udp->src_port;
        key->dst_port = udp->dst_port;
        return 0;
    }

    if (key->protocol == IPPROTO_TCP) {
        if (mbuf->pkt_len < offset + sizeof(struct rte_tcp_hdr)) {
            return -1;
        }
        const struct rte_tcp_hdr *tcp = (const struct rte_tcp_hdr *)(data + offset);
        key->src_port = tcp->src_port;
        key->dst_port = tcp->dst_port;
        return 0;
    }

    return -1;
}
