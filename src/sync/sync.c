#include "consensus.h"

int sync_start(consensus_node_t *node)
{
	if (!node)
		return -1;
	fork_cleanup(node);
	return 0;
}

int sync_broadcast_block(tcp_simulator_t *sim, int from_node, block_t *block, int num_nodes)
{
	int i;

	if (!sim || !block || from_node < 0)
		return -1;
	for (i = 0; i < num_nodes; i++)
	{
		if (i != from_node)
			network_send(sim, from_node, i, block, sizeof(block_t));
	}
	return 0;
}

bool sync_check_consensus(consensus_node_t **nodes, int node_count)
{
	int i;
	size_t len_0;
	size_t len_i;

	if (!nodes || node_count <= 0)
		return false;
	for (i = 1; i < node_count; i++)
	{
		len_0 = chain_get_length(nodes[0]->main_chain);
		len_i = chain_get_length(nodes[i]->main_chain);
		if (len_0 != len_i)
			return false;
		if (!state_compare(nodes[0]->ledger, nodes[i]->ledger))
			return false;
	}
	return true;
}
