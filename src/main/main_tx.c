#include "tcp_simulator.h"
#include "ledger.h"
#include "block.h"
#include "consensus.h"

transaction_t make_tx(int idx)
{
	transaction_t tx;

	tx.tx_id = idx + 1;
	tx.from = (idx % 3) + 1;
	tx.to = ((idx + 1) % 3) + 1;
	tx.amount = 10 + (idx * 5);
	return tx;
}

void broadcast_block_main(consensus_node_t **nodes, tcp_simulator_t *sim,
						  int idx, transaction_t *tx, int num_nodes)
{
	blockchain_t *chain;
	block_t *new_block;
	int creator_node;

	creator_node = idx % num_nodes;
	chain = cnode_get_chain(nodes[creator_node]);
	new_block = block_create(chain->tail->hash, chain->tail->height + 1);
	if (!new_block)
		return;
	block_add_transaction(new_block, tx);
	block_compute_hash(new_block);
	consensus_process_block(nodes[creator_node], new_block);
	sync_broadcast_block(sim, creator_node, new_block, num_nodes);
	free(new_block);
}

void create_broadcast_tx_main(consensus_node_t **nodes, tcp_simulator_t *sim,
							  int idx, int num_nodes)
{
	transaction_t tx;

	tx = make_tx(idx);
	printf("TX %lu: %lu -> %lu, amount=%ld\n", tx.tx_id, tx.from, tx.to, tx.amount);
	broadcast_block_main(nodes, sim, idx, &tx, num_nodes);
}

void run_step_main(consensus_node_t **nodes, tcp_simulator_t *sim,
				   int step, int num_nodes)
{
	block_t received_block;
	int received;
	int i;

	tcp_simulator_step(sim);
	for (i = 0; i < num_nodes; i++)
	{
		received = network_receive(sim, i, &received_block, sizeof(block_t));
		if (received > 0)
			consensus_process_block(nodes[i], &received_block);
	}
	if (step % 20 == 0)
		printf("Step %d: Processing blocks...\n", step);
}
