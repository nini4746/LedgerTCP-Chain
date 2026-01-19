#include "consensus.h"

ledger_t *reorg_rebuild_ledger(blockchain_t *chain) {
    if (!chain) return NULL;

    ledger_t *ledger = genesis_init_state();
    if (!ledger) return NULL;

    block_t *block = chain_get_head(chain);
    if (block) {
        block = block->next;
    }

    while (block) {
        for (size_t i = 0; i < block->tx_count; i++) {
            transaction_apply(ledger, &block->transactions[i]);
        }
        block = block->next;
    }

    return ledger;
}

bool reorg_validate(blockchain_t *chain) {
    if (!chain) return false;

    block_t *block = chain_get_head(chain);
    block_t *prev = NULL;

    while (block) {
        if (!validate_block_structure(block)) return false;
        if (!validate_block_transactions(block)) return false;
        if (prev && !validate_prev_hash(block, prev)) return false;

        prev = block;
        block = block->next;
    }

    return true;
}

int reorg_rollback(consensus_node_t *node) {
    if (!node) return -1;

    ledger_destroy(node->ledger);
    node->ledger = ledger_create();

    return node->ledger ? 0 : -1;
}

int reorg_commit(consensus_node_t *node, blockchain_t *new_chain,
                 ledger_t *new_ledger) {
    if (!node || !new_chain || !new_ledger) return -1;

    blockchain_t *old_chain = node->main_chain;
    ledger_t *old_ledger = node->ledger;

    node->main_chain = new_chain;
    node->ledger = new_ledger;

    chain_destroy(old_chain);
    ledger_destroy(old_ledger);

    return 0;
}

int reorg_execute(consensus_node_t *node, blockchain_t *new_chain) {
    if (!node || !new_chain) return -1;

    if (!reorg_validate(new_chain)) return -1;

    ledger_t *new_ledger = reorg_rebuild_ledger(new_chain);
    if (!new_ledger) return -1;

    blockchain_t *chain_copy = blockchain_clone(new_chain);
    if (!chain_copy) {
        ledger_destroy(new_ledger);
        return -1;
    }

    return reorg_commit(node, chain_copy, new_ledger);
}
