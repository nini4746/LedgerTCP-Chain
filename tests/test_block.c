#include "block.h"
#include <stdio.h>

int main(void) {
    int failed = 0;

    hash_t hash1, hash2;
    hash_zero(hash1);
    if (!hash_is_zero(hash1)) { printf("FAIL: Hash zero\n"); failed = 1; }

    hash_copy(hash2, hash1);
    if (!hash_equals(hash1, hash2)) { printf("FAIL: Hash copy\n"); failed = 1; }

    block_t *genesis = genesis_create();
    if (!genesis || !genesis_validate(genesis) || genesis->height != 0) {
        printf("FAIL: Genesis\n"); failed = 1;
        if (genesis) block_destroy(genesis);
        return 1;
    }

    block_t *block1 = block_create(genesis->hash, 1);
    if (!block1) { printf("FAIL: Block creation\n"); failed = 1; block_destroy(genesis); return 1; }

    transaction_t tx = {1, 1, 2, 100};
    if (block_add_transaction(block1, &tx) != 0 || block1->tx_count != 1) {
        printf("FAIL: Add transaction\n"); failed = 1;
    }

    block_compute_hash(block1);
    if (hash_is_zero(block1->hash)) { printf("FAIL: Hash compute\n"); failed = 1; }

    if (!validate_block_structure(block1)) { printf("FAIL: Block structure\n"); failed = 1; }
    if (!validate_prev_hash(block1, genesis)) { printf("FAIL: Prev hash\n"); failed = 1; }

    blockchain_t *chain = chain_create();
    if (!chain) { printf("FAIL: Chain creation\n"); failed = 1; }

    if (chain_add_block(chain, genesis) != 0) { printf("FAIL: Add genesis\n"); failed = 1; }
    if (chain_add_block(chain, block1) != 0) { printf("FAIL: Add block\n"); failed = 1; }
    if (chain_get_length(chain) != 2) { printf("FAIL: Chain length\n"); failed = 1; }

    block_t *found = blockchain_get_block_by_hash(chain, block1->hash);
    if (!found || found != block1) { printf("FAIL: Find by hash\n"); failed = 1; }

    chain_destroy(chain);
    if (failed) return 1;
    return 0;
}
