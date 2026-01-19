#include "consensus.h"

chain_candidate_t *fork_create(blockchain_t *chain, ledger_t *state) {
    if (!chain || !state) return NULL;

    chain_candidate_t *fork = malloc(sizeof(chain_candidate_t));
    if (!fork) return NULL;

    fork->chain = chain;
    fork->state = state;
    fork->next = NULL;

    return fork;
}

void fork_destroy(chain_candidate_t *fork) {
    if (!fork) return;

    chain_destroy(fork->chain);
    ledger_destroy(fork->state);
    free(fork);
}

int fork_add_candidate(consensus_node_t *node, blockchain_t *chain,
                       ledger_t *state) {
    if (!node || !chain || !state) return -1;
    if (node->fork_count >= MAX_CHAINS) return -1;

    chain_candidate_t *fork = fork_create(chain, state);
    if (!fork) return -1;

    fork->next = node->fork_chains;
    node->fork_chains = fork;
    node->fork_count++;

    return 0;
}

blockchain_t *fork_get_longest(consensus_node_t *node) {
    if (!node) return NULL;

    blockchain_t *longest = node->main_chain;

    chain_candidate_t *current = node->fork_chains;
    while (current) {
        if (chain_get_length(current->chain) > chain_get_length(longest)) {
            longest = current->chain;
        }
        current = current->next;
    }

    return longest;
}

void fork_cleanup(consensus_node_t *node) {
    if (!node) return;

    chain_candidate_t *current = node->fork_chains;
    while (current) {
        chain_candidate_t *next = current->next;
        fork_destroy(current);
        current = next;
    }

    node->fork_chains = NULL;
    node->fork_count = 0;
}
