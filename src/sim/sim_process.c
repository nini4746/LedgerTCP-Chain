#include "tcp_simulator.h"

static void process_packet(tcp_simulator_t *sim, tcp_node_t *node, packet_t *pkt, packet_queue_t *temp)
{
	if (pkt->timestamp <= sim->current_time)
	{
		if (random_double() < sim->reorder_prob && !queue_is_empty(node->pending))
			queue_enqueue(node->pending, pkt);
		else
			queue_enqueue(node->inbox, pkt);
	}
	else
		queue_enqueue(temp, pkt);
}

void tcp_simulator_process_node(tcp_simulator_t *sim, tcp_node_t *node)
{
	packet_queue_t *temp;
	packet_t *pkt;

	if (!sim || !node || !node->is_active)
		return;
	temp = queue_create();
	if (!temp)
		return;
	while (!queue_is_empty(node->pending))
	{
		pkt = queue_dequeue(node->pending);
		process_packet(sim, node, pkt, temp);
	}
	while (!queue_is_empty(temp))
		queue_enqueue(node->pending, queue_dequeue(temp));
	queue_destroy(temp);
}
