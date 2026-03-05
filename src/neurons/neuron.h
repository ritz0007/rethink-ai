/*
 * neuron.h — Leaky Integrate-and-Fire Spiking Neuron
 * Rethink AI — Phase 1
 */

#ifndef NEURON_H
#define NEURON_H

typedef struct {
    float v_rest;       /* Resting potential (mV), typically -70 */
    float v_threshold;  /* Spike threshold (mV), typically -55 */
    float v_reset;      /* Post-spike reset (mV), typically -75 */
    float tau;          /* Membrane time constant (ms), typically 10 */
    float r_membrane;   /* Membrane resistance (MOhm), typically 10 */
    int refractory_ms;  /* Refractory period (ms), typically 2 */
} NeuronParams;

typedef struct {
    NeuronParams params;
    float voltage;
    int refractory_timer;
    int spiked;
    int spike_count;
    float last_spike_time;
} Neuron;

/* Default biological parameters */
NeuronParams neuron_default_params(void);

/* Initialize neuron */
void neuron_init(Neuron *n, NeuronParams params);

/* Update neuron by one time step (dt in ms) */
void neuron_update(Neuron *n, float input_current, float dt);

/* Reset neuron to resting state */
void neuron_reset(Neuron *n);

#endif /* NEURON_H */
