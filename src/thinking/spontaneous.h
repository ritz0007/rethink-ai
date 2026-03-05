/*
 * spontaneous.h — Spontaneous Neural Activity
 * Rethink AI — Phase 6
 */

#ifndef SPONTANEOUS_H
#define SPONTANEOUS_H

#define SPONT_MAX_NEURONS 256

typedef struct {
    float pattern[SPONT_MAX_NEURONS];
    int dim;
    float strength;
} Thought;

typedef struct {
    int num_neurons;
    float activation[SPONT_MAX_NEURONS];
    float weights[SPONT_MAX_NEURONS][SPONT_MAX_NEURONS];
    float noise_level;
    float decay;
    float threshold;
    float inhibition;
} SpontaneousNet;

SpontaneousNet *spontaneous_create(int num_neurons);
void spontaneous_destroy(SpontaneousNet *sn);

void spontaneous_set_weights(SpontaneousNet *sn, int i, int j, float w);
void spontaneous_tick(SpontaneousNet *sn);
int spontaneous_capture_thought(SpontaneousNet *sn, Thought *out);
void spontaneous_set_mode(SpontaneousNet *sn, int mode); /* 0=focused, 1=creative, 2=dreaming */

#endif /* SPONTANEOUS_H */
