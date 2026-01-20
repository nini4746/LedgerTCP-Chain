#include "ledger.h"
#include "block.h"
#include "tcp_simulator.h"
#include <stdio.h>
#include <limits.h>

int main(void) {
    int failed = 0;

    // NULL pointer tests
    if (ledger_validate(NULL)) { printf("FAIL: NULL ledger\n"); failed = 1; }
    if (block_validate_structure(NULL)) { printf("FAIL: NULL block\n"); failed = 1; }
    if (chain_get_length(NULL) != 0) { printf("FAIL: NULL chain length\n"); failed = 1; }

    transaction_t tx = {1, 1, 2, 100};
    if (transaction_apply(NULL, &tx) == 0) { printf("FAIL: NULL ledger in tx\n"); failed = 1; }

    // Zero/invalid value tests
    ledger_t *ledger = ledger_create();
    transaction_t zero_tx = {0, 0, 0, 0};
    if (transaction_validate(&zero_tx)) { printf("FAIL: Zero tx\n"); failed = 1; }

    // Negative amount test
    transaction_t neg_tx = {99, 1, 2, -1000};
    if (transaction_validate(&neg_tx)) { printf("FAIL: Negative amount\n"); failed = 1; }

    // Self-transfer test
    transaction_t self_tx = {100, 1, 1, 100};
    if (transaction_validate(&self_tx)) { printf("FAIL: Self transfer\n"); failed = 1; }

    // Max values test
    transaction_t max_tx = {UINT64_MAX, 1, 2, INT64_MAX};
    account_set_balance(ledger, 1, INT64_MAX);
    if (transaction_apply(ledger, &max_tx) != 0) { printf("FAIL: Max values\n"); failed = 1; }

    // Overflow test - balance + amount > INT64_MAX
    ledger_t *overflow_ledger = ledger_create();
    account_set_balance(overflow_ledger, 1, INT64_MAX);
    account_set_balance(overflow_ledger, 2, INT64_MAX - 100);
    transaction_t overflow_tx = {200, 1, 2, 200};  // Would overflow account 2
    transaction_apply(overflow_ledger, &overflow_tx);  // Should handle gracefully

    // Insufficient balance test
    ledger_t *empty_ledger = ledger_create();
    account_set_balance(empty_ledger, 1, 0);
    transaction_t empty_tx = {201, 1, 2, 100};
    if (transaction_apply(empty_ledger, &empty_tx) == 0) { printf("FAIL: Empty balance\n"); failed = 1; }

    // Non-existent account test
    if (account_get_balance(empty_ledger, 9999) != 0) { printf("FAIL: Non-existent account\n"); failed = 1; }

    // MAX limits tests
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

    // Network edge cases
    tcp_simulator_t *sim = tcp_simulator_create(2);

    // Oversized packet
    char large_data[MAX_PACKET_SIZE + 100];
    if (network_send(sim, 0, 1, large_data, sizeof(large_data)) == 0) {
        printf("FAIL: Oversized packet\n"); failed = 1;
    }

    // Zero-size packet
    if (network_send(sim, 0, 1, large_data, 0) == 0) {
        printf("FAIL: Zero size packet\n"); failed = 1;
    }

    // Invalid node IDs
    if (network_send(sim, -1, 1, "test", 4) == 0) { printf("FAIL: Negative src node\n"); failed = 1; }
    if (network_send(sim, 0, 999, "test", 4) == 0) { printf("FAIL: Invalid dst node\n"); failed = 1; }

    // Block with invalid prev_hash
    hash_t fake_hash;
    memset(fake_hash, 0xFF, HASH_SIZE);
    block_t *orphan = block_create(fake_hash, 100);
    blockchain_t *chain = chain_create();
    block_t *genesis = genesis_create();
    chain_add_block(chain, genesis);
    // orphan block should not link properly
    if (validate_prev_hash(orphan, genesis)) { printf("FAIL: Invalid prev_hash accepted\n"); failed = 1; }

    // Empty chain operations
    blockchain_t *empty_chain = chain_create();
    if (chain_get_head(empty_chain) != NULL) { printf("FAIL: Empty chain head\n"); failed = 1; }
    hash_t dummy_hash;
    hash_zero(dummy_hash);
    if (blockchain_get_block_by_hash(empty_chain, dummy_hash) != NULL) { printf("FAIL: Empty chain search\n"); failed = 1; }

    // Cleanup
    ledger_destroy(ledger);
    ledger_destroy(ledger2);
    ledger_destroy(ledger3);
    ledger_destroy(overflow_ledger);
    ledger_destroy(empty_ledger);
    block_destroy(block);
    block_destroy(orphan);
    chain_destroy(chain);
    chain_destroy(empty_chain);
    tcp_simulator_destroy(sim);

    if (failed) return 1;
    return 0;
}
