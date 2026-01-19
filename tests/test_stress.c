#include "consensus.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    int failed = 0;
    consensus_init();

    // Test: Process 100 transactions with network issues
    tcp_simulator_t *sim = tcp_simulator_create(3);
    tcp_simulator_set_probabilities(sim, 0.05, 0.02, 0.02, 0.02);

    consensus_node_t *nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = cnode_create(i);
        ledger_t *ledger = cnode_get_ledger(nodes[i]);
        account_set_balance(ledger, 1, 10000000);
        account_set_balance(ledger, 2, 10000000);
        account_set_balance(ledger, 3, 10000000);
    }

    // Send 100 transactions with interleaved network processing
    for (int i = 0; i < 100; i++) {
        transaction_t tx;
        tx.tx_id = i + 1;
        tx.from = (i % 3) + 1;
        tx.to = ((i + 1) % 3) + 1;
        tx.amount = 1;

        int creator = i % 3;
        blockchain_t *chain = cnode_get_chain(nodes[creator]);
        block_t *block = block_create(chain->tail->hash, chain->tail->height + 1);
        block_add_transaction(block, &tx);
        block_compute_hash(block);

        consensus_process_block(nodes[creator], block);
        sync_broadcast_block(sim, creator, block, 3);

        free(block);

        // Process network packets between blocks
        for (int step = 0; step < 20; step++) {
            tcp_simulator_step(sim);

            for (int j = 0; j < 3; j++) {
                block_t recv_block;
                int received = network_receive(sim, j, &recv_block, sizeof(block_t));
                if (received > 0) {
                    consensus_process_block(nodes[j], &recv_block);
                }
            }
        }
    }

    // Final network processing for remaining packets
    for (int step = 0; step < 1000; step++) {
        tcp_simulator_step(sim);

        for (int i = 0; i < 3; i++) {
            block_t recv_block;
            int received = network_receive(sim, i, &recv_block, sizeof(block_t));
            if (received > 0) {
                consensus_process_block(nodes[i], &recv_block);
            }
        }

        // Periodically sync nodes
        if (step % 200 == 0) {
            for (int i = 0; i < 3; i++) {
                cnode_update_state(nodes[i]);
            }
        }
    }

    sync_finalize(nodes, 3);

    if (!consensus_verify(nodes, 3)) {
        printf("FAIL: Consensus not reached\n");
        for (int i = 0; i < 3; i++) {
            printf("Node %d: chain_length=%zu, balance1=%ld, balance2=%ld, balance3=%ld, forks=%zu\n",
                   i,
                   chain_get_length(nodes[i]->main_chain),
                   account_get_balance(nodes[i]->ledger, 1),
                   account_get_balance(nodes[i]->ledger, 2),
                   account_get_balance(nodes[i]->ledger, 3),
                   nodes[i]->fork_count);
        }
        failed = 1;
    }

    for (int i = 0; i < 3; i++) {
        cnode_destroy(nodes[i]);
    }
    tcp_simulator_destroy(sim);

    consensus_cleanup();

    if (failed) return 1;
    return 0;
}
