#include "consensus.h"

int consensus_init(void)
{
	random_init();
	return 0;
}

void consensus_cleanup(void)
{
}

int consensus_process_block(consensus_node_t *node, block_t *block)
{
	if (!node || !block)
		return -1;
	if (!validate_block_structure(block))
		return -1;
	if (!validate_block_transactions(block))
		return -1;
	return sync_receive_block(node, block);
}

ledger_t *consensus_get_state(consensus_node_t *node)
{
	return cnode_get_ledger(node);
}

bool consensus_verify(consensus_node_t **nodes, int node_count)
{
	int i;

	if (!nodes || node_count <= 0)
		return false;
	for (i = 0; i < node_count; i++)
	{
		if (!ledger_validate(nodes[i]->ledger))
			return false;
		if (!reorg_validate(nodes[i]->main_chain))
			return false;
	}
	return sync_check_consensus(nodes, node_count);
}
