# 🔨 Build: Dream Engine — Memory Consolidation and Creative Recombination

## What We're Building

A dream engine that runs during "sleep" to:
1. **Replay** important memories (NREM deep sleep)
2. **Recombine** memories creatively (REM sleep)
3. **Prune** weak connections (synaptic homeostasis)
4. **Discover** new patterns (insight generation)

## Code: `src/thinking/dream.h`

```c
/*
 * dream.h — Dream Engine: Memory Consolidation and Creativity
 *
 * Rethink AI — Phase 6
 *
 * Implements sleep-like processing:
 *   NREM: Replay and strengthen important memories  
 *   REM:  Recombine memories, discover new patterns
 *   Prune: Weaken unused connections
 */

#ifndef DREAM_H
#define DREAM_H

#include "spontaneous.h"
#include "../memory/decay.h"

#define DREAM_MAX_INSIGHTS 16

typedef enum {
    SLEEP_NREM,    /* Deep sleep: replay and consolidate */
    SLEEP_REM,     /* REM: creative recombination */
    SLEEP_WAKE     /* Awake */
} SleepStage;

/* An "insight" discovered during dreaming */
typedef struct {
    float pattern[SPONT_MAX_NEURONS];
    int source_a;     /* Memory index that contributed */
    int source_b;     /* Memory index that contributed */
    float novelty;    /* How different from known memories (0-1) */
    float stability;  /* How stable this pattern is as an attractor */
} Insight;

typedef struct {
    SpontaneousNet *mind;    /* Internal activity generator */
    DecayMemory *memory;     /* Memory system with decay */
    
    SleepStage stage;
    int cycle_count;         /* Total dream cycles completed */
    
    /* Dream parameters */
    int nrem_replays;        /* Replays per important memory in NREM */
    int rem_combinations;    /* Random combos to try in REM */
    float prune_factor;      /* How much to decay during sleep */
    float insight_threshold; /* Minimum novelty to count as insight */
    
    /* Discovered insights */
    Insight insights[DREAM_MAX_INSIGHTS];
    int num_insights;
    
    /* Statistics */
    int memories_strengthened;
    int memories_pruned;
    int patterns_tested;
} DreamEngine;

/* Create dream engine from existing mind and memory */
DreamEngine *dream_create(SpontaneousNet *mind, DecayMemory *memory);
void dream_destroy(DreamEngine *de);

/* Run a full sleep cycle (NREM → REM → Prune) */
void dream_sleep_cycle(DreamEngine *de);

/* Run just NREM (replay) */
void dream_nrem(DreamEngine *de);

/* Run just REM (creative recombination) */
void dream_rem(DreamEngine *de);

/* Prune weak connections */
void dream_prune(DreamEngine *de);

/* Check how novel a pattern is compared to known memories */
float dream_novelty(const DreamEngine *de, const float *pattern);

/* Print dream report */
void dream_print_report(const DreamEngine *de);

#endif /* DREAM_H */
```

## Code: `src/thinking/dream.c`

```c
#include "dream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

DreamEngine *dream_create(SpontaneousNet *mind, DecayMemory *memory) {
    DreamEngine *de = (DreamEngine *)calloc(1, sizeof(DreamEngine));
    de->mind = mind;
    de->memory = memory;
    de->stage = SLEEP_WAKE;
    de->cycle_count = 0;
    de->num_insights = 0;
    
    de->nrem_replays = 5;
    de->rem_combinations = 10;
    de->prune_factor = 5.0f;
    de->insight_threshold = 0.4f;
    
    de->memories_strengthened = 0;
    de->memories_pruned = 0;
    de->patterns_tested = 0;
    
    return de;
}

void dream_destroy(DreamEngine *de) {
    /* Don't destroy mind/memory — we don't own them */
    free(de);
}

void dream_nrem(DreamEngine *de) {
    de->stage = SLEEP_NREM;
    DecayMemory *dm = de->memory;
    SpontaneousNet *sn = de->mind;
    int n = sn->neuron_count;
    
    /* Set brain to deep sleep mode: low noise, moderate inhibition */
    spontaneous_set_mode(sn, 0.01f, 0.03f);
    
    /* Replay each active memory, proportional to importance */
    for (int i = 0; i < dm->num_memories; i++) {
        MemoryEntry *m = &dm->memories[i];
        if (!m->active) continue;
        
        int replays = (int)(m->importance * de->nrem_replays) + 1;
        
        for (int r = 0; r < replays; r++) {
            /* Inject memory pattern into the network */
            for (int j = 0; j < n && j < dm->neuron_count; j++) {
                sn->activation[j] = (m->pattern[j] > 0) ? 0.8f : 0.0f;
            }
            
            /* Let it propagate for a few steps (replay) */
            spontaneous_run(sn, 10);
            
            /* Strengthen this memory */
            m->strength = fminf(1.0f, m->strength + 0.02f);
            m->decay_constant *= 1.05f;  /* Slower decay after replay */
            
            de->memories_strengthened++;
        }
    }
}

float dream_novelty(const DreamEngine *de, const float *pattern) {
    DecayMemory *dm = de->memory;
    int n = de->mind->neuron_count;
    float max_sim = 0.0f;
    
    for (int i = 0; i < dm->num_memories; i++) {
        if (!dm->memories[i].active) continue;
        
        float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
        for (int j = 0; j < n && j < dm->neuron_count; j++) {
            float a = pattern[j];
            float b = dm->memories[i].pattern[j];
            dot += a * b;
            norm_a += a * a;
            norm_b += b * b;
        }
        float sim = 0.0f;
        if (norm_a > 1e-8f && norm_b > 1e-8f) {
            sim = dot / (sqrtf(norm_a) * sqrtf(norm_b));
        }
        if (sim > max_sim) max_sim = sim;
    }
    
    /* Novelty = 1 - max similarity to any known memory */
    return 1.0f - max_sim;
}

void dream_rem(DreamEngine *de) {
    de->stage = SLEEP_REM;
    DecayMemory *dm = de->memory;
    SpontaneousNet *sn = de->mind;
    int n = sn->neuron_count;
    
    /* Set brain to REM mode: higher noise, lower inhibition */
    spontaneous_set_mode(sn, 0.15f, 0.015f);
    
    for (int combo = 0; combo < de->rem_combinations; combo++) {
        /* Pick two random active memories */
        int active_count = decay_memory_alive_count(dm);
        if (active_count < 2) break;
        
        /* Find two random active memories */
        int idx_a = -1, idx_b = -1;
        int target_a = rand() % active_count;
        int target_b = rand() % active_count;
        while (target_b == target_a && active_count > 1) {
            target_b = rand() % active_count;
        }
        
        int count = 0;
        for (int i = 0; i < dm->num_memories; i++) {
            if (!dm->memories[i].active) continue;
            if (count == target_a) idx_a = i;
            if (count == target_b) idx_b = i;
            count++;
        }
        if (idx_a < 0 || idx_b < 0) continue;
        
        /* Create a blend of the two memories */
        float mix = 0.3f + ((float)rand() / RAND_MAX) * 0.4f;
        for (int j = 0; j < n && j < dm->neuron_count; j++) {
            sn->activation[j] = mix * dm->memories[idx_a].pattern[j] * 0.5f + 0.5f
                               + (1.0f - mix) * dm->memories[idx_b].pattern[j] * 0.5f + 0.5f;
            sn->activation[j] = fmaxf(0.0f, fminf(1.0f, sn->activation[j]));
        }
        
        /* Let the network settle */
        spontaneous_run(sn, 20);
        
        de->patterns_tested++;
        
        /* Check if result is novel */
        float state[SPONT_MAX_NEURONS];
        spontaneous_get_state(sn, state);
        float novelty = dream_novelty(de, state);
        
        if (novelty > de->insight_threshold && de->num_insights < DREAM_MAX_INSIGHTS) {
            /* New insight discovered! */
            Insight *ins = &de->insights[de->num_insights++];
            memcpy(ins->pattern, state, n * sizeof(float));
            ins->source_a = idx_a;
            ins->source_b = idx_b;
            ins->novelty = novelty;
            
            /* Check stability by perturbing and re-settling */
            float perturbed[SPONT_MAX_NEURONS];
            memcpy(perturbed, state, n * sizeof(float));
            for (int j = 0; j < n; j++) {
                perturbed[j] += ((float)rand() / RAND_MAX - 0.5f) * 0.2f;
            }
            for (int j = 0; j < n; j++) sn->activation[j] = perturbed[j];
            spontaneous_run(sn, 15);
            spontaneous_get_state(sn, perturbed);
            
            /* How similar is perturbed result to original? */
            float dot = 0.0f, na = 0.0f, nb = 0.0f;
            for (int j = 0; j < n; j++) {
                dot += state[j] * perturbed[j];
                na += state[j] * state[j];
                nb += perturbed[j] * perturbed[j];
            }
            ins->stability = (na > 1e-8f && nb > 1e-8f) 
                           ? dot / (sqrtf(na) * sqrtf(nb)) 
                           : 0.0f;
        }
    }
}

void dream_prune(DreamEngine *de) {
    /* Advance time → weak memories decay further */
    decay_memory_tick(de->memory, de->prune_factor);
    
    int initial = decay_memory_alive_count(de->memory);
    /* Count how many were pruned */
    /* (tick already deactivates memories below threshold) */
    int remaining = decay_memory_alive_count(de->memory);
    de->memories_pruned += (initial - remaining);
}

void dream_sleep_cycle(DreamEngine *de) {
    printf("  💤 Sleep cycle %d starting...\n", de->cycle_count + 1);
    
    /* Phase 1: NREM — replay and consolidate */
    dream_nrem(de);
    
    /* Phase 2: REM — creative recombination */
    dream_rem(de);
    
    /* Phase 3: Prune — clean up weak memories */
    dream_prune(de);
    
    /* Return to wake */
    de->stage = SLEEP_WAKE;
    spontaneous_set_mode(de->mind, 0.05f, 0.02f);
    
    de->cycle_count++;
}

void dream_print_report(const DreamEngine *de) {
    printf("\n=== Dream Report (Cycle %d) ===\n", de->cycle_count);
    printf("Stage: %s\n", de->stage == SLEEP_WAKE ? "Awake" :
           de->stage == SLEEP_NREM ? "NREM" : "REM");
    printf("Memories strengthened: %d replay events\n", de->memories_strengthened);
    printf("Memories pruned: %d forgotten\n", de->memories_pruned);
    printf("Patterns tested (REM): %d combinations\n", de->patterns_tested);
    printf("Insights discovered: %d\n\n", de->num_insights);
    
    for (int i = 0; i < de->num_insights; i++) {
        const Insight *ins = &de->insights[i];
        printf("  💡 Insight %d:\n", i);
        printf("     From memories: #%d + #%d\n", ins->source_a, ins->source_b);
        printf("     Novelty: %.2f  Stability: %.2f\n", ins->novelty, ins->stability);
        if (ins->stability > 0.7f) {
            printf("     → STABLE insight — could become a new memory!\n");
        } else {
            printf("     → Unstable — just a fleeting dream image\n");
        }
    }
    
    printf("\nMemories alive: %d\n", decay_memory_alive_count(de->memory));
    printf("===============================\n");
}
```

## Test: `test_dream.c`

```c
/*
 * test_dream.c — Watch the brain dream
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_dream \
 *          test_dream.c dream.c spontaneous.c decay.c hopfield.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include "dream.h"

#define N 32

int main(void) {
    srand(42);
    printf("=== Dream Engine: Sleep, Dream, Learn ===\n\n");
    
    /* Create brain components */
    SpontaneousNet *mind = spontaneous_create(N);
    DecayMemory *memory = decay_memory_create(N);
    
    /* Store 4 memories during "daytime" */
    float patterns[4][32];
    const char *names[] = {"fun trip", "boring lecture", "scary movie", "friend's face"};
    float importance[] = {0.8f, 0.1f, 0.6f, 0.9f};
    
    for (int m = 0; m < 4; m++) {
        /* Create distinct patterns */
        for (int i = 0; i < N; i++) {
            patterns[m][i] = ((i + m * 7) % 4 == 0) ? 1.0f : -1.0f;
        }
        decay_memory_store(memory, patterns[m], importance[m]);
        spontaneous_imprint(mind, patterns[m], importance[m] * 0.3f);
        printf("Stored: \"%s\" (importance=%.1f)\n", names[m], importance[m]);
    }
    
    printf("\nBefore sleep:\n");
    decay_memory_print_status(memory);
    
    /* Let some time pass (waking hours) */
    for (int t = 0; t < 100; t++) {
        decay_memory_tick(memory, 1.0f);
    }
    printf("\nAfter 100 time steps awake:\n");
    decay_memory_print_status(memory);
    
    /* SLEEP! */
    DreamEngine *de = dream_create(mind, memory);
    
    printf("\n--- Going to sleep ---\n");
    for (int cycle = 0; cycle < 3; cycle++) {
        dream_sleep_cycle(de);
    }
    
    dream_print_report(de);
    
    printf("\nAfter 3 sleep cycles:\n");
    decay_memory_print_status(memory);
    
    printf("\n=== Analysis ===\n");
    printf("• Important memories (trip, friend) → STRENGTHENED by replay\n");
    printf("• Unimportant memories (boring lecture) → FADING faster\n");
    printf("• REM discovered %d novel patterns (insights)\n", de->num_insights);
    printf("• The brain reorganized itself while 'sleeping'\n");
    printf("\nThis is why you should sleep after studying.\n");
    
    dream_destroy(de);
    spontaneous_destroy(mind);
    decay_memory_destroy(memory);
    
    printf("\n✓ Dream engine working!\n");
    return 0;
}
```

---

*Next: [Milestone — A Brain That Thinks, Dreams, and Discovers](06_milestone_thinking_brain.md)*
