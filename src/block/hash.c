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
