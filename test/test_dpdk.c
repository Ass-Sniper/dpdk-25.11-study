// test_dpdk.c
#include <rte_eal.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (rte_eal_init(argc, argv) < 0) {
        printf("EAL init failed\n");
        return 1;
    }
    printf("DPDK EAL init OK\n");
    return 0;
}

