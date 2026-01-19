#include "tcp_simulator.h"
#include <stdlib.h>
#include <time.h>

tcp_simulator_t *tcp_simulator_create(int node_count) {
    if (node_count <= 0 || node_count > MAX_NODES) {
        return NULL;
    }

    tcp_simulator_t *sim = malloc(sizeof(tcp_simulator_t));
    if (!sim) return NULL;

    sim->node_count = node_count;
    sim->current_time = 0;
    sim->delay_prob = 0.1;
    sim->drop_prob = 0.05;
    sim->duplicate_prob = 0.05;
    sim->reorder_prob = 0.05;

    for (int i = 0; i < node_count; i++) {
        if (node_init(&sim->nodes[i], i) != 0) {
            tcp_simulator_destroy(sim);
            return NULL;
        }
    }

    srand(time(NULL));
    return sim;
}

void tcp_simulator_destroy(tcp_simulator_t *sim) {
    if (!sim) return;

    for (int i = 0; i < sim->node_count; i++) {
        node_cleanup(&sim->nodes[i]);
    }
    free(sim);
}

void tcp_simulator_step(tcp_simulator_t *sim) {
    if (!sim) return;

    sim->current_time++;

    for (int i = 0; i < sim->node_count; i++) {
        tcp_simulator_process_node(sim, &sim->nodes[i]);
    }
}

void tcp_simulator_set_probabilities(tcp_simulator_t *sim,
                                     double delay, double drop,
                                     double duplicate, double reorder) {
    if (!sim) return;

    sim->delay_prob = delay;
    sim->drop_prob = drop;
    sim->duplicate_prob = duplicate;
    sim->reorder_prob = reorder;
}

void tcp_simulator_process_node(tcp_simulator_t *sim, tcp_node_t *node) {
    if (!sim || !node || !node->is_active) return;

    packet_queue_t *temp = queue_create();
    if (!temp) return;

    while (!queue_is_empty(node->pending)) {
        packet_t *pkt = queue_dequeue(node->pending);
        if (pkt->timestamp <= sim->current_time) {
            if (random_double() < sim->reorder_prob &&
                !queue_is_empty(node->pending)) {
                queue_enqueue(node->pending, pkt);
                continue;
            }
            queue_enqueue(node->inbox, pkt);
        } else {
            queue_enqueue(temp, pkt);
        }
    }

    while (!queue_is_empty(temp)) {
        queue_enqueue(node->pending, queue_dequeue(temp));
    }
    queue_destroy(temp);
}
