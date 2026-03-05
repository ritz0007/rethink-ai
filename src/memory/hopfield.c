/*
 * hopfield.c — Hopfield Memory Implementation
 * Rethink AI — Phase 4
 */

#include "hopfield.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

HopfieldNet *hopfield_create(int num_neurons) {
    HopfieldNet *net = (HopfieldNet *)calloc(1, sizeof(HopfieldNet));
    net->num_neurons = num_neurons < HOPFIELD_MAX_NEURONS ? num_neurons : HOPFIELD_MAX_NEURONS;
    return net;
}

void hopfield_destroy(HopfieldNet *net) { free(net); }

void hopfield_store(HopfieldNet *net, const float *pattern, int dim) {
    int n = dim < net->num_neurons ? dim : net->num_neurons;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                net->weights[i][j] += pattern[i] * pattern[j];
            }
        }
    }
    net->num_patterns++;
}

void hopfield_recall(HopfieldNet *net, const float *partial, int dim, int iterations) {
    int n = dim < net->num_neurons ? dim : net->num_neurons;
    memcpy(net->state, partial, n * sizeof(float));
    
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < n; i++) {
            float h = 0;
            for (int j = 0; j < n; j++) {
                h += net->weights[i][j] * net->state[j];
            }
            net->state[i] = h >= 0 ? 1.0f : -1.0f;
        }
    }
}

float hopfield_energy(const HopfieldNet *net) {
    float E = 0;
    for (int i = 0; i < net->num_neurons; i++)
        for (int j = i + 1; j < net->num_neurons; j++)
            E -= net->weights[i][j] * net->state[i] * net->state[j];
    return E;
}

float hopfield_similarity(const float *a, const float *b, int n) {
    float match = 0;
    for (int i = 0; i < n; i++) {
        if ((a[i] > 0) == (b[i] > 0)) match += 1.0f;
    }
    return match / n;
}

void hopfield_add_noise(float *pattern, int dim, float noise_level) {
    for (int i = 0; i < dim; i++) {
        if ((float)rand() / RAND_MAX < noise_level) {
            pattern[i] = -pattern[i];
        }
    }
}
