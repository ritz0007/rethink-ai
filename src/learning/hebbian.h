/*
 * hebbian.h — Hebbian Learning Rule
 * Rethink AI — Phase 2
 */

#ifndef HEBBIAN_H
#define HEBBIAN_H

#define HEBB_MAX_NEURONS 256

typedef struct {
    float learning_rate;
    float decay_rate;
    int use_oja;         /* 1 = Oja's normalized rule */
} HebbianParams;

typedef struct {
    HebbianParams params;
    float weights[HEBB_MAX_NEURONS][HEBB_MAX_NEURONS];
    int num_neurons;
} HebbianNet;

HebbianParams hebbian_default_params(void);
void hebbian_init(HebbianNet *h, int num_neurons, HebbianParams params);
void hebbian_update(HebbianNet *h, const float *activations);
void hebbian_decay(HebbianNet *h);
void hebbian_normalize(HebbianNet *h);

#endif /* HEBBIAN_H */
