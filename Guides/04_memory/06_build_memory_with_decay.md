# 🔨 Build: Memory with Decay — Memories That Fade

## Adding Time to Memory

The Hopfield network stores patterns forever. Real brains don't. Let's add:

1. **Decay** — memory strength fades over time
2. **Recall strengthening** — recalled memories get refreshed
3. **Priority** — important memories decay slower
4. **Capacity management** — when full, weakest memories go first

### Code: `src/memory/decay.h`

```c
/*
 * decay.h — Memory Decay and Consolidation
 * 
 * Rethink AI — Phase 4
 * 
 * Wraps around Hopfield memory to add:
 *   - Time-based decay (Ebbinghaus forgetting curve)
 *   - Recall-based strengthening (spaced repetition)
 *   - Importance-based decay rates
 *   - Automatic cleanup of faded memories
 */

#ifndef DECAY_H
#define DECAY_H

#include "hopfield.h"

#define MAX_MEMORIES 64

typedef struct {
    float pattern[HOPFIELD_MAX_NEURONS];  /* The stored pattern */
    float strength;       /* Current strength (1.0 = fresh, 0.0 = forgotten) */
    float initial_strength; /* Strength when stored */
    float decay_constant; /* τ in exp(-t/τ) — higher = slower decay */
    float importance;     /* 0.0 to 1.0 — affects decay speed */
    float store_time;     /* When this memory was stored */
    float last_recall_time; /* When last recalled */
    int recall_count;     /* Total recall count */
    int active;           /* 1 if this slot is in use, 0 if cleared */
} MemoryEntry;

typedef struct {
    Hopfield *hopfield;   /* The underlying associative memory */
    MemoryEntry memories[MAX_MEMORIES];
    int num_memories;
    int neuron_count;
    float current_time;   /* Simulation time */
    
    /* Parameters */
    float forget_threshold;   /* Below this strength → memory is "forgotten" */
    float base_decay_constant; /* Default decay constant */
    float recall_boost;       /* How much recall strengthens memory */
    float importance_multiplier; /* How much importance slows decay */
} DecayMemory;

/* Create a decay memory system */
DecayMemory *decay_memory_create(int neuron_count);
void decay_memory_destroy(DecayMemory *dm);

/* Store a new memory with given importance (0 to 1) */
int decay_memory_store(DecayMemory *dm, const float *pattern, float importance);

/* Recall from a probe — returns similarity to best match */
float decay_memory_recall(DecayMemory *dm, const float *probe, 
                          float *recalled_pattern, int max_iters);

/* Advance time — call this regularly to decay memories */
void decay_memory_tick(DecayMemory *dm, float dt);

/* "Dream" — replay and strengthen the top-N most important memories */
void decay_memory_consolidate(DecayMemory *dm, int top_n);

/* How many memories are still "alive" (above forget threshold)? */
int decay_memory_alive_count(const DecayMemory *dm);

/* Print memory status */
void decay_memory_print_status(const DecayMemory *dm);

#endif /* DECAY_H */
```

### Code: `src/memory/decay.c`

```c
#include "decay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

DecayMemory *decay_memory_create(int neuron_count) {
    DecayMemory *dm = (DecayMemory *)calloc(1, sizeof(DecayMemory));
    dm->hopfield = hopfield_create(neuron_count);
    dm->neuron_count = neuron_count;
    dm->num_memories = 0;
    dm->current_time = 0.0f;
    
    dm->forget_threshold = 0.1f;
    dm->base_decay_constant = 100.0f;  /* Time units until ~37% strength */
    dm->recall_boost = 0.3f;
    dm->importance_multiplier = 3.0f;
    
    return dm;
}

void decay_memory_destroy(DecayMemory *dm) {
    hopfield_destroy(dm->hopfield);
    free(dm);
}

int decay_memory_store(DecayMemory *dm, const float *pattern, float importance) {
    if (dm->num_memories >= MAX_MEMORIES) {
        /* Find weakest memory and overwrite it */
        int weakest = 0;
        float min_strength = dm->memories[0].strength;
        for (int i = 1; i < dm->num_memories; i++) {
            if (dm->memories[i].active && dm->memories[i].strength < min_strength) {
                min_strength = dm->memories[i].strength;
                weakest = i;
            }
        }
        /* Overwrite weakest */
        dm->memories[weakest].active = 0;
        /* Note: we can't easily remove from Hopfield, so quality degrades */
    }
    
    /* Find empty slot */
    int slot = -1;
    for (int i = 0; i < MAX_MEMORIES; i++) {
        if (!dm->memories[i].active) { slot = i; break; }
    }
    if (slot == -1) slot = dm->num_memories;
    
    /* Store in Hopfield */
    hopfield_store(dm->hopfield, pattern);
    
    /* Store metadata */
    MemoryEntry *m = &dm->memories[slot];
    memcpy(m->pattern, pattern, dm->neuron_count * sizeof(float));
    m->strength = 1.0f;
    m->initial_strength = 1.0f;
    m->decay_constant = dm->base_decay_constant * (1.0f + importance * dm->importance_multiplier);
    m->importance = importance;
    m->store_time = dm->current_time;
    m->last_recall_time = dm->current_time;
    m->recall_count = 0;
    m->active = 1;
    
    if (slot >= dm->num_memories) dm->num_memories = slot + 1;
    
    return slot;
}

float decay_memory_recall(DecayMemory *dm, const float *probe,
                          float *recalled_pattern, int max_iters) {
    /* Use Hopfield to recall */
    hopfield_recall(dm->hopfield, probe, max_iters);
    
    /* Copy result */
    if (recalled_pattern) {
        memcpy(recalled_pattern, dm->hopfield->state, 
               dm->neuron_count * sizeof(float));
    }
    
    /* Find which stored memory this matches best */
    float best_sim = 0.0f;
    int best_idx = -1;
    for (int i = 0; i < dm->num_memories; i++) {
        if (!dm->memories[i].active) continue;
        float sim = hopfield_similarity(dm->hopfield, dm->memories[i].pattern);
        if (sim > best_sim) {
            best_sim = sim;
            best_idx = i;
        }
    }
    
    /* Strengthen the recalled memory (recall boost!) */
    if (best_idx >= 0 && best_sim > 0.8f) {
        MemoryEntry *m = &dm->memories[best_idx];
        m->strength = fminf(1.0f, m->strength + dm->recall_boost);
        m->decay_constant *= 1.2f;  /* Slower decay after each recall */
        m->recall_count++;
        m->last_recall_time = dm->current_time;
    }
    
    return best_sim;
}

void decay_memory_tick(DecayMemory *dm, float dt) {
    dm->current_time += dt;
    
    for (int i = 0; i < dm->num_memories; i++) {
        if (!dm->memories[i].active) continue;
        
        MemoryEntry *m = &dm->memories[i];
        float elapsed = dm->current_time - m->last_recall_time;
        
        /* Ebbinghaus forgetting curve: strength = exp(-t / τ) */
        m->strength = m->initial_strength * expf(-elapsed / m->decay_constant);
        
        /* If below threshold → effectively forgotten */
        if (m->strength < dm->forget_threshold) {
            m->active = 0;
        }
    }
}

void decay_memory_consolidate(DecayMemory *dm, int top_n) {
    /* "Dream mode": replay the most important memories to strengthen them */
    /* Sort by importance × strength (descending) */
    
    /* Simple approach: find top_n and boost them */
    for (int round = 0; round < top_n; round++) {
        float best_score = -1.0f;
        int best_idx = -1;
        
        for (int i = 0; i < dm->num_memories; i++) {
            if (!dm->memories[i].active) continue;
            float score = dm->memories[i].importance * dm->memories[i].strength;
            /* Avoid re-selecting the same one (imperfect but simple) */
            if (score > best_score) {
                best_score = score;
                best_idx = i;
            }
        }
        
        if (best_idx >= 0) {
            MemoryEntry *m = &dm->memories[best_idx];
            /* "Replay" = recall the memory, which strengthens it */
            decay_memory_recall(dm, m->pattern, NULL, 50);
        }
    }
}

int decay_memory_alive_count(const DecayMemory *dm) {
    int count = 0;
    for (int i = 0; i < dm->num_memories; i++) {
        if (dm->memories[i].active) count++;
    }
    return count;
}

void decay_memory_print_status(const DecayMemory *dm) {
    printf("\n=== Memory Status (t=%.1f) ===\n", dm->current_time);
    printf("Alive: %d / %d stored\n", 
           decay_memory_alive_count(dm), dm->num_memories);
    
    for (int i = 0; i < dm->num_memories; i++) {
        if (!dm->memories[i].active) continue;
        const MemoryEntry *m = &dm->memories[i];
        
        int bar_len = (int)(m->strength * 20);
        printf("  [%2d] str=%.3f imp=%.1f recalls=%d τ=%.0f |",
               i, m->strength, m->importance, m->recall_count, m->decay_constant);
        for (int b = 0; b < bar_len; b++) printf("█");
        for (int b = bar_len; b < 20; b++) printf("░");
        printf("|\n");
    }
    printf("===========================\n");
}
```

---

## Milestone Test

```c
/* 
 * Test: Store memories, let time pass, watch them fade.
 * Important memories survive. Unimportant ones fade.
 * Recalled memories get stronger.
 */

// Store 5 memories with varying importance
decay_memory_store(dm, pattern_name, 1.0f);      // Very important (your name)
decay_memory_store(dm, pattern_breakfast, 0.3f);  // Meh (what you ate)
decay_memory_store(dm, pattern_face, 0.1f);       // Unimportant (random face)
decay_memory_store(dm, pattern_skill, 0.8f);      // Important (a skill)
decay_memory_store(dm, pattern_fact, 0.5f);       // Medium (a fact)

// Let time pass
for (int t = 0; t < 500; t++) {
    decay_memory_tick(dm, 1.0f);
    
    // Periodically recall your name (reinforcement)
    if (t % 50 == 0) {
        decay_memory_recall(dm, pattern_name, NULL, 50);
    }
}

decay_memory_print_status(dm);
// Expected: name = strong, breakfast/face = faded/gone, skill = moderate
```

**Phase 4 Complete → Tag as V4**

---

*Next: [Phase 5 — Learning Like a Child](../05_learning_like_a_child/01_brain_few_shot_learning.md)*
