#include "block.h"

block_t *block_create(const hash_t prev_hash, uint64_t height)
{
	block_t *block;
	time_t now;

	block = malloc(sizeof(block_t));
	if (!block)
		return NULL;
	now = time(NULL);
	if (now == (time_t)-1)
	{
		free(block);
		return NULL;
	}
	block->height = height;
	memcpy(block->prev_hash, prev_hash, HASH_SIZE);
	memset(block->hash, 0, HASH_SIZE);
	block->tx_count = 0;
	block->timestamp = (uint64_t)now;
	block->next = NULL;
	return block;
}

void block_destroy(block_t *block)
{
	free(block);
}

int block_add_transaction(block_t *block, const transaction_t *tx)
{
	if (!block || !tx)
		return -1;
	if (block->tx_count >= MAX_TX_PER_BLOCK)
		return -1;
	memcpy(&block->transactions[block->tx_count++], tx, sizeof(transaction_t));
	return 0;
}

bool block_validate_structure(const block_t *block)
{
	if (!block)
		return false;
	if (block->tx_count > MAX_TX_PER_BLOCK)
		return false;
	if (hash_is_zero(block->hash))
		return false;
	return true;
}

size_t block_serialize(const block_t *block, void *buffer, size_t buf_size)
{
	size_t needed;

	if (!block || !buffer)
		return 0;
	needed = sizeof(block_t);
	if (buf_size < needed)
		return 0;
	memcpy(buffer, block, needed);
	return needed;
}
