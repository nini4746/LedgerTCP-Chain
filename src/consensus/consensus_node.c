#include "consensus.h"

static void cnode_init_fields(consensus_node_t *node, int id, balance_t bal)
{
	node->node_id = id;
	node->genesis_balance = bal;
	node->main_chain = chain_create();
	node->ledger = genesis_init_state_custom(bal);
	node->fork_chains = NULL;
	node->fork_count = 0;
}

consensus_node_t *cnode_create(int node_id, balance_t genesis_balance)
{
	consensus_node_t *node;
	block_t *genesis;

	node = malloc(sizeof(consensus_node_t));
	if (!node)
		return NULL;
	cnode_init_fields(node, node_id, genesis_balance);
	if (!node->main_chain || !node->ledger)
	{
		cnode_destroy(node);
		return NULL;
	}
	genesis = genesis_create();
	if (!genesis)
	{
		cnode_destroy(node);
		return NULL;
	}
	chain_add_block(node->main_chain, genesis);
	return node;
}

void cnode_destroy(consensus_node_t *node)
{
	if (!node)
		return;
	chain_destroy(node->main_chain);
	ledger_destroy(node->ledger);
	fork_cleanup(node);
	free(node);
}
