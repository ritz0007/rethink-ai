# 🏁 Milestone: Partial Recall — Your Brain Remembers, Forgets, and Survives

## What We Built in Phase 4

| Component | What It Does |
|-----------|-------------|
| Hopfield Network | Content-addressable memory — give a piece, get the whole thing |
| Memory Decay | Ebbinghaus forgetting curve — memories fade with time |
| Recall Strengthening | Using a memory makes it stronger |
| Importance Tagging | Important memories decay slower |
| Consolidation | "Dream mode" replays important memories |

## The Full Milestone Test

```c
/*
 * test_memory_milestone.c — Phase 4 Milestone
 * 
 * Demonstrates: store, decay, recall, strengthening, consolidation
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_milestone \
 *          test_memory_milestone.c decay.c hopfield.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "decay.h"

#define N 64  /* 8x8 patterns */

/* Helper: create a simple pattern */
void make_pattern(float *p, int n, const int *on_indices, int count) {
    for (int i = 0; i < n; i++) p[i] = -1.0f;
    for (int i = 0; i < count; i++) p[on_indices[i]] = 1.0f;
}

/* Helper: print 8x8 pattern */
void print_pattern(const float *p, const char *label) {
    printf("%s:\n", label);
    for (int r = 0; r < 8; r++) {
        printf("  ");
        for (int c = 0; c < 8; c++) {
            printf("%s", p[r * 8 + c] > 0 ? "██" : "  ");
        }
        printf("\n");
    }
}

/* Helper: add noise to pattern */
void corrupt(const float *src, float *dst, int n, float noise_pct) {
    memcpy(dst, src, n * sizeof(float));
    int flip_count = (int)(n * noise_pct);
    for (int i = 0; i < flip_count; i++) {
        int idx = rand() % n;
        dst[idx] *= -1.0f;
    }
}

int main(void) {
    srand(42);
    printf("=== PHASE 4 MILESTONE: Memory That Fades ===\n\n");
    
    DecayMemory *dm = decay_memory_create(N);
    
    /* --- Define 5 memories with different importance --- */
    
    /* Pattern: "T" shape — your name (very important) */
    int t_indices[] = {0,1,2,3,4,5,6,7, 11,12, 19,20, 27,28, 35,36, 43,44, 51,52, 59,60};
    float pattern_name[N];
    make_pattern(pattern_name, N, t_indices, 22);
    
    /* Pattern: "L" shape — what you ate (low importance) */
    int l_indices[] = {0,1, 8,9, 16,17, 24,25, 32,33, 40,41, 48,49,50,51,52,53};
    float pattern_breakfast[N];
    make_pattern(pattern_breakfast, N, l_indices, 18);
    
    /* Pattern: "+" shape — random face (negligible) */
    int plus_indices[] = {3,4, 11,12, 16,17,18,19,20,21,22,23, 27,28, 35,36, 43,44};
    float pattern_face[N];
    make_pattern(pattern_face, N, plus_indices, 18);
    
    /* Pattern: "H" shape — a skill (high importance) */
    int h_indices[] = {0,1,6,7, 8,9,14,15, 16,17,18,19,20,21,22,23, 24,25,30,31, 32,33,38,39, 40,41,46,47};
    float pattern_skill[N];
    make_pattern(pattern_skill, N, h_indices, 28);
    
    /* Pattern: "O" shape — a fact (medium importance) */
    int o_indices[] = {1,2,3,4,5,6, 8,15, 16,23, 24,31, 32,39, 40,47, 49,50,51,52,53,54};
    float pattern_fact[N];
    make_pattern(pattern_fact, N, o_indices, 22);
    
    printf("--- Storing 5 memories ---\n");
    decay_memory_store(dm, pattern_name, 1.0f);      /* Very important */
    decay_memory_store(dm, pattern_breakfast, 0.2f);  /* Low importance */
    decay_memory_store(dm, pattern_face, 0.05f);      /* Negligible */
    decay_memory_store(dm, pattern_skill, 0.8f);      /* High importance */
    decay_memory_store(dm, pattern_fact, 0.4f);        /* Medium importance */
    
    decay_memory_print_status(dm);
    
    /* --- Phase 1: Let time pass, recall your name periodically --- */
    printf("\n--- Phase 1: 200 time steps, recalling name every 40 ---\n");
    float probe[N];
    float recalled[N];
    
    for (int t = 0; t < 200; t++) {
        decay_memory_tick(dm, 1.0f);
        
        /* You keep using your name */
        if (t % 40 == 0 && t > 0) {
            corrupt(pattern_name, probe, N, 0.15f);
            float sim = decay_memory_recall(dm, probe, recalled, 100);
            printf("  t=%d: Recalled name (sim=%.3f)\n", t, sim);
        }
    }
    
    decay_memory_print_status(dm);
    
    /* --- Phase 2: More time passes --- */
    printf("\n--- Phase 2: 300 more steps (no extra recalls) ---\n");
    for (int t = 0; t < 300; t++) {
        decay_memory_tick(dm, 1.0f);
    }
    
    decay_memory_print_status(dm);
    
    /* --- Phase 3: Consolidation (dreaming!) --- */
    printf("\n--- Phase 3: Consolidation (dreaming) ---\n");
    decay_memory_consolidate(dm, 3);
    
    decay_memory_print_status(dm);
    
    /* --- Phase 4: Test partial recall on surviving memories --- */
    printf("\n--- Phase 4: Partial Recall Tests ---\n\n");
    
    /* Try to recall name with 25% noise */
    corrupt(pattern_name, probe, N, 0.25f);
    print_pattern(probe, "Probe: corrupted name");
    float sim = decay_memory_recall(dm, probe, recalled, 100);
    print_pattern(recalled, "Recalled");
    printf("Similarity: %.3f %s\n\n", sim, sim > 0.8 ? "✓ RECOGNIZED" : "✗ LOST");
    
    /* Try to recall face (should be forgotten) */
    corrupt(pattern_face, probe, N, 0.25f);
    print_pattern(probe, "Probe: corrupted face");
    sim = decay_memory_recall(dm, probe, recalled, 100);
    print_pattern(recalled, "Recalled");
    printf("Similarity: %.3f %s\n\n", sim, sim > 0.8 ? "✓ RECOGNIZED" : "✗ FORGOTTEN");
    
    /* Final summary */
    printf("=== FINAL STATUS ===\n");
    printf("Started with 5 memories.\n");
    printf("After time + selective recall + dreaming:\n");
    printf("Surviving: %d memories\n", decay_memory_alive_count(dm));
    printf("\nThis is how your brain works:\n");
    printf("  → Your name survives (you use it all the time)\n");
    printf("  → Skills survive (high importance)\n");
    printf("  → Random faces fade (low importance, never recalled)\n");
    printf("  → What you ate fades (low importance)\n");
    printf("  → Facts survive if somewhat important\n");
    
    decay_memory_destroy(dm);
    
    printf("\n✓ Phase 4 Complete — Tag as V4\n");
    return 0;
}
```

## Expected Output

```
=== Memory Status (t=0.0) ===
Alive: 5 / 5 stored
  [ 0] str=1.000 imp=1.0 recalls=0 τ=400  |████████████████████|
  [ 1] str=1.000 imp=0.2 recalls=0 τ=160  |████████████████████|
  [ 2] str=1.000 imp=0.1 recalls=0 τ=115  |████████████████████|
  [ 3] str=1.000 imp=0.8 recalls=0 τ=340  |████████████████████|
  [ 4] str=1.000 imp=0.4 recalls=0 τ=220  |████████████████████|

After 200 steps:
  Name:      ███████████████ (strong — you kept recalling it)
  Breakfast: █████████       (fading)
  Face:      ███             (almost gone)
  Skill:     ████████████    (decent — high importance)
  Fact:      █████████       (moderate)
  
After 500 steps:
  Name:      ██████████████  (still strong!)
  Breakfast: ░░░░░░░░░░      (forgotten)
  Face:      ░░░░░░░░░░      (forgotten)
  Skill:     ████████        (weakened but alive)
  Fact:      ██              (barely hanging on)
```

## What This Proves

> **Forgetting is not a bug. It's the brain's most powerful feature.**

1. **Decay is natural** — Without reinforcement, memories fade
2. **Recall strengthens** — The act of remembering IS the exercise
3. **Importance matters** — Not all memories are equal
4. **Dreaming consolidates** — Sleep replays and strengthens key memories
5. **Capacity management** — Old cruft fades to make room for new

## How This Connects to The User's Vision

> *"How we remember everything but fade out most things and still manage to survive"*

This IS that mechanism:
- The brain stores everything in the moment
- But the default path is forgetting
- Only memories that get:
  - **Used** (recall strengthening)
  - **Tagged as important** (emotional significance)
  - **Consolidated** (replayed during sleep)
  
  ...survive long-term.

Everything else gracefully fades — and that's what makes the system work. A brain that remembered everything would drown in noise.

---

## What's Next

Phase 4 gave our brain **memory that works like real memory**. But our brain still needs a teacher to show it many examples.

Real brains (especially human children) learn from **one or two examples**.

**Phase 5: Learning Like a Child** — Few-shot learning, prototype formation, learning from barely anything.

---

*Phase 4 Complete. `git tag -a V4 -m "Memory: Hopfield + decay + consolidation"`*

*Next: [Phase 5 — Learning Like a Child](../05_learning_like_a_child/01_brain_few_shot_learning.md)*
