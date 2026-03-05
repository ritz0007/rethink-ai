/*
 * spontaneous.c — Spontaneous Activity Implementation
 * Rethink AI — Phase 6
 */

#include "spontaneous.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

SpontaneousNet *spontaneous_create(int num_neurons) {
    SpontaneousNet *sn = (SpontaneousNet *)calloc(1, sizeof(SpontaneousNet));
    sn->num_neurons = num_neurons < SPONT_MAX_NEURONS ? num_neurons : SPONT_MAX_NEURONS;
    sn->noise_level = 0.1f;
    sn->decay = 0.95f;
    sn->threshold = 0.5f;
    sn->inhibition = 0.1f;
    
    /* Random initial weights */
    for (int i = 0; i < sn->num_neurons; i++)
        for (int j = 0; j < sn->num_neurons; j++)
            if (i != j) sn->weights[i][j] = ((float)rand() / RAND_MAX - 0.5f) * 0.2f;
    
    return sn;
}

void spontaneous_destroy(SpontaneousNet *sn) { free(sn); }

void spontaneous_set_weights(SpontaneousNet *sn, int i, int j, float w) {
    if (i >= 0 && i < sn->num_neurons && j >= 0 && j < sn->num_neurons)
        sn->weights[i][j] = w;
}

void spontaneous_tick(SpontaneousNet *sn) {
    float new_act[SPONT_MAX_NEURONS];
    
    for (int i = 0; i < sn->num_neurons; i++) {
        float input = 0;
        for (int j = 0; j < sn->num_neurons; j++)
            input += sn->weights[i][j] * sn->activation[j];
        
        /* Add noise */
        float noise = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * sn->noise_level;
        input += noise;
        
        /* Apply with decay */
        new_act[i] = sn->activation[i] * sn->decay + input * (1.0f - sn->decay);
        
        /* Threshold (soft) */
        new_act[i] = 1.0f / (1.0f + expf(-5.0f * (new_act[i] - sn->threshold)));
        
        /* Global inhibition */
        float total = 0;
        for (int j = 0; j < sn->num_neurons; j++) total += sn->activation[j];
        new_act[i] -= sn->inhibition * total / sn->num_neurons;
        if (new_act[i] < 0) new_act[i] = 0;
    }
    
    memcpy(sn->activation, new_act, sn->num_neurons * sizeof(float));
}

int spontaneous_capture_thought(SpontaneousNet *sn, Thought *out) {
    float total = 0;
    for (int i = 0; i < sn->num_neurons; i++) total += sn->activation[i];
    
    if (total > sn->threshold * sn->num_neurons * 0.1f) {
        memcpy(out->pattern, sn->activation, sn->num_neurons * sizeof(float));
        out->dim = sn->num_neurons;
        out->strength = total / sn->num_neurons;
        return 1;
    }
    return 0;
}

void spontaneous_set_mode(SpontaneousNet *sn, int mode) {
    switch (mode) {
        case 0: /* focused */
            sn->noise_level = 0.05f;
            sn->inhibition = 0.2f;
            break;
        case 1: /* creative */
            sn->noise_level = 0.2f;
            sn->inhibition = 0.05f;
            break;
        case 2: /* dreaming */
            sn->noise_level = 0.3f;
            sn->inhibition = 0.02f;
            break;
    }
}
