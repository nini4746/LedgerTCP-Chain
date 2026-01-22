#ifndef BLOCK_H
#define BLOCK_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "ledger.h"

#define MAX_TX_PER_BLOCK 100
#define HASH_SIZE 32

typedef uint8_t hash_t[HASH_SIZE];

typedef struct block {
    uint64_t height;
    hash_t prev_hash;
    hash_t hash;
    transaction_t transactions[MAX_TX_PER_BLOCK];
    size_t tx_count;
    uint64_t timestamp;
    struct block *next;
} block_t;

typedef struct {
    block_t *head;
    block_t *tail;
    size_t length;
} blockchain_t;

block_t *block_create(const hash_t prev_hash, uint64_t height);
void block_destroy(block_t *block);
int block_add_transaction(block_t *block, const transaction_t *tx);
bool block_validate_structure(const block_t *block);
size_t block_serialize(const block_t *block, void *buffer, size_t buffer_size);

blockchain_t *chain_create(void);
void chain_destroy(blockchain_t *chain);
int chain_add_block(blockchain_t *chain, block_t *block);

static inline size_t chain_get_length(const blockchain_t *chain) {
    return chain ? chain->length : 0;
}

static inline block_t *chain_get_head(const blockchain_t *chain) {
    return chain ? chain->head : NULL;
}

bool hash_is_zero(const hash_t hash);
uint32_t hash_compute_simple(const void *data, size_t len);

block_t *genesis_create(void);
bool genesis_validate(const block_t *block);
void genesis_get_hash(const block_t *genesis, hash_t hash);
ledger_t *genesis_init_state_custom(balance_t initial_balance);
int genesis_export(const block_t *genesis, const char *filename);

bool validate_block_structure(const block_t *block);
bool validate_block_transactions(const block_t *block);
bool validate_block_hash(const block_t *block);
bool validate_prev_hash(const block_t *block, const block_t *prev_block);

void block_compute_hash(block_t *block);
block_t *blockchain_get_block_by_hash(blockchain_t *chain, const hash_t hash);
blockchain_t *blockchain_clone(const blockchain_t *chain);

#endif
