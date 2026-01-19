#include "block.h"

void hash_copy(hash_t dest, const hash_t src) {
    memcpy(dest, src, HASH_SIZE);
}

bool hash_equals(const hash_t a, const hash_t b) {
    return memcmp(a, b, HASH_SIZE) == 0;
}

void hash_zero(hash_t hash) {
    memset(hash, 0, HASH_SIZE);
}

bool hash_is_zero(const hash_t hash) {
    for (int i = 0; i < HASH_SIZE; i++) {
        if (hash[i] != 0) return false;
    }
    return true;
}

uint32_t hash_compute_simple(const void *data, size_t len) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = 5381;

    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + bytes[i];
    }

    return hash;
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

    for (int i = sizeof(hash_val); i < HASH_SIZE; i++) {
        block->hash[i] = (uint8_t)((hash_val >> ((i % 4) * 8)) & 0xFF);
    }
}
