#include "consensus.h"

blockchain_t *cnode_get_chain(consensus_node_t *node)
{
	if (!node)
		return NULL;
	return node->main_chain;
}

ledger_t *cnode_get_ledger(consensus_node_t *node)
{
	if (!node)
		return NULL;
	return node->ledger;
}

int cnode_update_state(consensus_node_t *node)
{
	blockchain_t *longest;

	if (!node)
		return -1;
	longest = fork_get_longest(node);
	if (longest != node->main_chain)
		return reorg_execute(node, longest);
	return 0;
}
