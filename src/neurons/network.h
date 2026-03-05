/*
 * network.h — Spiking Neural Network
 * Rethink AI — Phase 1
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "neuron.h"
#include "synapse.h"

#define NET_MAX_NEURONS  256
#define NET_MAX_SYNAPSES 4096
#define NET_MAX_DELAY    20

typedef struct {
    int target;         /* Target neuron */
    float weight;       /* Weight at time of spike */
    int deliver_at;     /* Time step to deliver */
} PendingSpike;

typedef struct {
    Neuron neurons[NET_MAX_NEURONS];
    int num_neurons;
    
    Synapse synapses[NET_MAX_SYNAPSES];
    int num_synapses;
    
    PendingSpike pending[NET_MAX_SYNAPSES];
    int num_pending;
    
    float input_currents[NET_MAX_NEURONS];
    int time_step;
    float dt;
} Network;

/* Create and manage */
void network_init(Network *net, float dt);
int network_add_neuron(Network *net, NeuronParams params);
int network_add_synapse(Network *net, int pre, int post, float weight, int delay);

/* Simulate one time step */
void network_step(Network *net, const float *external_input);

/* Reset */
void network_reset(Network *net);

#endif /* NETWORK_H */
