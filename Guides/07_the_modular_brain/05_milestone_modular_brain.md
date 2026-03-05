# 🏁 Milestone: The Modular Brain — Working Together

## Phase 7 Summary

We organized the brain into **independent modules** with **centralized routing**:

| Module | Brain Region | What It Does |
|--------|-------------|-------------|
| Visual | Occipital lobe | Edge detection, feature extraction |
| Memory | Hippocampus | Associative storage, decay, recall |
| Learning | Temporal lobe | Prototype formation, classification |
| Thinking | Default Mode Network | Spontaneous activity, dreaming |
| Emotion | Amygdala | Importance tagging, priority |
| Executive | Prefrontal cortex | Decision making, attention control |
| **Thalamus** | **Thalamus** | **Routes and gates all signals** |

## Integration Test

```c
/*
 * test_modular_brain.c — Phase 7 Milestone
 *
 * The complete modular brain in action.
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_modular \
 *          test_modular_brain.c brain.c emotion.c dream.c \
 *          spontaneous.c decay.c hopfield.c prototype.c \
 *          retina.c som.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include "brain.h"
#include "emotion.h"

#define FDIM 8

int main(void) {
    srand(42);
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  PHASE 7 MILESTONE: The Modular Brain       ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    
    Brain *brain = brain_create();
    Amygdala *amy = amygdala_create();
    
    /* === Scenario: A Day in the Life === */
    
    printf("=== Morning: Learn some things ===\n");
    
    /* Learn categories */
    float cat_features[FDIM] = {0.8, 0.9, 0.3, 1.0, 0.2, 0.6, 0.0, 0.0};
    float dog_features[FDIM] = {0.8, 0.5, 0.6, 1.0, 0.9, 0.5, 0.1, 0.0};
    float snake_features[FDIM] = {0.0, 1.0, 0.5, 0.3, 0.0, 0.3, 0.0, 0.0};
    
    brain_learn(brain, cat_features, FDIM, "cat");
    brain_learn(brain, dog_features, FDIM, "dog");
    brain_learn(brain, snake_features, FDIM, "snake");
    
    /* Associate snake with fear */
    amygdala_associate(amy, snake_features, FDIM, EMO_FEAR, 0.9f);
    /* Associate cat with reward */
    amygdala_associate(amy, cat_features, FDIM, EMO_REWARD, 0.6f);
    
    /* === Encounter 1: See a cat === */
    printf("\n=== Encounter: See a cat ===\n");
    float new_cat[FDIM] = {0.7, 0.85, 0.35, 0.95, 0.25, 0.55, 0.0, 0.0};
    
    /* Emotional fast-path */
    EmotionType emo;
    float emo_intensity = amygdala_evaluate(amy, new_cat, FDIM, &emo);
    printf("  Emotion: %s (intensity=%.2f)\n", EMOTION_NAMES[emo], emo_intensity);
    amygdala_print_state(amy);
    
    /* Cognitive path */
    char label[32];
    float conf;
    brain_classify(brain, new_cat, FDIM, label, &conf);
    printf("  Classification: %s (confidence=%.3f)\n", label, conf);
    
    /* === Encounter 2: See a snake! === */
    printf("\n=== Encounter: See a SNAKE! ===\n");
    float new_snake[FDIM] = {0.05, 0.95, 0.4, 0.2, 0.0, 0.35, 0.0, 0.0};
    
    /* Fast danger check (before cortex finishes!) */
    if (amygdala_danger_check(amy, new_snake, FDIM)) {
        printf("  ⚠️  DANGER DETECTED (amygdala fast path)\n");
        printf("  → Attention LOCKED on threat\n");
        brain_shift_attention(brain, REGION_VISUAL);
    }
    
    emo_intensity = amygdala_evaluate(amy, new_snake, FDIM, &emo);
    printf("  Emotion: %s (intensity=%.2f)\n", EMOTION_NAMES[emo], emo_intensity);
    amygdala_print_state(amy);
    
    /* This memory will be stored with HIGH importance */
    float importance = amygdala_get_importance_modifier(amy);
    printf("  → Storing with importance = %.2f (fear boost!)\n", importance);
    
    brain_classify(brain, new_snake, FDIM, label, &conf);
    printf("  Classification: %s (confidence=%.3f)\n", label, conf);
    
    /* === Encounter 3: Something never seen === */
    printf("\n=== Encounter: Something UNKNOWN ===\n");
    float alien[FDIM] = {0.1, 0.1, 0.9, 0.0, 0.5, 0.0, 0.9, 0.8};
    
    emo_intensity = amygdala_evaluate(amy, alien, FDIM, &emo);
    printf("  Emotion: %s (intensity=%.2f)\n", EMOTION_NAMES[emo], emo_intensity);
    amygdala_print_state(amy);
    
    brain_classify(brain, alien, FDIM, label, &conf);
    printf("  Classification: %s (confidence=%.3f)\n", label, conf);
    
    /* === Let time pass, then sleep === */
    printf("\n=== Evening: Time passes ===\n");
    for (int t = 0; t < 100; t++) {
        brain_tick(brain, 1.0f);
        amygdala_tick(amy, 1.0f);
    }
    
    printf("\n=== Night: Sleep and dream ===\n");
    brain_sleep(brain, 3);
    
    /* === Next morning === */
    printf("\n=== Next Morning: What survived? ===\n");
    brain_print_status(brain);
    
    /* The snake memory should be strongest (fear = deep encoding) */
    /* The cat memory should be good (reward = good encoding) */
    /* The alien memory might partially fade (surprise but no strong emotion) */
    
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║           PHASE 7 COMPLETE                   ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║                                              ║\n");
    printf("║  The brain now has:                          ║\n");
    printf("║  ✓ Modular architecture (6 regions)          ║\n");
    printf("║  ✓ Central routing (thalamus)                ║\n");
    printf("║  ✓ Competitive attention (zero-sum)          ║\n");
    printf("║  ✓ Emotion system (amygdala)                 ║\n");
    printf("║  ✓ Fast danger detection                     ║\n");
    printf("║  ✓ Emotion-modulated memory                  ║\n");
    printf("║  ✓ Activity logging                          ║\n");
    printf("║                                              ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    
    amygdala_destroy(amy);
    brain_destroy(brain);
    
    printf("\n✓ Phase 7 Complete — Tag as V7\n");
    printf("  git tag -a V7 -m \"Modular brain with routing and emotions\"\n");
    return 0;
}
```

## What Makes This Different from Standard AI

| Standard AI | Rethink Brain |
|------------|---------------|
| One model, one task | Multiple specialized regions |
| No routing | Thalamic attention gating |
| No emotions | Emotional priority system |
| All inputs equal | Context-dependent processing |
| No surprise detection | Novelty triggers exploration |
| Flat architecture | Hierarchical, modular |

**Phase 8: Understanding** — The brain can perceive, learn, remember, think, dream, and feel. But does it *understand*? Next, we tackle the hardest question: what does it mean to truly understand something?

---

*`git tag -a V7 -m "Modular brain architecture with thalamus routing and amygdala emotions"`*

*Next: [Phase 8 — Understanding](../08_understanding/01_brain_what_is_understanding.md)*
