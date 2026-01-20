#include "block.h"

blockchain_t *chain_create(void) {
    blockchain_t *chain = malloc(sizeof(blockchain_t));
    if (!chain) return NULL;

    chain->head = NULL;
    chain->tail = NULL;
    chain->length = 0;

    return chain;
}

void chain_destroy(blockchain_t *chain) {
    if (!chain) return;

    block_t *current = chain->head;
    while (current) {
        block_t *next = current->next;
        block_destroy(current);
        current = next;
    }

    free(chain);
}

int chain_add_block(blockchain_t *chain, block_t *block) {
    if (!chain || !block) return -1;

    if (chain->tail && !hash_equals(block->prev_hash,
                                    chain->tail->hash)) {
        return -1;
    }

    if (chain->tail) {
        chain->tail->next = block;
    } else {
        chain->head = block;
    }

    chain->tail = block;
    chain->length++;
    return 0;
}

static block_t *block_clone(const block_t *src) {
    block_t *clone = malloc(sizeof(block_t));
    if (clone) { memcpy(clone, src, sizeof(block_t)); clone->next = NULL; }
    return clone;
}

blockchain_t *blockchain_clone(const blockchain_t *chain) {
    if (!chain) return NULL;

    blockchain_t *clone = chain_create();
    if (!clone) return NULL;

    block_t *prev = NULL;
    for (block_t *cur = chain->head; cur; cur = cur->next) {
        block_t *b = block_clone(cur);
        if (!b) { chain_destroy(clone); return NULL; }

        if (!clone->head) clone->head = b;
        if (prev) prev->next = b;
        clone->tail = b;
        prev = b;
        clone->length++;
    }
    return clone;
}

block_t *blockchain_get_block_by_hash(blockchain_t *chain,
                                      const hash_t hash) {
    if (!chain) return NULL;

    block_t *current = chain->head;
    while (current) {
        if (hash_equals(current->hash, hash)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
