#include "consensus.h"

static int add_block_to_main(consensus_node_t *node, block_t *block)
{
	block_t *new_block;
	size_t i;

	new_block = malloc(sizeof(block_t));
	if (!new_block)
		return -1;
	memcpy(new_block, block, sizeof(block_t));
	new_block->next = NULL;
	if (chain_add_block(node->main_chain, new_block) != 0)
	{
		free(new_block);
		return -1;
	}
	for (i = 0; i < new_block->tx_count; i++)
		transaction_apply(node->ledger, &new_block->transactions[i]);
	return 0;
}

static block_t *clone_block(block_t *src)
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

static int copy_chain_until(blockchain_t *fork, block_t *head, block_t *end)
{
	block_t *cur;
	block_t *cloned;

	for (cur = head; cur && cur != end; cur = cur->next)
	{
		cloned = clone_block(cur);
		if (!cloned)
			return -1;
		chain_add_block(fork, cloned);
	}
	return 0;
}

static int create_fork_chain(consensus_node_t *node, block_t *block, block_t *parent)
{
	blockchain_t *fork_chain;
	block_t *new_block;
	ledger_t *fork_state;

	fork_chain = chain_create();
	if (!fork_chain)
		return -1;
	if (copy_chain_until(fork_chain, node->main_chain->head, parent->next) != 0)
	{
		chain_destroy(fork_chain);
		return -1;
	}
	new_block = clone_block(block);
	if (!new_block)
	{
		chain_destroy(fork_chain);
		return -1;
	}
	chain_add_block(fork_chain, new_block);
	fork_state = reorg_rebuild_ledger(fork_chain, node->genesis_balance);
	if (!fork_state)
	{
		chain_destroy(fork_chain);
		return -1;
	}
	fork_add_candidate(node, fork_chain, fork_state);
	return 0;
}

int sync_receive_block(consensus_node_t *node, block_t *block)
{
	block_t *parent;

	if (!node || !block)
		return -1;
	if (node->main_chain->tail && memcmp(block->prev_hash, node->main_chain->tail->hash, HASH_SIZE) == 0)
		return add_block_to_main(node, block);
	parent = blockchain_get_block_by_hash(node->main_chain, block->prev_hash);
	if (parent && node->fork_count < MAX_CHAINS)
		return create_fork_chain(node, block, parent);
	return -1;
}
