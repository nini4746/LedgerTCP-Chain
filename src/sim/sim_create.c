#include "tcp_simulator.h"
#include <stdlib.h>
#include <time.h>

static void sim_set_defaults(tcp_simulator_t *sim, int node_count)
{
	sim->node_count = node_count;
	sim->current_time = 0;
	sim->delay_prob = 0.1;
	sim->drop_prob = 0.05;
	sim->duplicate_prob = 0.05;
	sim->reorder_prob = 0.05;
}

static int sim_init_nodes(tcp_simulator_t *sim, int node_count)
{
	int i;

	for (i = 0; i < node_count; i++)
	{
		if (node_init(&sim->nodes[i], i) != 0)
			return -1;
	}
	return 0;
}

tcp_simulator_t *tcp_simulator_create(int node_count)
{
	tcp_simulator_t *sim;

	if (node_count <= 0 || node_count > MAX_NODES)
		return NULL;
	sim = malloc(sizeof(tcp_simulator_t));
	if (!sim)
		return NULL;
	sim_set_defaults(sim, node_count);
	if (sim_init_nodes(sim, node_count) != 0)
	{
		tcp_simulator_destroy(sim);
		return NULL;
	}
	srand(time(NULL));
	return sim;
}
