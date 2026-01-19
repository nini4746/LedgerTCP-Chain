#include "ledger.h"
#include "block.h"
#include "tcp_simulator.h"
#include <stdio.h>

int main(void) {
    int failed = 0;

    if (ledger_validate(NULL)) { printf("FAIL: NULL ledger\n"); failed = 1; }

    transaction_t tx = {1, 1, 2, 100};
    if (transaction_apply(NULL, &tx) == 0) { printf("FAIL: NULL ledger in tx\n"); failed = 1; }

    ledger_t *ledger = ledger_create();
    transaction_t zero_tx = {0, 0, 0, 0};
    if (transaction_validate(&zero_tx)) { printf("FAIL: Zero tx\n"); failed = 1; }

    transaction_t max_tx = {UINT64_MAX, 1, 2, INT64_MAX};
    account_set_balance(ledger, 1, INT64_MAX);
    if (transaction_apply(ledger, &max_tx) != 0) { printf("FAIL: Max values\n"); failed = 1; }

    ledger_t *ledger2 = ledger_create();
    for (size_t i = 0; i < MAX_ACCOUNTS + 10; i++) {
        account_set_balance(ledger2, i, 100);
    }

    ledger_t *ledger3 = ledger_create();
    account_set_balance(ledger3, 1, 1000000);
    for (size_t i = 0; i < MAX_TX_HISTORY + 10; i++) {
        transaction_t tx_i = {i, 1, 2, 1};
        transaction_apply(ledger3, &tx_i);
    }

    block_t *block = genesis_create();
    for (size_t i = 0; i < MAX_TX_PER_BLOCK + 10; i++) {
        transaction_t tx_i = {i, 1, 2, 1};
        block_add_transaction(block, &tx_i);
    }

    tcp_simulator_t *sim = tcp_simulator_create(2);
    char large_data[MAX_PACKET_SIZE + 100];
    if (network_send(sim, 0, 1, large_data, sizeof(large_data)) == 0) {
        printf("FAIL: Oversized packet\n"); failed = 1;
    }

    ledger_destroy(ledger);
    ledger_destroy(ledger2);
    ledger_destroy(ledger3);
    block_destroy(block);
    tcp_simulator_destroy(sim);

    if (failed) return 1;
    return 0;
}
