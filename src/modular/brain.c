/*
 * brain.c — Modular Brain Bus Implementation
 * Rethink AI — Phase 7
 */

#include "brain.h"
#include <string.h>

void brain_bus_init(BrainBus *bb) {
    memset(bb, 0, sizeof(BrainBus));
    for (int r = 0; r < REGION_COUNT; r++) {
        bb->thalamus.gain[r] = 1.0f;
        bb->thalamus.priority[r] = 0.5f;
    }
    bb->thalamus.surprise_threshold = 0.3f;
}

int brain_bus_send(BrainBus *bb, Signal sig) {
    if (bb->num_signals >= BRAIN_MAX_SIGNALS) return -1;
    
    /* Apply thalamic gating */
    float gate = bb->thalamus.gain[sig.target];
    sig.priority *= gate;
    
    if (sig.surprise > bb->thalamus.surprise_threshold) {
        sig.priority *= 1.5f; /* Surprise boosts priority */
    }
    
    bb->bus[bb->num_signals++] = sig;
    return 0;
}

int brain_bus_receive(BrainBus *bb, RegionType target, Signal *out) {
    float best_priority = -1.0f;
    int best_idx = -1;
    
    for (int i = 0; i < bb->num_signals; i++) {
        if (bb->bus[i].target == target && bb->bus[i].priority > best_priority) {
            best_priority = bb->bus[i].priority;
            best_idx = i;
        }
    }
    
    if (best_idx >= 0) {
        *out = bb->bus[best_idx];
        /* Remove from bus */
        for (int i = best_idx; i < bb->num_signals - 1; i++)
            bb->bus[i] = bb->bus[i + 1];
        bb->num_signals--;
        return 1;
    }
    return 0;
}

void brain_bus_clear(BrainBus *bb) { bb->num_signals = 0; }

void thalamus_update_gain(Thalamus *t, RegionType region, float surprise) {
    if (surprise > t->surprise_threshold) {
        t->gain[region] *= 1.2f;
        if (t->gain[region] > 3.0f) t->gain[region] = 3.0f;
    } else {
        t->gain[region] *= 0.95f;
        if (t->gain[region] < 0.1f) t->gain[region] = 0.1f;
    }
}
