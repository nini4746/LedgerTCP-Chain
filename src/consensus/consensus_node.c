#include "consensus.h"

consensus_node_t *cnode_create(int node_id) {
    consensus_node_t *node = malloc(sizeof(consensus_node_t));
    if (!node) return NULL;

    node->node_id = node_id;
    node->main_chain = chain_create();
    node->ledger = ledger_create();
    node->fork_chains = NULL;
    node->fork_count = 0;

    if (!node->main_chain || !node->ledger) {
        cnode_destroy(node);
        return NULL;
    }

    block_t *genesis = genesis_create();
    if (!genesis) {
        cnode_destroy(node);
        return NULL;
    }

    chain_add_block(node->main_chain, genesis);

    return node;
}

void cnode_destroy(consensus_node_t *node) {
    if (!node) return;

    chain_destroy(node->main_chain);
    ledger_destroy(node->ledger);
    fork_cleanup(node);

    free(node);
}

blockchain_t *cnode_get_chain(consensus_node_t *node) {
    return node ? node->main_chain : NULL;
}

ledger_t *cnode_get_ledger(consensus_node_t *node) {
    return node ? node->ledger : NULL;
}

int cnode_update_state(consensus_node_t *node) {
    if (!node) return -1;

    blockchain_t *longest = fork_get_longest(node);

    if (longest != node->main_chain) {
        return reorg_execute(node, longest);
    }

    return 0;
}
