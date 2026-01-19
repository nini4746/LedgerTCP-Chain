#include "tcp_simulator.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int failed = 0;

    tcp_simulator_t *sim = tcp_simulator_create(3);
    if (!sim) { printf("FAIL: Simulator creation\n"); return 1; }

    if (tcp_simulator_create(0) != NULL) { printf("FAIL: Should reject 0 nodes\n"); failed = 1; }
    if (tcp_simulator_create(MAX_NODES + 1) != NULL) { printf("FAIL: Should reject too many nodes\n"); failed = 1; }

    tcp_simulator_set_probabilities(sim, 0.5, 0.3, 0.2, 0.1);

    char data[] = "test";
    if (network_send(sim, 0, 1, data, strlen(data) + 1) != 0) { printf("FAIL: Send\n"); failed = 1; }
    if (network_send(sim, 0, 10, data, strlen(data)) == 0) { printf("FAIL: Invalid node check\n"); failed = 1; }

    for (int i = 0; i < 100; i++) tcp_simulator_step(sim);

    char buffer[256];
    int received = network_receive(sim, 1, buffer, sizeof(buffer));
    if (received < 0) { printf("FAIL: Receive error\n"); failed = 1; }

    packet_queue_t *queue = queue_create();
    if (!queue || !queue_is_empty(queue)) { printf("FAIL: Queue\n"); failed = 1; }

    packet_t *pkt = packet_create(0, 1, data, strlen(data) + 1, 0);
    if (!pkt) { printf("FAIL: Packet creation\n"); failed = 1; }

    queue_enqueue(queue, pkt);
    if (queue_is_empty(queue)) { printf("FAIL: Queue empty after enqueue\n"); failed = 1; }

    packet_t *dequeued = queue_dequeue(queue);
    if (!dequeued) { printf("FAIL: Dequeue\n"); failed = 1; }
    else packet_destroy(dequeued);

    queue_destroy(queue);
    tcp_simulator_destroy(sim);

    if (failed) return 1;
    return 0;
}
