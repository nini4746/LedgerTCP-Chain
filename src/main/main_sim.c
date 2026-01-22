#include "tcp_simulator.h"
#include "ledger.h"
#include "block.h"
#include "consensus.h"

void print_ledger_state_main(consensus_node_t *node)
{
	ledger_t *ledger;
	blockchain_t *chain;
	account_t *acc;
	size_t i;

	ledger = cnode_get_ledger(node);
	chain = cnode_get_chain(node);
	printf("Node %d Ledger State:\n", node->node_id);
	printf("  Chain length: %zu\n", chain_get_length(chain));
	for (i = 0; i < ledger->account_count; i++)
	{
		acc = &ledger->accounts[i];
		printf("  Account %lu: Balance = %ld\n", acc->account_id, acc->balance);
	}
	printf("\n");
}

bool finalize_and_verify_main(consensus_node_t **nodes, int num_nodes)
{
	bool result;
	int i;

	printf("Finalizing consensus...\n");
	sync_finalize(nodes, num_nodes);
	printf("\n=== Final Ledger States ===\n\n");
	for (i = 0; i < num_nodes; i++)
		print_ledger_state_main(nodes[i]);
	printf("Verifying consensus...\n");
	result = consensus_verify(nodes, num_nodes);
	if (result)
		printf("SUCCESS: All nodes reached consensus!\n");
	else
		printf("FAIL: Consensus not reached\n");
	printf("\n");
	return result;
}

void cleanup_all_main(consensus_node_t **nodes, tcp_simulator_t *sim, int num_nodes)
{
	int i;

	for (i = 0; i < num_nodes; i++)
		cnode_destroy(nodes[i]);
	tcp_simulator_destroy(sim);
}
