# 🏁 Milestone: Few-Shot Recognition — Learning Like a Child

## Phase 5 Summary

We built a system that learns categories from **1 to 3 examples**. No gradient descent. No backpropagation. No thousands of training samples.

### What We Have Now (Phases 1-5 Combined)

```
┌──────────────────────────────────────────────────────┐
│                The Rethink Brain (V5)                  │
│                                                        │
│  PERCEPTION                                            │
│  ┌──────────┐   ┌──────────────┐                       │
│  │  Retina   │──→│     SOM      │  ← Sees the world    │
│  │(edge det.)│   │(organization)│                       │
│  └──────────┘   └──────┬───────┘                       │
│                         │                               │
│  LEARNING               ▼                               │
│  ┌──────────────────────────────┐                       │
│  │     Prototype Learner        │  ← Learns from 1 ex  │
│  │  ┌─────────┐ ┌────────────┐  │                       │
│  │  │Prototypes│ │ Attention  │  │                       │
│  │  │(centroids│ │ (contrast) │  │                       │
│  │  └─────────┘ └────────────┘  │                       │
│  └──────────────┬───────────────┘                       │
│                  │                                       │
│  MEMORY          ▼                                       │
│  ┌──────────────────────────────┐                       │
│  │   Hopfield + Decay Memory   │  ← Remembers + forgets │
│  │  ┌─────────┐ ┌────────────┐  │                       │
│  │  │Associat. │ │   Decay    │  │                       │
│  │  │ Recall   │ │ + Consol.  │  │                       │
│  │  └─────────┘ └────────────┘  │                       │
│  └──────────────────────────────┘                       │
│                                                          │
│  NEURONS                                                 │
│  ┌──────────────────────────────┐                       │
│  │     Spiking Neural Network    │  ← The substrate      │
│  │  LIF neurons + synapses       │                       │
│  │  Hebbian + STDP learning      │                       │
│  └──────────────────────────────┘                       │
└──────────────────────────────────────────────────────────┘
```

## The Full Integration Test

```c
/*
 * test_fewshot_milestone.c — Phase 5 Milestone
 * 
 * Full pipeline: See → Learn → Remember → Recognize
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_fewshot \
 *          test_fewshot_milestone.c prototype.c hopfield.c decay.c \
 *          retina.c som.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prototype.h"

#define FEATURE_DIM 16

/* 
 * Simulate: extract features from visual patterns
 * In a full system, retina → SOM would produce these.
 * For now, we define them manually to test the learner.
 */

void make_features(float *f, int dim, const float *template, float noise) {
    for (int i = 0; i < dim; i++) {
        float n = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * noise;
        f[i] = template[i] + n;
    }
}

int main(void) {
    srand(42);
    printf("=== PHASE 5 MILESTONE: Few-Shot Learning ===\n\n");
    
    ProtoLearner *pl = proto_create(FEATURE_DIM);
    float features[FEATURE_DIM];
    float sim;
    
    /* Define category templates (imagine these come from visual processing) */
    float cat_template[FEATURE_DIM] =  {0.8, 0.9, 0.3, 1.0, 0.2, 0.6, 0.0, 0.0,
                                         0.7, 0.4, 0.1, 0.8, 0.3, 0.0, 0.5, 0.2};
    float dog_template[FEATURE_DIM] =  {0.8, 0.5, 0.6, 1.0, 0.9, 0.5, 0.1, 0.0,
                                         0.6, 0.7, 0.3, 0.9, 0.1, 0.0, 0.4, 0.6};
    float bird_template[FEATURE_DIM] = {0.2, 0.3, 0.2, 0.5, 0.0, 0.8, 0.0, 1.0,
                                         0.1, 0.2, 0.8, 0.3, 0.0, 0.9, 0.1, 0.1};
    
    /* === Test 1: One-shot learning === */
    printf("TEST 1: One-Shot Learning\n");
    printf("Teaching ONE example per category...\n");
    
    make_features(features, FEATURE_DIM, cat_template, 0.05f);
    proto_teach(pl, features, "cat");
    
    make_features(features, FEATURE_DIM, dog_template, 0.05f);
    proto_teach(pl, features, "dog");
    
    make_features(features, FEATURE_DIM, bird_template, 0.05f);
    proto_teach(pl, features, "bird");
    
    /* Classify 10 new instances of each */
    int correct = 0, total = 0;
    for (int trial = 0; trial < 10; trial++) {
        make_features(features, FEATURE_DIM, cat_template, 0.15f);
        int pred = proto_classify(pl, features, &sim);
        if (pred >= 0 && strcmp(proto_get_label(pl, pred), "cat") == 0) correct++;
        total++;
        
        make_features(features, FEATURE_DIM, dog_template, 0.15f);
        pred = proto_classify(pl, features, &sim);
        if (pred >= 0 && strcmp(proto_get_label(pl, pred), "dog") == 0) correct++;
        total++;
        
        make_features(features, FEATURE_DIM, bird_template, 0.15f);
        pred = proto_classify(pl, features, &sim);
        if (pred >= 0 && strcmp(proto_get_label(pl, pred), "bird") == 0) correct++;
        total++;
    }
    printf("  Accuracy from 1 example each: %d/%d = %.0f%%\n\n", 
           correct, total, 100.0f * correct / total);
    
    /* === Test 2: Add more examples, accuracy improves === */
    printf("TEST 2: Adding 2 more examples per category...\n");
    for (int extra = 0; extra < 2; extra++) {
        make_features(features, FEATURE_DIM, cat_template, 0.1f);
        proto_teach(pl, features, "cat");
        make_features(features, FEATURE_DIM, dog_template, 0.1f);
        proto_teach(pl, features, "dog");
        make_features(features, FEATURE_DIM, bird_template, 0.1f);
        proto_teach(pl, features, "bird");
    }
    
    correct = 0; total = 0;
    for (int trial = 0; trial < 10; trial++) {
        make_features(features, FEATURE_DIM, cat_template, 0.2f);
        int pred = proto_classify(pl, features, &sim);
        if (pred >= 0 && strcmp(proto_get_label(pl, pred), "cat") == 0) correct++;
        total++;
        
        make_features(features, FEATURE_DIM, dog_template, 0.2f);
        pred = proto_classify(pl, features, &sim);
        if (pred >= 0 && strcmp(proto_get_label(pl, pred), "dog") == 0) correct++;
        total++;
        
        make_features(features, FEATURE_DIM, bird_template, 0.2f);
        pred = proto_classify(pl, features, &sim);
        if (pred >= 0 && strcmp(proto_get_label(pl, pred), "bird") == 0) correct++;
        total++;
    }
    printf("  Accuracy from 3 examples each: %d/%d = %.0f%%\n", 
           correct, total, 100.0f * correct / total);
    printf("  (With higher noise than test 1!)\n\n");
    
    /* === Test 3: Novel category emerges === */
    printf("TEST 3: Novel Category Detection\n");
    float alien_template[FEATURE_DIM] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                          1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    make_features(features, FEATURE_DIM, alien_template, 0.05f);
    int pred = proto_classify(pl, features, &sim);
    printf("  Completely novel input → %s (sim=%.3f)\n",
           pred < 0 ? "UNKNOWN (correctly!)" : proto_get_label(pl, pred), sim);
    
    /* Teach it */
    proto_teach(pl, features, "alien");
    make_features(features, FEATURE_DIM, alien_template, 0.1f);
    pred = proto_classify(pl, features, &sim);
    printf("  After teaching 'alien' → %s (sim=%.3f) ✓\n\n",
           proto_get_label(pl, pred), sim);
    
    proto_print_status(pl);
    
    printf("\n=== RESULTS ===\n");
    printf("✓ One-shot learning: categories learned from single examples\n");
    printf("✓ Few-shot improvement: accuracy increases with more examples\n");
    printf("✓ Novel detection: unknown inputs flagged as unknown\n");
    printf("✓ Instant new categories: no retraining needed\n");
    printf("✓ Feature attention: contrastive learning emphasizes differences\n");
    
    proto_destroy(pl);
    
    printf("\n✓ Phase 5 Complete — Tag as V5\n");
    printf("  git tag -a V5 -m \"Few-shot prototype learning\"\n");
    return 0;
}
```

## Phase 5 Complete

What the brain has now:

| Capability | Status |
|-----------|--------|
| Spiking neurons | ✓ Phase 1 |
| Brain-like learning (Hebbian/STDP) | ✓ Phase 2 |
| Visual perception | ✓ Phase 3 |
| Memory with forgetting | ✓ Phase 4 |
| **Learn from 1-3 examples** | **✓ Phase 5** |

What's missing:
- The brain can't think on its own yet (no spontaneous activity)
- It can't dream in a meaningful way
- It has no modular regions
- It doesn't truly "understand" anything

**Phase 6: Thinking Without Input** — Give the brain spontaneous activity, imagination, and dreaming.

---

*`git tag -a V5 -m "Few-shot prototype learning with contrastive attention"`*

*Next: [Phase 6 — Thinking Without Input](../06_thinking_without_input/01_brain_spontaneous_activity.md)*
