#include "tcp_simulator.h"
#include <stdlib.h>

packet_queue_t *queue_create(void)
{
	packet_queue_t *queue;

	queue = malloc(sizeof(packet_queue_t));
	if (!queue)
		return NULL;
	queue->head = NULL;
	queue->tail = NULL;
	queue->count = 0;
	return queue;
}

void queue_destroy(packet_queue_t *queue)
{
	packet_t *current;
	packet_t *next;

	if (!queue)
		return;
	current = queue->head;
	while (current)
	{
		next = current->next;
		free(current);
		current = next;
	}
	free(queue);
}

void queue_enqueue(packet_queue_t *queue, packet_t *packet)
{
	if (!queue || !packet)
		return;
	packet->next = NULL;
	if (queue->tail)
	{
		queue->tail->next = packet;
		queue->tail = packet;
	}
	else
	{
		queue->head = packet;
		queue->tail = packet;
	}
	queue->count++;
}

packet_t *queue_dequeue(packet_queue_t *queue)
{
	packet_t *packet;

	if (!queue || !queue->head)
		return NULL;
	packet = queue->head;
	queue->head = packet->next;
	if (!queue->head)
		queue->tail = NULL;
	queue->count--;
	packet->next = NULL;
	return packet;
}

bool queue_is_empty(packet_queue_t *queue)
{
	return !queue || queue->count == 0;
}
