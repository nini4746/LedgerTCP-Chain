#include "tcp_simulator.h"
#include "ledger.h"
#include "block.h"
#include "consensus.h"
#include "utils.h"

#define NUM_NODES 3
#define NUM_TRANSACTIONS 10
#define SIMULATION_STEPS 100

extern void create_broadcast_tx_main(consensus_node_t **nodes, tcp_simulator_t *sim,
									 int idx, int num_nodes);
extern void run_step_main(consensus_node_t **nodes, tcp_simulator_t *sim,
						  int step, int num_nodes);
extern bool finalize_and_verify_main(consensus_node_t **nodes, int num_nodes);
extern void cleanup_all_main(consensus_node_t **nodes, tcp_simulator_t *sim, int num_nodes);

static int init_nodes(consensus_node_t **nodes, tcp_simulator_t *sim)
{
	int i;

	for (i = 0; i < NUM_NODES; i++)
	{
		nodes[i] = cnode_create(i, 1000);
		if (!nodes[i])
		{
			fprintf(stderr, "Failed to create consensus node %d\n", i);
			tcp_simulator_destroy(sim);
			return -1;
		}
	}
	return 0;
}

static void run_transactions(consensus_node_t **nodes, tcp_simulator_t *sim)
{
	int i;

	printf("Creating and broadcasting transactions...\n");
	for (i = 0; i < NUM_TRANSACTIONS; i++)
		create_broadcast_tx_main(nodes, sim, i, NUM_NODES);
	printf("\n");
	printf("Running simulation for %d steps...\n", SIMULATION_STEPS);
	for (i = 0; i < SIMULATION_STEPS; i++)
		run_step_main(nodes, sim, i, NUM_NODES);
	printf("\n");
}

int main(void)
{
	tcp_simulator_t *sim;
	consensus_node_t *nodes[NUM_NODES];
	bool consensus_reached;

	printf("=== LedgerTCP-Chain Simulation ===\n\n");
	sim = tcp_simulator_create(NUM_NODES);
	if (!sim)
	{
		fprintf(stderr, "Failed to create TCP simulator\n");
		return 1;
	}
	tcp_simulator_set_probabilities(sim, 0.2, 0.1, 0.1, 0.1);
	printf("TCP Simulator initialized with %d nodes\n", NUM_NODES);
	printf("Probabilities: delay=0.2, drop=0.1, dup=0.1, reorder=0.1\n\n");
	if (init_nodes(nodes, sim) != 0)
		return 1;
	printf("Initialized %d consensus nodes with genesis balances\n\n", NUM_NODES);
	run_transactions(nodes, sim);
	consensus_reached = finalize_and_verify_main(nodes, NUM_NODES);
	cleanup_all_main(nodes, sim, NUM_NODES);
	printf("=== Simulation Complete ===\n");
	return consensus_reached ? 0 : 1;
}
