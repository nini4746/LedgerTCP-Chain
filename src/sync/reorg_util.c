#include "consensus.h"

static void apply_block_txs(ledger_t *ledger, block_t *block)
{
	size_t i;

	for (i = 0; i < block->tx_count; i++)
		transaction_apply(ledger, &block->transactions[i]);
}

ledger_t *reorg_rebuild_ledger(blockchain_t *chain, balance_t genesis_balance)
{
	ledger_t *ledger;
	block_t *block;

	if (!chain)
		return NULL;
	ledger = genesis_init_state_custom(genesis_balance);
	if (!ledger)
		return NULL;
	block = chain_get_head(chain);
	if (block)
		block = block->next;
	while (block)
	{
		apply_block_txs(ledger, block);
		block = block->next;
	}
	return ledger;
}

bool reorg_validate(blockchain_t *chain)
{
	block_t *block;
	block_t *prev;

	if (!chain)
		return false;
	block = chain_get_head(chain);
	prev = NULL;
	while (block)
	{
		if (!validate_block_structure(block))
			return false;
		if (!validate_block_transactions(block))
			return false;
		if (prev && !validate_prev_hash(block, prev))
			return false;
		prev = block;
		block = block->next;
	}
	return true;
}
