/*
 * network.c — Spiking Network Implementation
 * Rethink AI — Phase 1
 */

#include "network.h"
#include <string.h>

void network_init(Network *net, float dt) {
    memset(net, 0, sizeof(Network));
    net->dt = dt;
}

int network_add_neuron(Network *net, NeuronParams params) {
    if (net->num_neurons >= NET_MAX_NEURONS) return -1;
    int idx = net->num_neurons++;
    neuron_init(&net->neurons[idx], params);
    return idx;
}

int network_add_synapse(Network *net, int pre, int post, float weight, int delay) {
    if (net->num_synapses >= NET_MAX_SYNAPSES) return -1;
    if (delay < 1) delay = 1;
    int idx = net->num_synapses++;
    net->synapses[idx].pre = pre;
    net->synapses[idx].post = post;
    net->synapses[idx].weight = weight;
    net->synapses[idx].delay = delay;
    return idx;
}

void network_step(Network *net, const float *external_input) {
    /* Clear input currents */
    memset(net->input_currents, 0, net->num_neurons * sizeof(float));
    
    /* Add external input */
    if (external_input) {
        for (int i = 0; i < net->num_neurons; i++) {
            net->input_currents[i] += external_input[i];
        }
    }
    
    /* Deliver pending spikes */
    int new_pending = 0;
    for (int i = 0; i < net->num_pending; i++) {
        if (net->pending[i].deliver_at <= net->time_step) {
            net->input_currents[net->pending[i].target] += net->pending[i].weight;
        } else {
            net->pending[new_pending++] = net->pending[i];
        }
    }
    net->num_pending = new_pending;
    
    /* Update all neurons */
    for (int i = 0; i < net->num_neurons; i++) {
        neuron_update(&net->neurons[i], net->input_currents[i], net->dt);
    }
    
    /* Queue spikes from neurons that fired */
    for (int i = 0; i < net->num_neurons; i++) {
        if (!net->neurons[i].spiked) continue;
        net->neurons[i].last_spike_time = net->time_step * net->dt;
        
        for (int s = 0; s < net->num_synapses; s++) {
            if (net->synapses[s].pre != i) continue;
            if (net->num_pending >= NET_MAX_SYNAPSES) break;
            
            PendingSpike ps;
            ps.target = net->synapses[s].post;
            ps.weight = net->synapses[s].weight;
            ps.deliver_at = net->time_step + net->synapses[s].delay;
            net->pending[net->num_pending++] = ps;
        }
    }
    
    net->time_step++;
}

void network_reset(Network *net) {
    for (int i = 0; i < net->num_neurons; i++) {
        neuron_reset(&net->neurons[i]);
    }
    net->num_pending = 0;
    net->time_step = 0;
}
