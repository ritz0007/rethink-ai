/*
 * decay.c — Memory Decay Implementation
 * Rethink AI — Phase 4
 */

#include "decay.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

DecayMemory *decay_create(void) {
    DecayMemory *dm = (DecayMemory *)calloc(1, sizeof(DecayMemory));
    return dm;
}

void decay_destroy(DecayMemory *dm) { free(dm); }

static float cosine_sim(const float *a, const float *b, int n) {
    float dot = 0, ma = 0, mb = 0;
    for (int i = 0; i < n; i++) {
        dot += a[i] * b[i]; ma += a[i]*a[i]; mb += b[i]*b[i];
    }
    if (ma < 1e-8f || mb < 1e-8f) return 0.0f;
    return dot / (sqrtf(ma) * sqrtf(mb));
}

int decay_store(DecayMemory *dm, const float *pattern, int dim, float importance) {
    int idx;
    if (dm->count < MAX_MEMORIES) {
        idx = dm->count++;
    } else {
        /* Replace weakest memory */
        float weakest = 1e30f;
        idx = 0;
        for (int i = 0; i < dm->count; i++) {
            float s = dm->memories[i].strength;
            if (s < weakest) { weakest = s; idx = i; }
        }
    }
    
    MemoryEntry *m = &dm->memories[idx];
    int d = dim < DECAY_MAX_DIM ? dim : DECAY_MAX_DIM;
    memcpy(m->pattern, pattern, d * sizeof(float));
    m->dim = d;
    m->strength = importance;
    m->initial_strength = importance;
    m->decay_constant = 1.0f / (importance + 0.1f);
    m->importance = importance;
    m->access_count = 0;
    m->store_time = dm->current_time;
    m->last_access_time = dm->current_time;
    return idx;
}

int decay_recall(DecayMemory *dm, const float *cue, int dim, float *out_pattern, float *out_strength) {
    float best_sim = -1.0f;
    int best_idx = -1;
    int d = dim < DECAY_MAX_DIM ? dim : DECAY_MAX_DIM;
    
    for (int i = 0; i < dm->count; i++) {
        int md = dm->memories[i].dim < d ? dm->memories[i].dim : d;
        float sim = cosine_sim(cue, dm->memories[i].pattern, md) * dm->memories[i].strength;
        if (sim > best_sim) { best_sim = sim; best_idx = i; }
    }
    
    if (best_idx >= 0) {
        MemoryEntry *m = &dm->memories[best_idx];
        if (out_pattern) memcpy(out_pattern, m->pattern, m->dim * sizeof(float));
        if (out_strength) *out_strength = m->strength;
        /* Boost from recall */
        m->strength = fminf(1.0f, m->strength + 0.1f);
        m->access_count++;
        m->last_access_time = dm->current_time;
    }
    return best_idx;
}

void decay_tick(DecayMemory *dm, float dt) {
    dm->current_time += dt;
    for (int i = 0; i < dm->count; i++) {
        MemoryEntry *m = &dm->memories[i];
        float elapsed = dm->current_time - m->store_time;
        /* Ebbinghaus forgetting curve: R = e^(-t/S) */
        m->strength = m->initial_strength * expf(-elapsed * m->decay_constant);
        /* Boost from access count */
        m->strength += 0.05f * m->access_count;
        if (m->strength > 1.0f) m->strength = 1.0f;
    }
}

void decay_consolidate(DecayMemory *dm) {
    for (int i = 0; i < dm->count; i++) {
        if (dm->memories[i].importance > 0.5f) {
            dm->memories[i].decay_constant *= 0.8f;
            dm->memories[i].initial_strength = fminf(1.0f, 
                dm->memories[i].initial_strength + 0.05f);
        }
    }
}
