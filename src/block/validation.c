#include "block.h"

bool validate_block_structure(const block_t *block)
{
	if (!block)
	{
		return false;
	}
	if (block->tx_count > MAX_TX_PER_BLOCK)
	{
		return false;
	}
	return true;
}

bool validate_block_transactions(const block_t *block)
{
	size_t i;

	if (!block)
	{
		return false;
	}
	for (i = 0; i < block->tx_count; i++)
	{
		if (!transaction_validate(&block->transactions[i]))
		{
			return false;
		}
	}
	return true;
}

bool validate_block_hash(const block_t *block)
{
	block_t temp;

	if (!block)
	{
		return false;
	}
	if (hash_is_zero(block->hash))
	{
		return false;
	}
	memcpy(&temp, block, sizeof(block_t));
	block_compute_hash(&temp);
	return memcmp(temp.hash, block->hash, HASH_SIZE) == 0;
}

bool validate_prev_hash(const block_t *block, const block_t *prev_block)
{
	if (!block || !prev_block)
		return false;
	return memcmp(block->prev_hash, prev_block->hash, HASH_SIZE) == 0;
}

void block_compute_hash(block_t *block)
{
	uint32_t hash_val;
	size_t i;

	if (!block)
		return;
	hash_val = 0;
	hash_val ^= hash_compute_simple(&block->height, sizeof(block->height));
	hash_val ^= hash_compute_simple(block->prev_hash, HASH_SIZE);
	hash_val ^= hash_compute_simple(&block->timestamp, sizeof(block->timestamp));
	for (i = 0; i < block->tx_count; i++)
		hash_val ^= hash_compute_simple(&block->transactions[i], sizeof(transaction_t));
	memset(block->hash, 0, HASH_SIZE);
	memcpy(block->hash, &hash_val, sizeof(hash_val));
}
