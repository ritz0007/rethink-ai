/*
 * neuron.c — LIF Neuron Implementation
 * Rethink AI — Phase 1
 */

#include "neuron.h"

NeuronParams neuron_default_params(void) {
    NeuronParams p;
    p.v_rest = -70.0f;
    p.v_threshold = -55.0f;
    p.v_reset = -75.0f;
    p.tau = 10.0f;
    p.r_membrane = 10.0f;
    p.refractory_ms = 2;
    return p;
}

void neuron_init(Neuron *n, NeuronParams params) {
    n->params = params;
    n->voltage = params.v_rest;
    n->refractory_timer = 0;
    n->spiked = 0;
    n->spike_count = 0;
    n->last_spike_time = -1000.0f;
}

void neuron_update(Neuron *n, float input_current, float dt) {
    n->spiked = 0;

    if (n->refractory_timer > 0) {
        n->refractory_timer--;
        return;
    }

    /* LIF equation: dV/dt = (-(V - V_rest) + R * I) / tau */
    float dv = (-(n->voltage - n->params.v_rest)
                + n->params.r_membrane * input_current) / n->params.tau;
    n->voltage += dv * dt;

    /* Check for spike */
    if (n->voltage >= n->params.v_threshold) {
        n->spiked = 1;
        n->spike_count++;
        n->voltage = n->params.v_reset;
        n->refractory_timer = n->params.refractory_ms;
    }
}

void neuron_reset(Neuron *n) {
    n->voltage = n->params.v_rest;
    n->refractory_timer = 0;
    n->spiked = 0;
}
