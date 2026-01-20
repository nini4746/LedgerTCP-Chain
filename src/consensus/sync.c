#include "consensus.h"

int sync_start(consensus_node_t *node) {
    if (!node) return -1;

    fork_cleanup(node);
    return 0;
}

static int add_block_to_main(consensus_node_t *node, block_t *block) {
    block_t *new_block = malloc(sizeof(block_t));
    if (!new_block) return -1;

    memcpy(new_block, block, sizeof(block_t));
    new_block->next = NULL;

    if (chain_add_block(node->main_chain, new_block) != 0) {
        free(new_block);
        return -1;
    }

    for (size_t i = 0; i < new_block->tx_count; i++) {
        transaction_apply(node->ledger, &new_block->transactions[i]);
    }
    return 0;
}

static block_t *clone_block(block_t *src) {
    block_t *clone = malloc(sizeof(block_t));
    if (clone) { memcpy(clone, src, sizeof(block_t)); clone->next = NULL; }
    return clone;
}

static int create_fork_chain(consensus_node_t *node, block_t *block,
                             block_t *parent) {
    blockchain_t *fork_chain = chain_create();
    if (!fork_chain) return -1;

    for (block_t *cur = node->main_chain->head; cur && cur != parent->next; cur = cur->next) {
        block_t *clone = clone_block(cur);
        if (!clone) { chain_destroy(fork_chain); return -1; }
        chain_add_block(fork_chain, clone);
    }

    block_t *new_block = clone_block(block);
    if (!new_block) { chain_destroy(fork_chain); return -1; }
    chain_add_block(fork_chain, new_block);

    ledger_t *fork_state = reorg_rebuild_ledger(fork_chain, node->genesis_balance);
    if (!fork_state) { chain_destroy(fork_chain); return -1; }

    fork_add_candidate(node, fork_chain, fork_state);
    return 0;
}

int sync_receive_block(consensus_node_t *node, block_t *block) {
    if (!node || !block) return -1;

    if (node->main_chain->tail &&
        hash_equals(block->prev_hash, node->main_chain->tail->hash)) {
        return add_block_to_main(node, block);
    }

    block_t *parent = blockchain_get_block_by_hash(node->main_chain,
                                                    block->prev_hash);
    if (parent && node->fork_count < MAX_CHAINS) {
        return create_fork_chain(node, block, parent);
    }

    return -1;
}

int sync_broadcast_block(tcp_simulator_t *sim, int from_node,
                         block_t *block, int num_nodes) {
    if (!sim || !block || from_node < 0) return -1;

    for (int i = 0; i < num_nodes; i++) {
        if (i != from_node) {
            network_send(sim, from_node, i, block, sizeof(block_t));
        }
    }

    return 0;
}

bool sync_check_consensus(consensus_node_t **nodes, int node_count) {
    if (!nodes || node_count <= 0) return false;

    for (int i = 1; i < node_count; i++) {
        size_t len_0 = chain_get_length(nodes[0]->main_chain);
        size_t len_i = chain_get_length(nodes[i]->main_chain);

        if (len_0 != len_i) return false;

        if (!state_compare(nodes[0]->ledger, nodes[i]->ledger)) {
            return false;
        }
    }

    return true;
}

static blockchain_t *find_longest_chain(consensus_node_t **nodes, int count) {
    blockchain_t *longest = NULL;
    size_t max_length = 0;

    for (int i = 0; i < count; i++) {
        cnode_update_state(nodes[i]);
        blockchain_t *chain = fork_get_longest(nodes[i]);
        size_t length = chain_get_length(chain);

        if (length > max_length ||
            (length == max_length && longest && chain->tail &&
             memcmp(chain->tail->hash, longest->tail->hash, HASH_SIZE) < 0)) {
            max_length = length;
            longest = chain;
        }
    }
    return longest;
}

int sync_finalize(consensus_node_t **nodes, int node_count) {
    if (!nodes || node_count <= 0) return -1;

    blockchain_t *longest = find_longest_chain(nodes, node_count);
    if (!longest) return -1;

    for (int i = 0; i < node_count; i++) {
        if (nodes[i]->main_chain != longest) {
            if (reorg_execute(nodes[i], longest) != 0) return -1;
        }
    }

    return 0;
}
