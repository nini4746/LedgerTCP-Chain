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

size_t chain_get_length(const blockchain_t *chain) {
    return chain ? chain->length : 0;
}

block_t *chain_get_head(const blockchain_t *chain) {
    return chain ? chain->head : NULL;
}

blockchain_t *blockchain_clone(const blockchain_t *chain) {
    if (!chain) return NULL;

    blockchain_t *clone = chain_create();
    if (!clone) return NULL;

    block_t *current = chain->head;
    block_t *prev_clone = NULL;

    while (current) {
        block_t *block_clone = malloc(sizeof(block_t));
        if (!block_clone) {
            chain_destroy(clone);
            return NULL;
        }

        memcpy(block_clone, current, sizeof(block_t));
        block_clone->next = NULL;

        if (!clone->head) {
            clone->head = block_clone;
        }
        if (prev_clone) {
            prev_clone->next = block_clone;
        }

        clone->tail = block_clone;
        prev_clone = block_clone;
        clone->length++;
        current = current->next;
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

block_t *blockchain_get_block_by_height(blockchain_t *chain,
                                        uint64_t height) {
    if (!chain) return NULL;

    block_t *current = chain->head;
    while (current) {
        if (current->height == height) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
