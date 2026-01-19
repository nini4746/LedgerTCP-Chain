#include "block.h"

block_t *genesis_create(void) {
    hash_t zero_hash;
    hash_zero(zero_hash);

    block_t *genesis = block_create(zero_hash, 0);
    if (!genesis) return NULL;

    block_compute_hash(genesis);
    return genesis;
}

bool genesis_validate(const block_t *block) {
    if (!block) return false;
    if (block->height != 0) return false;
    if (!hash_is_zero(block->prev_hash)) return false;
    if (block->tx_count != 0) return false;
    return true;
}

void genesis_get_hash(const block_t *genesis, hash_t hash) {
    if (!genesis) {
        hash_zero(hash);
        return;
    }
    hash_copy(hash, genesis->hash);
}

ledger_t *genesis_init_state(void) {
    ledger_t *ledger = ledger_create();
    if (!ledger) return NULL;

    account_set_balance(ledger, 1, 10000000);
    account_set_balance(ledger, 2, 10000000);
    account_set_balance(ledger, 3, 10000000);

    return ledger;
}

int genesis_export(const block_t *genesis, const char *filename) {
    if (!genesis || !filename) return -1;

    FILE *f = fopen(filename, "w");
    if (!f) return -1;

    fprintf(f, "Genesis Block\n");
    fprintf(f, "=============\n");
    fprintf(f, "Height: %lu\n", genesis->height);
    fprintf(f, "Timestamp: %lu\n", genesis->timestamp);

    fclose(f);
    return 0;
}
