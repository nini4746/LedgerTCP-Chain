#include "block.h"

block_t *block_create(const hash_t prev_hash, uint64_t height) {
    block_t *block = malloc(sizeof(block_t));
    if (!block) return NULL;

    block->height = height;
    hash_copy(block->prev_hash, prev_hash);
    hash_zero(block->hash);
    block->tx_count = 0;
    block->timestamp = (uint64_t)time(NULL);
    block->next = NULL;

    return block;
}

void block_destroy(block_t *block) {
    free(block);
}

int block_add_transaction(block_t *block, const transaction_t *tx) {
    if (!block || !tx) return -1;
    if (block->tx_count >= MAX_TX_PER_BLOCK) return -1;

    memcpy(&block->transactions[block->tx_count++], tx,
           sizeof(transaction_t));
    return 0;
}

bool block_validate_structure(const block_t *block) {
    if (!block) return false;
    if (block->tx_count > MAX_TX_PER_BLOCK) return false;
    if (hash_is_zero(block->hash)) return false;
    return true;
}

size_t block_serialize(const block_t *block, void *buffer,
                       size_t buffer_size) {
    if (!block || !buffer) return 0;

    size_t needed = sizeof(block_t);
    if (buffer_size < needed) return 0;

    memcpy(buffer, block, needed);
    return needed;
}
