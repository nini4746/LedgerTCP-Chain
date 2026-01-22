#include "tcp_simulator.h"
#include <stdlib.h>
#include <string.h>

packet_t *packet_create(int from, int to, const void *data,
						size_t size, uint64_t timestamp)
{
	packet_t *packet;

	if (!data || size == 0 || size > MAX_PACKET_SIZE)
		return NULL;

	packet = malloc(sizeof(packet_t));
	if (!packet)
		return NULL;
	memcpy(packet->data, data, size);
	packet->size = size;
	packet->from_node = from;
	packet->to_node = to;
	packet->timestamp = timestamp;
	packet->next = NULL;

	return packet;
}

void packet_destroy(packet_t *packet)
{
	free(packet);
}

packet_t *packet_copy(const packet_t *packet)
{
	packet_t *copy;

	if (!packet)
		return NULL;
	copy = malloc(sizeof(packet_t));
	if (!copy)
		return NULL;
	memcpy(copy, packet, sizeof(packet_t));
	copy->next = NULL;
	return copy;
}

bool packet_validate(const packet_t *packet)
{
	if (!packet)
		return false;
	if (packet->size == 0)
		return false;
	if (packet->size > MAX_PACKET_SIZE)
		return false;
	if (packet->from_node < 0)
		return false;
	if (packet->to_node < 0)
		return false;
	return true;
}

size_t packet_serialize(const packet_t *packet, void *buffer,
						size_t buffer_size)
{
	size_t copy_size;

	if (!packet || !buffer)
		return 0;
	copy_size = packet->size;
	if (copy_size > buffer_size)
		copy_size = buffer_size;
	memcpy(buffer, packet->data, copy_size);
	return copy_size;
}
