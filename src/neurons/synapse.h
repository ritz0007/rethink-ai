/*
 * synapse.h — Weighted + Delayed Synaptic Connection
 * Rethink AI — Phase 1
 */

#ifndef SYNAPSE_H
#define SYNAPSE_H

typedef struct {
    int pre;        /* Pre-synaptic neuron index */
    int post;       /* Post-synaptic neuron index */
    float weight;   /* Synaptic weight */
    int delay;      /* Transmission delay (time steps) */
} Synapse;

#endif /* SYNAPSE_H */
