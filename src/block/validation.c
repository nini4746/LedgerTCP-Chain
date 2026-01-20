#include "block.h"

bool validate_block_structure(const block_t *block) {
    if (!block) return false;
    if (block->tx_count > MAX_TX_PER_BLOCK) return false;
    return true;
}

bool validate_block_transactions(const block_t *block) {
    if (!block) return false;

    for (size_t i = 0; i < block->tx_count; i++) {
        if (!transaction_validate(&block->transactions[i])) {
            return false;
        }
    }
    return true;
}

bool validate_block_hash(const block_t *block) {
    if (!block) return false;
    if (hash_is_zero(block->hash)) return false;

    block_t temp;
    memcpy(&temp, block, sizeof(block_t));
    block_compute_hash(&temp);

    return hash_equals(temp.hash, block->hash);
}

bool validate_prev_hash(const block_t *block,
                        const block_t *prev_block) {
    if (!block || !prev_block) return false;

    return hash_equals(block->prev_hash, prev_block->hash);
}

void block_compute_hash(block_t *block) {
    if (!block) return;

    uint32_t hash_val = 0;
    hash_val ^= hash_compute_simple(&block->height, sizeof(block->height));
    hash_val ^= hash_compute_simple(block->prev_hash, HASH_SIZE);
    hash_val ^= hash_compute_simple(&block->timestamp, sizeof(block->timestamp));

    for (size_t i = 0; i < block->tx_count; i++) {
        hash_val ^= hash_compute_simple(&block->transactions[i],
                                        sizeof(transaction_t));
    }

    memset(block->hash, 0, HASH_SIZE);
    memcpy(block->hash, &hash_val, sizeof(hash_val));
}
