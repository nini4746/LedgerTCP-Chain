#include "consensus.h"

static void update_longest(blockchain_t **longest, size_t *max_len, blockchain_t *chain)
{
	size_t length;

	length = chain_get_length(chain);
	if (length > *max_len)
	{
		*max_len = length;
		*longest = chain;
	}
	else if (length == *max_len && *longest && chain->tail)
	{
		if (memcmp(chain->tail->hash, (*longest)->tail->hash, HASH_SIZE) < 0)
			*longest = chain;
	}
}

static blockchain_t *find_longest_chain(consensus_node_t **nodes, int count)
{
	blockchain_t *longest;
	blockchain_t *chain;
	size_t max_length;
	int i;

	longest = NULL;
	max_length = 0;
	for (i = 0; i < count; i++)
	{
		cnode_update_state(nodes[i]);
		chain = fork_get_longest(nodes[i]);
		update_longest(&longest, &max_length, chain);
	}
	return longest;
}

int sync_finalize(consensus_node_t **nodes, int node_count)
{
	blockchain_t *longest;
	int i;

	if (!nodes || node_count <= 0)
		return -1;
	longest = find_longest_chain(nodes, node_count);
	if (!longest)
		return -1;
	for (i = 0; i < node_count; i++)
	{
		if (nodes[i]->main_chain != longest)
		{
			if (reorg_execute(nodes[i], longest) != 0)
				return -1;
		}
	}
	return 0;
}
