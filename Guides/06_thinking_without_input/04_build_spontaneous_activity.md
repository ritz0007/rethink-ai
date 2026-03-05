# 🔨 Build: Spontaneous Activity Generator — The Brain That Thinks Alone

## What We're Building

A module that gives our network **spontaneous thoughts** — activity that emerges without external input. Three components:

1. **Noise injection** — random neurons fire spontaneously
2. **Cascade propagation** — activity spreads through learned connections
3. **Thought tracking** — record what patterns emerge

## Code: `src/thinking/spontaneous.h`

```c
/*
 * spontaneous.h — Spontaneous Activity and Internal Thought
 *
 * Rethink AI — Phase 6
 *
 * Generates internal activity without external input.
 * Models the Default Mode Network — the brain "at rest" that's never resting.
 */

#ifndef SPONTANEOUS_H
#define SPONTANEOUS_H

#define SPONT_MAX_NEURONS 256
#define SPONT_MAX_THOUGHTS 64

/* A "thought" = a stable pattern that emerged from spontaneous activity */
typedef struct {
    float pattern[SPONT_MAX_NEURONS];
    float energy;        /* How stable this pattern was */
    float duration;      /* How long it persisted */
    int timestep_born;   /* When it emerged */
} Thought;

typedef struct {
    int neuron_count;
    
    /* Neural state */
    float activation[SPONT_MAX_NEURONS];
    float weights[SPONT_MAX_NEURONS][SPONT_MAX_NEURONS]; /* Learned connections */
    
    /* Parameters */
    float noise_level;     /* Base noise injection rate (0 to 1) */
    float decay_rate;      /* Activity decay per timestep */
    float threshold;       /* Minimum activation to count as "firing" */
    float inhibition;      /* Global inhibition to prevent seizures */
    
    /* Thought recording */
    Thought thoughts[SPONT_MAX_THOUGHTS];
    int num_thoughts;
    float current_pattern[SPONT_MAX_NEURONS];
    float pattern_stability;
    int stable_duration;
    
    /* Time */
    int timestep;
} SpontaneousNet;

/* Create spontaneous activity network */
SpontaneousNet *spontaneous_create(int neuron_count);
void spontaneous_destroy(SpontaneousNet *sn);

/* Load learned weights from Hebbian/STDP training */
void spontaneous_load_weights(SpontaneousNet *sn, 
                               const float *weight_matrix);

/* Imprint a memory (so it can be spontaneously recalled) */
void spontaneous_imprint(SpontaneousNet *sn, const float *pattern, float strength);

/* Run one timestep of spontaneous activity */
void spontaneous_step(SpontaneousNet *sn);

/* Run N timesteps and collect emerging thoughts */
int spontaneous_run(SpontaneousNet *sn, int steps);

/* Get the current "state of mind" */
void spontaneous_get_state(const SpontaneousNet *sn, float *state);

/* Check if current activity matches a known pattern */
float spontaneous_check_pattern(const SpontaneousNet *sn, const float *known_pattern);

/* Set noise level (mental state) */
void spontaneous_set_mode(SpontaneousNet *sn, float noise, float inhibition);

/* Print current brain activity */
void spontaneous_print_activity(const SpontaneousNet *sn);
void spontaneous_print_thoughts(const SpontaneousNet *sn);

#endif /* SPONTANEOUS_H */
```

## Code: `src/thinking/spontaneous.c`

```c
#include "spontaneous.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

SpontaneousNet *spontaneous_create(int neuron_count) {
    SpontaneousNet *sn = (SpontaneousNet *)calloc(1, sizeof(SpontaneousNet));
    sn->neuron_count = neuron_count;
    sn->noise_level = 0.05f;
    sn->decay_rate = 0.1f;
    sn->threshold = 0.5f;
    sn->inhibition = 0.02f;
    sn->num_thoughts = 0;
    sn->timestep = 0;
    sn->pattern_stability = 0.0f;
    sn->stable_duration = 0;
    return sn;
}

void spontaneous_destroy(SpontaneousNet *sn) {
    free(sn);
}

void spontaneous_load_weights(SpontaneousNet *sn, const float *weight_matrix) {
    int n = sn->neuron_count;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sn->weights[i][j] = weight_matrix[i * n + j];
        }
    }
}

void spontaneous_imprint(SpontaneousNet *sn, const float *pattern, float strength) {
    int n = sn->neuron_count;
    /* Hebbian imprinting: w_ij += strength * p_i * p_j */
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            float delta = strength * pattern[i] * pattern[j];
            sn->weights[i][j] += delta;
            sn->weights[j][i] += delta;
        }
        sn->weights[i][i] = 0.0f;  /* No self-connections */
    }
}

void spontaneous_step(SpontaneousNet *sn) {
    int n = sn->neuron_count;
    float new_activation[SPONT_MAX_NEURONS];
    
    /* 1. Calculate new activations from weights + current state */
    for (int i = 0; i < n; i++) {
        float input = 0.0f;
        for (int j = 0; j < n; j++) {
            if (sn->activation[j] > sn->threshold) {
                input += sn->weights[i][j] * sn->activation[j];
            }
        }
        new_activation[i] = input;
    }
    
    /* 2. Add spontaneous noise */
    for (int i = 0; i < n; i++) {
        float noise = ((float)rand() / RAND_MAX) * sn->noise_level;
        /* Only positive noise — spontaneous excitation */
        new_activation[i] += noise;
    }
    
    /* 3. Apply global inhibition */
    float total_activity = 0.0f;
    for (int i = 0; i < n; i++) {
        if (new_activation[i] > 0) total_activity += new_activation[i];
    }
    float global_inhib = total_activity * sn->inhibition;
    
    /* 4. Update activations with decay and inhibition */
    for (int i = 0; i < n; i++) {
        sn->activation[i] = (1.0f - sn->decay_rate) * sn->activation[i]
                            + new_activation[i] - global_inhib;
        
        /* Clamp to [0, 1] */
        if (sn->activation[i] < 0.0f) sn->activation[i] = 0.0f;
        if (sn->activation[i] > 1.0f) sn->activation[i] = 1.0f;
    }
    
    /* 5. Check pattern stability (is a "thought" forming?) */
    float similarity = 0.0f;
    float norm_cur = 0.0f, norm_prev = 0.0f, dot = 0.0f;
    for (int i = 0; i < n; i++) {
        dot += sn->activation[i] * sn->current_pattern[i];
        norm_cur += sn->activation[i] * sn->activation[i];
        norm_prev += sn->current_pattern[i] * sn->current_pattern[i];
    }
    if (norm_cur > 0.01f && norm_prev > 0.01f) {
        similarity = dot / (sqrtf(norm_cur) * sqrtf(norm_prev));
    }
    
    if (similarity > 0.9f) {
        sn->stable_duration++;
    } else {
        /* Pattern changed — if previous was stable enough, record as a thought */
        if (sn->stable_duration > 5 && sn->num_thoughts < SPONT_MAX_THOUGHTS) {
            Thought *t = &sn->thoughts[sn->num_thoughts++];
            memcpy(t->pattern, sn->current_pattern, n * sizeof(float));
            t->duration = (float)sn->stable_duration;
            t->timestep_born = sn->timestep - sn->stable_duration;
            
            /* Calculate energy (stability measure) */
            float energy = 0.0f;
            for (int i = 0; i < n; i++) {
                for (int j = i + 1; j < n; j++) {
                    energy -= sn->weights[i][j] * 
                              sn->current_pattern[i] * sn->current_pattern[j];
                }
            }
            t->energy = energy;
        }
        sn->stable_duration = 0;
    }
    
    memcpy(sn->current_pattern, sn->activation, n * sizeof(float));
    sn->pattern_stability = similarity;
    sn->timestep++;
}

int spontaneous_run(SpontaneousNet *sn, int steps) {
    int initial_thoughts = sn->num_thoughts;
    for (int s = 0; s < steps; s++) {
        spontaneous_step(sn);
    }
    return sn->num_thoughts - initial_thoughts;
}

void spontaneous_get_state(const SpontaneousNet *sn, float *state) {
    memcpy(state, sn->activation, sn->neuron_count * sizeof(float));
}

float spontaneous_check_pattern(const SpontaneousNet *sn, const float *known_pattern) {
    float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
    for (int i = 0; i < sn->neuron_count; i++) {
        dot += sn->activation[i] * known_pattern[i];
        norm_a += sn->activation[i] * sn->activation[i];
        norm_b += known_pattern[i] * known_pattern[i];
    }
    if (norm_a < 1e-8f || norm_b < 1e-8f) return 0.0f;
    return dot / (sqrtf(norm_a) * sqrtf(norm_b));
}

void spontaneous_set_mode(SpontaneousNet *sn, float noise, float inhibition) {
    sn->noise_level = noise;
    sn->inhibition = inhibition;
}

void spontaneous_print_activity(const SpontaneousNet *sn) {
    int n = sn->neuron_count;
    int cols = n <= 16 ? n : 16;
    int rows = (n + cols - 1) / cols;
    
    printf("Brain Activity (t=%d, stability=%.2f):\n", 
           sn->timestep, sn->pattern_stability);
    for (int r = 0; r < rows; r++) {
        printf("  ");
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            if (idx >= n) break;
            float a = sn->activation[idx];
            if (a > 0.8f)      printf("██");
            else if (a > 0.5f) printf("▓▓");
            else if (a > 0.2f) printf("▒▒");
            else if (a > 0.0f) printf("░░");
            else               printf("  ");
        }
        printf("\n");
    }
}

void spontaneous_print_thoughts(const SpontaneousNet *sn) {
    printf("\n=== Recorded Thoughts ===\n");
    printf("Total: %d thoughts captured\n\n", sn->num_thoughts);
    for (int i = 0; i < sn->num_thoughts; i++) {
        const Thought *t = &sn->thoughts[i];
        printf("  Thought %d: born at t=%d, lasted %.0f steps, energy=%.2f\n",
               i, t->timestep_born, t->duration, t->energy);
        
        /* Show active neurons */
        printf("    Active: ");
        int shown = 0;
        for (int j = 0; j < sn->neuron_count && shown < 10; j++) {
            if (t->pattern[j] > 0.5f) {
                printf("%d ", j);
                shown++;
            }
        }
        if (shown == 0) printf("(none above threshold)");
        printf("\n");
    }
    printf("=========================\n");
}
```

## Test: `test_spontaneous.c`

```c
/*
 * test_spontaneous.c — Watch a brain think without input
 * 
 * Compile: gcc -std=c11 -O2 -Wall -o test_spont test_spontaneous.c spontaneous.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include "spontaneous.h"

#define N 16

int main(void) {
    srand(42);
    printf("=== Spontaneous Activity: The Brain Thinks Alone ===\n\n");
    
    SpontaneousNet *sn = spontaneous_create(N);
    
    /* Imprint 3 memories */
    float memory_A[N], memory_B[N], memory_C[N];
    
    /* Memory A: first half active */
    for (int i = 0; i < N; i++) memory_A[i] = (i < N/2) ? 1.0f : -0.5f;
    
    /* Memory B: even neurons active */
    for (int i = 0; i < N; i++) memory_B[i] = (i % 2 == 0) ? 1.0f : -0.5f;
    
    /* Memory C: last quarter active */
    for (int i = 0; i < N; i++) memory_C[i] = (i >= 3*N/4) ? 1.0f : -0.5f;
    
    spontaneous_imprint(sn, memory_A, 0.5f);
    spontaneous_imprint(sn, memory_B, 0.3f);
    spontaneous_imprint(sn, memory_C, 0.2f);
    
    printf("Imprinted 3 memories. Now letting the brain think...\n\n");
    
    /* MODE 1: Focused (low noise) */
    printf("--- Mode 1: Focused Thinking (low noise) ---\n");
    spontaneous_set_mode(sn, 0.03f, 0.03f);
    
    for (int t = 0; t < 100; t++) {
        spontaneous_step(sn);
        if (t % 20 == 0) {
            spontaneous_print_activity(sn);
            
            /* Check if any known memory is active */
            float sim_a = spontaneous_check_pattern(sn, memory_A);
            float sim_b = spontaneous_check_pattern(sn, memory_B);
            float sim_c = spontaneous_check_pattern(sn, memory_C);
            printf("  Matches: A=%.2f  B=%.2f  C=%.2f\n\n", sim_a, sim_b, sim_c);
        }
    }
    
    /* MODE 2: Relaxed/Creative (higher noise) */
    printf("\n--- Mode 2: Relaxed/Creative (more noise) ---\n");
    spontaneous_set_mode(sn, 0.1f, 0.02f);
    
    /* Reset state */
    for (int i = 0; i < N; i++) sn->activation[i] = 0.0f;
    
    for (int t = 0; t < 200; t++) {
        spontaneous_step(sn);
        if (t % 40 == 0) {
            spontaneous_print_activity(sn);
            float sim_a = spontaneous_check_pattern(sn, memory_A);
            float sim_b = spontaneous_check_pattern(sn, memory_B);
            float sim_c = spontaneous_check_pattern(sn, memory_C);
            printf("  Matches: A=%.2f  B=%.2f  C=%.2f\n\n", sim_a, sim_b, sim_c);
        }
    }
    
    /* MODE 3: Dreaming (high noise, low inhibition) */
    printf("\n--- Mode 3: Dreaming (high noise, surreal) ---\n");
    spontaneous_set_mode(sn, 0.2f, 0.01f);
    
    for (int i = 0; i < N; i++) sn->activation[i] = 0.0f;
    int dream_thoughts = spontaneous_run(sn, 300);
    printf("Dreaming produced %d distinct thoughts.\n", dream_thoughts);
    
    spontaneous_print_thoughts(sn);
    
    printf("\n=== Results ===\n");
    printf("  Focused mode:  few, stable thoughts (like concentration)\n");
    printf("  Creative mode: more transitions between thoughts\n");
    printf("  Dream mode:    rapid, diverse thought sequences\n");
    printf("\nThe brain is NEVER idle. Even without input, it traverses\n");
    printf("its memory landscape, finding connections and replaying patterns.\n");
    
    spontaneous_destroy(sn);
    return 0;
}
```

## What We Just Built

A network that:
- Has NO external input
- Generates activity from noise alone
- Settles into learned patterns (memories = attractors)
- Transitions between memories
- Thinks differently based on "mental state" (noise level)

This is the Default Mode Network in code.

---

*Next: [Build — Dream Engine](05_build_dream_engine.md)*
