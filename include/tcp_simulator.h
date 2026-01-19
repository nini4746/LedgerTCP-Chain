#ifndef TCP_SIMULATOR_H
#define TCP_SIMULATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NODES 10
#define MAX_PACKET_SIZE 4096

typedef enum {
    EVENT_DELAY,
    EVENT_DROP,
    EVENT_DUPLICATE,
    EVENT_REORDER
} tcp_event_t;

typedef struct packet {
    uint8_t data[MAX_PACKET_SIZE];
    size_t size;
    int from_node;
    int to_node;
    uint64_t timestamp;
    struct packet *next;
} packet_t;

typedef struct {
    packet_t *head;
    packet_t *tail;
    size_t count;
} packet_queue_t;

typedef struct {
    int node_id;
    packet_queue_t *inbox;
    packet_queue_t *pending;
    bool is_active;
} tcp_node_t;

typedef struct {
    tcp_node_t nodes[MAX_NODES];
    int node_count;
    uint64_t current_time;

    double delay_prob;
    double drop_prob;
    double duplicate_prob;
    double reorder_prob;
} tcp_simulator_t;

tcp_simulator_t *tcp_simulator_create(int node_count);
void tcp_simulator_destroy(tcp_simulator_t *sim);
void tcp_simulator_step(tcp_simulator_t *sim);
void tcp_simulator_set_probabilities(tcp_simulator_t *sim,
                                     double delay, double drop,
                                     double duplicate, double reorder);
void tcp_simulator_process_node(tcp_simulator_t *sim, tcp_node_t *node);

packet_queue_t *queue_create(void);
void queue_destroy(packet_queue_t *queue);
void queue_enqueue(packet_queue_t *queue, packet_t *packet);
packet_t *queue_dequeue(packet_queue_t *queue);
bool queue_is_empty(packet_queue_t *queue);

packet_t *packet_create(int from, int to, const void *data,
                        size_t size, uint64_t timestamp);
void packet_destroy(packet_t *packet);
packet_t *packet_copy(const packet_t *packet);
bool packet_validate(const packet_t *packet);
size_t packet_serialize(const packet_t *packet, void *buffer,
                        size_t buffer_size);

int node_init(tcp_node_t *node, int node_id);
void node_cleanup(tcp_node_t *node);
void node_activate(tcp_node_t *node);
void node_deactivate(tcp_node_t *node);
bool node_get_status(const tcp_node_t *node);

int network_send(tcp_simulator_t *sim, int from, int to,
                 const void *data, size_t size);
int network_receive(tcp_simulator_t *sim, int node_id,
                    void *buffer, size_t buffer_size);
void network_apply_delay(tcp_simulator_t *sim, packet_t *packet);
void network_apply_events(tcp_simulator_t *sim, tcp_node_t *node,
                          packet_t *packet);

double random_double(void);

#endif
