# 🔨 Build: Hopfield Memory — Store and Recall Patterns

## Code: `src/memory/hopfield.h`

```c
/*
 * hopfield.h — Hopfield Associative Memory
 * 
 * Rethink AI — Phase 4
 * 
 * Stores binary patterns and recalls them from partial/noisy cues.
 * Content-addressable memory — like the hippocampus.
 */

#ifndef HOPFIELD_H
#define HOPFIELD_H

#include <stdint.h>

#define HOPFIELD_MAX_NEURONS 256

typedef struct {
    int size;  /* Number of neurons */
    
    /* Connection weights (symmetric: w[i][j] == w[j][i]) */
    float weights[HOPFIELD_MAX_NEURONS][HOPFIELD_MAX_NEURONS];
    
    /* Current state of each neuron (+1 or -1) */
    float state[HOPFIELD_MAX_NEURONS];
    
    /* Memory tracking */
    int num_stored;     /* How many patterns stored */
    int max_capacity;   /* Theoretical capacity: N / (2 * ln(N)) */
    
    /* Per-pattern metadata */
    float strength[64];       /* Strength of each stored memory (decays over time) */
    float decay_rate[64];     /* How fast each memory fades */
    int recall_count[64];     /* How many times each was recalled */
    float store_time[64];     /* When each was stored (simulation time) */
} Hopfield;

/* ─── Creation ─── */
Hopfield *hopfield_create(int size);
void hopfield_destroy(Hopfield *h);

/* ─── Store a pattern ─── */
/* Pattern must be array of +1 and -1 values, length = h->size */
void hopfield_store(Hopfield *h, const float *pattern);

/* ─── Recall ─── */
/* 
 * Given a partial/noisy probe, iterate until the network settles
 * into the nearest stored pattern.
 * 
 * probe: initial state (can be noisy/incomplete)
 * max_iterations: stop after this many iterations
 * 
 * Result is stored in h->state
 * Returns: number of iterations until convergence
 */
int hopfield_recall(Hopfield *h, const float *probe, int max_iterations);

/* ─── Helpers ─── */

/* Calculate energy of current state (lower = more stable) */
float hopfield_energy(const Hopfield *h);

/* Calculate similarity between current state and a pattern (0 to 1) */
float hopfield_similarity(const Hopfield *h, const float *pattern);

/* Add noise to a pattern: flip 'noise_fraction' of bits randomly */
void hopfield_add_noise(float *pattern, int size, float noise_fraction);

/* Print the current state as a visual pattern */
void hopfield_print_state(const Hopfield *h, int width);

/* Print a pattern for comparison */
void hopfield_print_pattern(const float *pattern, int size, int width);

#endif /* HOPFIELD_H */
```

## Code: `src/memory/hopfield.c`

```c
#include "hopfield.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

Hopfield *hopfield_create(int size) {
    if (size > HOPFIELD_MAX_NEURONS) size = HOPFIELD_MAX_NEURONS;
    
    Hopfield *h = (Hopfield *)calloc(1, sizeof(Hopfield));
    h->size = size;
    h->num_stored = 0;
    h->max_capacity = (int)(size / (2.0 * log(size)));
    
    /* Zero all weights */
    memset(h->weights, 0, sizeof(h->weights));
    
    /* Initialize state to random */
    for (int i = 0; i < size; i++) {
        h->state[i] = (rand() % 2 == 0) ? 1.0f : -1.0f;
    }
    
    return h;
}

void hopfield_destroy(Hopfield *h) { free(h); }

void hopfield_store(Hopfield *h, const float *pattern) {
    /*
     * Hebbian storage rule:
     *   w[i][j] += pattern[i] × pattern[j] / N
     * 
     * Both active (+1 × +1 = +1): strengthen
     * Both inactive (-1 × -1 = +1): strengthen
     * Mixed (+1 × -1 = -1): weaken
     * 
     * This is PURE Hebbian learning!
     */
    
    for (int i = 0; i < h->size; i++) {
        for (int j = i + 1; j < h->size; j++) {
            float dw = pattern[i] * pattern[j] / (float)h->size;
            h->weights[i][j] += dw;
            h->weights[j][i] += dw;  /* Symmetric */
        }
        h->weights[i][i] = 0.0f;  /* No self-connections */
    }
    
    /* Track metadata */
    int idx = h->num_stored;
    if (idx < 64) {
        h->strength[idx] = 1.0f;
        h->decay_rate[idx] = 0.01f;
        h->recall_count[idx] = 0;
        h->store_time[idx] = 0.0f;
    }
    h->num_stored++;
    
    if (h->num_stored > h->max_capacity) {
        printf("WARNING: Stored %d patterns (capacity ~%d). "
               "Recall accuracy will degrade.\n",
               h->num_stored, h->max_capacity);
    }
}

int hopfield_recall(Hopfield *h, const float *probe, int max_iterations) {
    /* Set initial state to the probe */
    memcpy(h->state, probe, h->size * sizeof(float));
    
    for (int iter = 0; iter < max_iterations; iter++) {
        int changed = 0;
        
        /* Asynchronous update: update neurons one at a time in random order */
        for (int i = 0; i < h->size; i++) {
            /* Pick a random neuron to update */
            int idx = rand() % h->size;
            
            /* Compute input sum for this neuron */
            float sum = 0.0f;
            for (int j = 0; j < h->size; j++) {
                sum += h->weights[idx][j] * h->state[j];
            }
            
            /* Update state: threshold at 0 */
            float new_state = (sum >= 0.0f) ? 1.0f : -1.0f;
            
            if (new_state != h->state[idx]) {
                h->state[idx] = new_state;
                changed = 1;
            }
        }
        
        /* If no neuron changed → converged */
        if (!changed) return iter + 1;
    }
    
    return max_iterations;
}

float hopfield_energy(const Hopfield *h) {
    float energy = 0.0f;
    for (int i = 0; i < h->size; i++) {
        for (int j = i + 1; j < h->size; j++) {
            energy -= h->weights[i][j] * h->state[i] * h->state[j];
        }
    }
    return energy;
}

float hopfield_similarity(const Hopfield *h, const float *pattern) {
    int matches = 0;
    for (int i = 0; i < h->size; i++) {
        if ((h->state[i] > 0 && pattern[i] > 0) || 
            (h->state[i] < 0 && pattern[i] < 0)) {
            matches++;
        }
    }
    return (float)matches / (float)h->size;
}

void hopfield_add_noise(float *pattern, int size, float noise_fraction) {
    int num_flip = (int)(size * noise_fraction);
    for (int i = 0; i < num_flip; i++) {
        int idx = rand() % size;
        pattern[idx] *= -1.0f;  /* Flip the bit */
    }
}

void hopfield_print_state(const Hopfield *h, int width) {
    for (int i = 0; i < h->size; i++) {
        printf("%c", h->state[i] > 0 ? '#' : '.');
        if ((i + 1) % width == 0) printf("\n");
    }
}

void hopfield_print_pattern(const float *pattern, int size, int width) {
    for (int i = 0; i < size; i++) {
        printf("%c", pattern[i] > 0 ? '#' : '.');
        if ((i + 1) % width == 0) printf("\n");
    }
}
```

---

## Test: Store Patterns, Corrupt Them, Recall

```c
/* test_hopfield.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "memory/hopfield.h"

int main(void) {
    srand((unsigned)time(NULL));
    printf("=== RETHINK AI — Phase 4: Hopfield Memory ===\n\n");
    
    /* 64 neurons = 8×8 grid (visual patterns) */
    Hopfield *h = hopfield_create(64);
    
    /* Store 3 patterns (letters: T, L, +) */
    float pattern_T[64] = {
        1,1,1,1,1,1,1,1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
    };
    
    float pattern_L[64] = {
        1,1,-1,-1,-1,-1,-1,-1,
        1,1,-1,-1,-1,-1,-1,-1,
        1,1,-1,-1,-1,-1,-1,-1,
        1,1,-1,-1,-1,-1,-1,-1,
        1,1,-1,-1,-1,-1,-1,-1,
        1,1,-1,-1,-1,-1,-1,-1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
    };
    
    float pattern_plus[64] = {
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
       -1,-1,-1,1,1,-1,-1,-1,
    };
    
    hopfield_store(h, pattern_T);
    hopfield_store(h, pattern_L);
    hopfield_store(h, pattern_plus);
    printf("Stored 3 patterns (capacity: ~%d)\n\n", h->max_capacity);
    
    /* Show original patterns */
    printf("--- Stored Patterns ---\n");
    printf("T:\n"); hopfield_print_pattern(pattern_T, 64, 8);
    printf("\nL:\n"); hopfield_print_pattern(pattern_L, 64, 8);
    printf("\n+:\n"); hopfield_print_pattern(pattern_plus, 64, 8);
    
    /* Corrupt pattern_T with 30% noise and try to recall */
    printf("\n--- Recall Test: T with 30%% noise ---\n");
    float probe[64];
    memcpy(probe, pattern_T, sizeof(probe));
    hopfield_add_noise(probe, 64, 0.3f);
    
    printf("Corrupted input:\n");
    hopfield_print_pattern(probe, 64, 8);
    
    int iters = hopfield_recall(h, probe, 100);
    float sim = hopfield_similarity(h, pattern_T);
    
    printf("Recalled (after %d iterations, %.0f%% match):\n", iters, sim * 100);
    hopfield_print_state(h, 8);
    
    if (sim > 0.9f) {
        printf("✓ Successfully recalled T from noisy input!\n");
    }
    
    hopfield_destroy(h);
    return 0;
}
```

---

*Next: [06_build_memory_with_decay.md](06_build_memory_with_decay.md) — Add forgetting to the memory system*
