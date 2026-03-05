/*
 * hebbian.c — Hebbian Learning Implementation
 * Rethink AI — Phase 2
 */

#include "hebbian.h"
#include <string.h>
#include <math.h>

HebbianParams hebbian_default_params(void) {
    HebbianParams p;
    p.learning_rate = 0.01f;
    p.decay_rate = 0.001f;
    p.use_oja = 0;
    return p;
}

void hebbian_init(HebbianNet *h, int num_neurons, HebbianParams params) {
    h->params = params;
    h->num_neurons = num_neurons < HEBB_MAX_NEURONS ? num_neurons : HEBB_MAX_NEURONS;
    memset(h->weights, 0, sizeof(h->weights));
}

void hebbian_update(HebbianNet *h, const float *activations) {
    float lr = h->params.learning_rate;
    for (int i = 0; i < h->num_neurons; i++) {
        for (int j = 0; j < h->num_neurons; j++) {
            if (i == j) continue;
            float dw = lr * activations[i] * activations[j];
            if (h->params.use_oja) {
                dw -= lr * activations[j] * activations[j] * h->weights[i][j];
            }
            h->weights[i][j] += dw;
            if (h->weights[i][j] > 1.0f) h->weights[i][j] = 1.0f;
            if (h->weights[i][j] < -1.0f) h->weights[i][j] = -1.0f;
        }
    }
}

void hebbian_decay(HebbianNet *h) {
    float decay = h->params.decay_rate;
    for (int i = 0; i < h->num_neurons; i++) {
        for (int j = 0; j < h->num_neurons; j++) {
            h->weights[i][j] *= (1.0f - decay);
        }
    }
}

void hebbian_normalize(HebbianNet *h) {
    for (int i = 0; i < h->num_neurons; i++) {
        float sum_sq = 0.0f;
        for (int j = 0; j < h->num_neurons; j++) {
            sum_sq += h->weights[i][j] * h->weights[i][j];
        }
        float mag = sqrtf(sum_sq);
        if (mag > 1e-6f) {
            for (int j = 0; j < h->num_neurons; j++) {
                h->weights[i][j] /= mag;
            }
        }
    }
}
