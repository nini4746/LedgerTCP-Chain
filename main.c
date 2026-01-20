#include "tcp_simulator.h"
#include "ledger.h"
#include "block.h"
#include "consensus.h"
#include "utils.h"

#define NUM_NODES 3
#define NUM_TRANSACTIONS 10
#define SIMULATION_STEPS 100

void print_ledger_state(consensus_node_t *node) {
    ledger_t *ledger = cnode_get_ledger(node);
    blockchain_t *chain = cnode_get_chain(node);

    printf("Node %d Ledger State:\n", node->node_id);
    printf("  Chain length: %zu\n", chain_get_length(chain));

    for (size_t i = 0; i < ledger->account_count; i++) {
        account_t *acc = &ledger->accounts[i];
        printf("  Account %lu: Balance = %ld\n",
               acc->account_id, acc->balance);
    }
    printf("\n");
}

int main(void) {
    printf("=== LedgerTCP-Chain Simulation ===\n\n");

    consensus_init();

    tcp_simulator_t *sim = tcp_simulator_create(NUM_NODES);
    if (!sim) {
        fprintf(stderr, "Failed to create TCP simulator\n");
        return 1;
    }

    tcp_simulator_set_probabilities(sim, 0.2, 0.1, 0.1, 0.1);
    printf("TCP Simulator initialized with %d nodes\n", NUM_NODES);
    printf("Probabilities: delay=0.2, drop=0.1, dup=0.1, reorder=0.1\n\n");

    consensus_node_t *nodes[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i] = cnode_create(i, 1000);
        if (!nodes[i]) {
            fprintf(stderr, "Failed to create consensus node %d\n", i);
            tcp_simulator_destroy(sim);
            return 1;
        }
    }
    printf("Initialized %d consensus nodes with genesis balances\n\n", NUM_NODES);

    printf("Creating and broadcasting transactions...\n");
    for (int i = 0; i < NUM_TRANSACTIONS; i++) {
        transaction_t tx;
        tx.tx_id = i + 1;
        tx.from = (i % 3) + 1;
        tx.to = ((i + 1) % 3) + 1;
        tx.amount = 10 + (i * 5);

        printf("TX %lu: %lu -> %lu, amount=%ld\n",
               tx.tx_id, tx.from, tx.to, tx.amount);

        int creator_node = i % NUM_NODES;

        blockchain_t *chain = cnode_get_chain(nodes[creator_node]);
        block_t *new_block = block_create(chain->tail->hash,
                                          chain->tail->height + 1);
        if (new_block) {
            block_add_transaction(new_block, &tx);
            block_compute_hash(new_block);

            consensus_process_block(nodes[creator_node], new_block);

            sync_broadcast_block(sim, creator_node, new_block, NUM_NODES);

            free(new_block);
        }
    }
    printf("\n");

    printf("Running simulation for %d steps...\n", SIMULATION_STEPS);
    for (int step = 0; step < SIMULATION_STEPS; step++) {
        tcp_simulator_step(sim);

        for (int i = 0; i < NUM_NODES; i++) {
            block_t received_block;
            int received = network_receive(sim, i, &received_block,
                                          sizeof(block_t));

            if (received > 0) {
                consensus_process_block(nodes[i], &received_block);
            }
        }

        if (step % 20 == 0) {
            printf("Step %d: Processing blocks...\n", step);
        }
    }
    printf("\n");

    printf("Finalizing consensus...\n");
    sync_finalize(nodes, NUM_NODES);
    printf("\n");

    printf("=== Final Ledger States ===\n\n");
    for (int i = 0; i < NUM_NODES; i++) {
        print_ledger_state(nodes[i]);
    }

    printf("Verifying consensus...\n");
    bool consensus_reached = consensus_verify(nodes, NUM_NODES);

    if (consensus_reached) {
        printf("SUCCESS: All nodes reached consensus!\n");
    } else {
        printf("FAIL: Consensus not reached\n");
    }
    printf("\n");

    for (int i = 0; i < NUM_NODES; i++) {
        cnode_destroy(nodes[i]);
    }
    tcp_simulator_destroy(sim);

    consensus_cleanup();

    printf("=== Simulation Complete ===\n");
    return consensus_reached ? 0 : 1;
}
