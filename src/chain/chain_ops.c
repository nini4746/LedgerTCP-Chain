#include "block.h"

static block_t *block_clone(const block_t *src)
{
	block_t *clone;

	clone = malloc(sizeof(block_t));
	if (clone)
	{
		memcpy(clone, src, sizeof(block_t));
		clone->next = NULL;
	}
	return clone;
}

static int clone_add_block(blockchain_t *clone, block_t *b, block_t **prev)
{
	if (!clone->head)
		clone->head = b;
	if (*prev)
		(*prev)->next = b;
	clone->tail = b;
	*prev = b;
	clone->length++;
	return 0;
}

blockchain_t *blockchain_clone(const blockchain_t *chain)
{
	blockchain_t *clone;
	block_t *prev;
	block_t *cur;
	block_t *b;

	if (!chain)
		return NULL;
	clone = chain_create();
	if (!clone)
		return NULL;
	prev = NULL;
	for (cur = chain->head; cur; cur = cur->next)
	{
		b = block_clone(cur);
		if (!b)
		{
			chain_destroy(clone);
			return NULL;
		}
		clone_add_block(clone, b, &prev);
	}
	return clone;
}

block_t *blockchain_get_block_by_hash(blockchain_t *chain, const hash_t hash)
{
	block_t *current;

	if (!chain)
		return NULL;
	current = chain->head;
	while (current)
	{
		if (memcmp(current->hash, hash, HASH_SIZE) == 0)
			return current;
		current = current->next;
	}
	return NULL;
}
