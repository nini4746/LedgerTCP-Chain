#include "tcp_simulator.h"
#include <stdlib.h>

void tcp_simulator_destroy(tcp_simulator_t *sim)
{
	int i;

	if (!sim)
		return;
	for (i = 0; i < sim->node_count; i++)
		node_cleanup(&sim->nodes[i]);
	free(sim);
}

void tcp_simulator_step(tcp_simulator_t *sim)
{
	int i;

	if (!sim)
		return;
	sim->current_time++;
	for (i = 0; i < sim->node_count; i++)
		tcp_simulator_process_node(sim, &sim->nodes[i]);
}

void tcp_simulator_set_probabilities(tcp_simulator_t *sim,
									 double delay, double drop,
									 double duplicate, double reorder)
{
	if (!sim)
		return;
	sim->delay_prob = delay;
	sim->drop_prob = drop;
	sim->duplicate_prob = duplicate;
	sim->reorder_prob = reorder;
}
