/*
 * hopfield.h — Hopfield Associative Memory
 * Rethink AI — Phase 4
 */

#ifndef HOPFIELD_H
#define HOPFIELD_H

#define HOPFIELD_MAX_NEURONS 256

typedef struct {
    int num_neurons;
    float weights[HOPFIELD_MAX_NEURONS][HOPFIELD_MAX_NEURONS];
    float state[HOPFIELD_MAX_NEURONS];
    int num_patterns;
} HopfieldNet;

HopfieldNet *hopfield_create(int num_neurons);
void hopfield_destroy(HopfieldNet *net);

void hopfield_store(HopfieldNet *net, const float *pattern, int dim);
void hopfield_recall(HopfieldNet *net, const float *partial, int dim, int iterations);
float hopfield_energy(const HopfieldNet *net);
float hopfield_similarity(const float *a, const float *b, int n);
void hopfield_add_noise(float *pattern, int dim, float noise_level);

#endif /* HOPFIELD_H */
