#include "tcp_simulator.h"
#include <stdlib.h>
#include <string.h>

int network_send(tcp_simulator_t *sim, int from, int to,
                 const void *data, size_t size) {
    if (!sim || !data || size == 0) return -1;
    if (from < 0 || from >= sim->node_count) return -1;
    if (to < 0 || to >= sim->node_count) return -1;
    if (!sim->nodes[from].is_active) return -1;
    if (!sim->nodes[to].is_active) return -1;

    packet_t *packet = packet_create(from, to, data, size,
                                     sim->current_time);
    if (!packet) return -1;

    network_apply_events(sim, &sim->nodes[to], packet);
    return 0;
}

int network_receive(tcp_simulator_t *sim, int node_id,
                    void *buffer, size_t buffer_size) {
    if (!sim || !buffer || buffer_size == 0) return -1;
    if (node_id < 0 || node_id >= sim->node_count) return -1;
    if (!sim->nodes[node_id].is_active) return -1;

    packet_t *packet = queue_dequeue(sim->nodes[node_id].inbox);
    if (!packet) return 0;

    size_t sent = packet_serialize(packet, buffer, buffer_size);
    packet_destroy(packet);
    return sent;
}

void network_apply_delay(tcp_simulator_t *sim, packet_t *packet) {
    if (!sim || !packet) return;

    double rand_val = random_double();
    if (rand_val < sim->delay_prob) {
        packet->timestamp += (rand() % 100) + 10;
    }
}

void network_apply_events(tcp_simulator_t *sim, tcp_node_t *node,
                          packet_t *packet) {
    if (!sim || !node || !packet) return;

    if (random_double() < sim->drop_prob) {
        packet_destroy(packet);
        return;
    }

    if (random_double() < sim->duplicate_prob) {
        packet_t *dup = packet_copy(packet);
        if (dup) queue_enqueue(node->pending, dup);
    }

    network_apply_delay(sim, packet);
    queue_enqueue(node->pending, packet);
}
