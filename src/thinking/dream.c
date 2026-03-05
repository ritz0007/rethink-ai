/*
 * dream.c — Dream Engine Implementation
 * Rethink AI — Phase 6
 */

#include "dream.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

DreamEngine *dream_create(DecayMemory *memory) {
    DreamEngine *de = (DreamEngine *)calloc(1, sizeof(DreamEngine));
    de->memory = memory;
    de->stage = SLEEP_AWAKE;
    de->replay_fidelity = 0.9f;
    de->recombine_rate = 0.3f;
    return de;
}

void dream_destroy(DreamEngine *de) { free(de); }

void dream_nrem_replay(DreamEngine *de) {
    de->stage = SLEEP_NREM;
    
    /* Replay memories proportional to importance */
    for (int i = 0; i < de->memory->count; i++) {
        MemoryEntry *m = &de->memory->memories[i];
        if (m->importance > 0.3f) {
            /* Consolidate: reduce decay rate */
            m->decay_constant *= 0.95f;
            m->initial_strength = fminf(1.0f, m->initial_strength + 0.02f);
        }
    }
    
    /* Tick memory decay */
    decay_tick(de->memory, 0.5f);
}

void dream_rem_recombine(DreamEngine *de) {
    de->stage = SLEEP_REM;
    de->has_insight = 0;
    
    if (de->memory->count < 2) return;
    
    /* Pick two random memories and blend */
    int a = rand() % de->memory->count;
    int b = rand() % de->memory->count;
    if (a == b) b = (b + 1) % de->memory->count;
    
    MemoryEntry *ma = &de->memory->memories[a];
    MemoryEntry *mb = &de->memory->memories[b];
    int dim = ma->dim < mb->dim ? ma->dim : mb->dim;
    
    /* Create blended pattern */
    float blend[DREAM_MAX_DIM];
    float blend_ratio = 0.3f + ((float)rand() / RAND_MAX) * 0.4f;
    for (int i = 0; i < dim; i++) {
        blend[i] = ma->pattern[i] * blend_ratio + mb->pattern[i] * (1.0f - blend_ratio);
    }
    
    /* Check novelty (how different from both parents) */
    float sim_a = 0, sim_b = 0;
    for (int i = 0; i < dim; i++) {
        sim_a += blend[i] * ma->pattern[i];
        sim_b += blend[i] * mb->pattern[i];
    }
    float novelty = 1.0f - (fabsf(sim_a) + fabsf(sim_b)) / (2.0f * dim + 1e-6f);
    
    if (novelty > 0.3f) {
        memcpy(de->last_insight.pattern, blend, dim * sizeof(float));
        de->last_insight.dim = dim;
        de->last_insight.novelty = novelty;
        de->last_insight.stability = 0.5f;
        de->has_insight = 1;
    }
}

int dream_check_insight(DreamEngine *de, Insight *out) {
    if (de->has_insight) {
        *out = de->last_insight;
        de->has_insight = 0;
        return 1;
    }
    return 0;
}
