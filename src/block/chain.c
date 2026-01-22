#include "block.h"

blockchain_t *chain_create(void)
{
	blockchain_t *chain;

	chain = malloc(sizeof(blockchain_t));
	if (!chain)
	{
		return NULL;
	}
	chain->head = NULL;
	chain->tail = NULL;
	chain->length = 0;
	return chain;
}

void chain_destroy(blockchain_t *chain)
{
	block_t *current;
	block_t *next;

	if (!chain)
	{
		return;
	}
	current = chain->head;
	while (current)
	{
		next = current->next;
		block_destroy(current);
		current = next;
	}
	free(chain);
}

int chain_add_block(blockchain_t *chain, block_t *block)
{
	if (!chain || !block)
		return -1;
	if (chain->tail && memcmp(block->prev_hash, chain->tail->hash, HASH_SIZE) != 0)
		return -1;
	if (chain->tail)
		chain->tail->next = block;
	else
		chain->head = block;
	chain->tail = block;
	chain->length++;
	return 0;
}
