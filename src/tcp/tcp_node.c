#include "tcp_simulator.h"
#include <stdlib.h>

int node_init(tcp_node_t *node, int node_id)
{
	if (!node || node_id < 0)
		return -1;
	node->node_id = node_id;
	node->inbox = queue_create();
	node->pending = queue_create();
	node->is_active = true;
	if (!node->inbox || !node->pending)
	{
		queue_destroy(node->inbox);
		queue_destroy(node->pending);
		return -1;
	}
	return 0;
}

void node_cleanup(tcp_node_t *node)
{
	if (!node)
		return;
	queue_destroy(node->inbox);
	queue_destroy(node->pending);
	node->inbox = NULL;
	node->pending = NULL;
}

void node_activate(tcp_node_t *node)
{
	if (node)
		node->is_active = true;
}

void node_deactivate(tcp_node_t *node)
{
	if (node)
		node->is_active = false;
}

bool node_get_status(const tcp_node_t *node)
{
	if (!node)
		return false;
	return node->is_active;
}
