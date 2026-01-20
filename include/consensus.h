#ifndef CONSENSUS_H
#define CONSENSUS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "ledger.h"
#include "tcp_simulator.h"
#include "utils.h"

#define MAX_CHAINS 50

typedef struct chain_candidate {
    blockchain_t *chain;
    ledger_t *state;
    struct chain_candidate *next;
} chain_candidate_t;

typedef struct {
    int node_id;
    blockchain_t *main_chain;
    ledger_t *ledger;
    chain_candidate_t *fork_chains;
    size_t fork_count;
    balance_t genesis_balance;
} consensus_node_t;

int consensus_init(void);
void consensus_cleanup(void);
int consensus_process_block(consensus_node_t *node, block_t *block);
ledger_t *consensus_get_state(consensus_node_t *node);
bool consensus_verify(consensus_node_t **nodes, int node_count);

consensus_node_t *cnode_create(int node_id, balance_t genesis_balance);
void cnode_destroy(consensus_node_t *node);
blockchain_t *cnode_get_chain(consensus_node_t *node);
ledger_t *cnode_get_ledger(consensus_node_t *node);
int cnode_update_state(consensus_node_t *node);

chain_candidate_t *fork_create(blockchain_t *chain, ledger_t *state);
void fork_destroy(chain_candidate_t *fork);
int fork_add_candidate(consensus_node_t *node, blockchain_t *chain, ledger_t *state);
blockchain_t *fork_get_longest(consensus_node_t *node);
void fork_cleanup(consensus_node_t *node);

ledger_t *reorg_rebuild_ledger(blockchain_t *chain, balance_t genesis_balance);
bool reorg_validate(blockchain_t *chain);
int reorg_rollback(consensus_node_t *node);
int reorg_commit(consensus_node_t *node, blockchain_t *new_chain, ledger_t *new_ledger);
int reorg_execute(consensus_node_t *node, blockchain_t *new_chain);

int sync_start(consensus_node_t *node);
int sync_receive_block(consensus_node_t *node, block_t *block);
int sync_broadcast_block(tcp_simulator_t *sim, int from_node, block_t *block, int num_nodes);
bool sync_check_consensus(consensus_node_t **nodes, int node_count);
int sync_finalize(consensus_node_t **nodes, int node_count);

#endif
