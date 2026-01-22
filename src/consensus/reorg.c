#include "consensus.h"

int reorg_rollback(consensus_node_t *node)
{
	if (!node)
		return -1;
	ledger_destroy(node->ledger);
	node->ledger = ledger_create();
	if (!node->ledger)
		return -1;
	return 0;
}

int reorg_commit(consensus_node_t *node, blockchain_t *new_chain, ledger_t *new_ledger)
{
	blockchain_t *old_chain;
	ledger_t *old_ledger;

	if (!node || !new_chain || !new_ledger)
		return -1;
	old_chain = node->main_chain;
	old_ledger = node->ledger;
	node->main_chain = new_chain;
	node->ledger = new_ledger;
	chain_destroy(old_chain);
	ledger_destroy(old_ledger);
	return 0;
}

int reorg_execute(consensus_node_t *node, blockchain_t *new_chain)
{
	ledger_t *new_ledger;
	blockchain_t *chain_copy;

	if (!node || !new_chain)
		return -1;
	if (!reorg_validate(new_chain))
		return -1;
	new_ledger = reorg_rebuild_ledger(new_chain, node->genesis_balance);
	if (!new_ledger)
		return -1;
	chain_copy = blockchain_clone(new_chain);
	if (!chain_copy)
	{
		ledger_destroy(new_ledger);
		return -1;
	}
	return reorg_commit(node, chain_copy, new_ledger);
}
