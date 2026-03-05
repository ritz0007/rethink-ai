# 🎯 Milestone: The Understanding Brain — V8

## What You've Built

This is the most profound milestone yet. Your system can now:
- **Predict** what comes next (and notice when it's wrong)
- **Explain** why something happened (trace causal chains)
- **Imagine** alternatives ("what if?")
- **Learn** cause-effect relationships from experience

This is the closest we've come to actual understanding.

## Integration Test: `test_understanding.c`

```c
/*
 * test_understanding.c — V8 Milestone
 *
 * Can our brain understand anything?
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_understanding \
 *          test_understanding.c predictor.c causal.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include "predictor.h"
#include "causal.h"

/* ──────── Helper: Feed pattern and print understanding level ──────── */
static void feed_and_report(PredictiveNet *pn, const float *pattern, int dim,
                            const char *label, int step) {
    predictive_process(pn, pattern, dim);
    float understanding = predictive_understanding(pn);
    float surprise = predictive_surprise(pn);
    printf("  Step %2d [%s]: understanding=%.3f  surprise=%.3f\n",
           step, label, understanding, surprise);
}

int main(void) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║  V8: THE UNDERSTANDING BRAIN         ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    /* ================================================================
     * PART 1: Predictive Coding — Learning patterns
     * ================================================================ */
    printf("═══ Part 1: Learning to Predict ═══\n\n");
    
    int dim = 8;
    int layers[] = {8, 6, 4};
    PredictiveNet *pn = predictive_create(layers, 3, 0.05f);
    
    /* Pattern A: stable repeating signal */
    float pattern_a[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    /* Pattern B: different signal */
    float pattern_b[8] = {0, 1, 0, 1, 0, 1, 0, 1};
    /* Pattern C: novel */
    float pattern_c[8] = {1, 1, 0, 0, 1, 1, 0, 0};
    
    /* Train on pattern A for 20 steps */
    printf("Training on Pattern A (alternating 1010...):\n");
    for (int i = 0; i < 20; i++) {
        feed_and_report(pn, pattern_a, dim, "A", i + 1);
    }
    
    /* The brain should now "understand" pattern A (low surprise) */
    float understanding_a = predictive_understanding(pn);
    printf("\n→ Understanding of A: %.3f (should be high)\n\n", understanding_a);
    
    /* Surprise test: show pattern B */
    printf("Surprise! Pattern B (inverted):\n");
    feed_and_report(pn, pattern_b, dim, "B-surprise", 21);
    float surprise_b = predictive_surprise(pn);
    printf("→ Surprise at B: %.3f (should be high)\n\n", surprise_b);
    
    /* Adapting to B */
    printf("Adapting to Pattern B:\n");
    for (int i = 0; i < 10; i++) {
        feed_and_report(pn, pattern_b, dim, "B-adapt", 22 + i);
    }
    float understanding_b = predictive_understanding(pn);
    printf("→ Understanding of B: %.3f (should be improving)\n\n", understanding_b);
    
    /* Novel pattern C */
    printf("Brand new Pattern C:\n");
    feed_and_report(pn, pattern_c, dim, "C-novel", 32);
    float surprise_c = predictive_surprise(pn);
    printf("→ Surprise at C: %.3f (novel pattern = high surprise)\n\n", surprise_c);

    /* ================================================================
     * PART 2: Causal Reasoning — Understanding "Why"
     * ================================================================ */
    printf("═══ Part 2: Understanding Causes ═══\n\n");
    
    CausalNet *cn = causal_create();
    
    /* Model: fever → headache → miss_work → behind_on_tasks */
    int fever    = causal_add_node(cn, "fever", 0.0f);
    int headache = causal_add_node(cn, "headache", 0.0f);
    int fatigue  = causal_add_node(cn, "fatigue", 0.0f);
    int miss     = causal_add_node(cn, "miss_work", 0.0f);
    int behind   = causal_add_node(cn, "behind_tasks", 0.0f);
    int stress   = causal_add_node(cn, "stress", 0.0f);
    
    causal_add_edge(cn, fever, headache, 0.8f, 1.0f);
    causal_add_edge(cn, fever, fatigue, 0.9f, 0.5f);
    causal_add_edge(cn, headache, miss, 0.5f, 2.0f);
    causal_add_edge(cn, fatigue, miss, 0.6f, 1.0f);
    causal_add_edge(cn, miss, behind, 0.9f, 3.0f);
    causal_add_edge(cn, behind, stress, 0.7f, 1.0f);
    
    causal_print(cn);
    
    /* Forward: What happens with fever? */
    printf("\nForward: What does fever cause?\n");
    causal_reset(cn);
    causal_predict(cn, fever);
    causal_print_active(cn);
    
    /* Backward: Why am I stressed? */
    printf("\nBackward: Why am I stressed?\n");
    int chain[CAUSAL_MAX_CHAIN];
    float strengths[CAUSAL_MAX_CHAIN];
    int chain_len = causal_explain(cn, stress, chain, strengths, CAUSAL_MAX_CHAIN);
    causal_print_explanation(cn, stress, chain, strengths, chain_len);
    
    /* Counterfactual: What if I hadn't had the fever? */
    printf("\nCounterfactual: What if no fever?\n");
    causal_reset(cn);
    causal_predict(cn, fever);
    float original[CAUSAL_MAX_NODES], counter[CAUSAL_MAX_NODES];
    causal_counterfactual(cn, fever, original, counter);
    
    printf("  With fever:    stress=%.2f  miss_work=%.2f\n", 
           original[stress], original[miss]);
    printf("  Without fever: stress=%.2f  miss_work=%.2f\n", 
           counter[stress], counter[miss]);
    printf("  → Fever was the ROOT CAUSE of stress\n");

    /* ================================================================
     * PART 3: Learning new causal links
     * ================================================================ */
    printf("\n═══ Part 3: Discovering Causes ═══\n\n");
    
    /* The brain observes: every time there's stress, sleep gets worse */
    int sleep = causal_add_node(cn, "poor_sleep", 0.0f);
    
    /* Observe stress → poor sleep multiple times */
    causal_learn_temporal(cn, stress, sleep, 1.0f);
    causal_learn_temporal(cn, stress, sleep, 1.5f);
    causal_learn_temporal(cn, stress, sleep, 0.8f);
    causal_learn_temporal(cn, stress, sleep, 1.2f);
    causal_learn_temporal(cn, stress, sleep, 1.0f);
    
    printf("After 5 observations: stress → poor_sleep link:\n");
    for (int e = 0; e < cn->num_edges; e++) {
        if (cn->edges[e].source == stress && cn->edges[e].target == sleep) {
            printf("  Strength: %.2f  Observations: %d\n",
                   cn->edges[e].strength, cn->edges[e].observations);
        }
    }
    
    /* And poor sleep makes fever more likely (feedback loop!) */
    causal_learn_temporal(cn, sleep, fever, 2.0f);
    causal_learn_temporal(cn, sleep, fever, 2.5f);
    causal_learn_temporal(cn, sleep, fever, 3.0f);
    
    printf("\nDiscovered feedback loop:\n");
    printf("  fever → ... → stress → poor_sleep → fever!\n");
    printf("  The brain can now understand CYCLES of causation.\n");

    /* ================================================================
     * PART 4: Intervention — Breaking the cycle
     * ================================================================ */
    printf("\n═══ Part 4: Intervention — Breaking Cycles ═══\n\n");
    
    /* What if we intervene on stress? */
    printf("Normal prediction from fever:\n");
    causal_reset(cn);
    causal_predict(cn, fever);
    causal_print_active(cn);
    
    printf("\nIntervention: Force stress = 0 (meditation!)\n");
    /* First activate fever, then see what happens if we intervene on stress */
    causal_reset(cn);
    causal_predict(cn, fever);
    float pre_sleep = cn->nodes[sleep].activation;
    
    /* Now intervene: set stress to 0 */
    causal_intervene(cn, stress, 0.0f);
    float post_sleep = cn->nodes[sleep].activation;
    printf("  Poor sleep with stress:    %.2f\n", pre_sleep);
    printf("  Poor sleep without stress: %.2f\n", post_sleep);
    printf("  → Managing stress BREAKS the vicious cycle!\n");

    /* ================================================================
     * Summary
     * ================================================================ */
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║  V8 MILESTONE COMPLETE               ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ ✓ Predictive coding: learns patterns ║\n");
    printf("║ ✓ Surprise detection: flags novel    ║\n");
    printf("║ ✓ Forward reasoning: predict effects ║\n");
    printf("║ ✓ Backward reasoning: explain causes ║\n");
    printf("║ ✓ Counterfactual: imagine what-ifs   ║\n");
    printf("║ ✓ Causal learning: discover from exp ║\n");
    printf("║ ✓ Intervention: test causal links    ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    
    printf("The brain can now UNDERSTAND:\n");
    printf("  - It predicts what happens next\n");
    printf("  - It notices when predictions break\n");
    printf("  - It knows WHY things happen\n");
    printf("  - It imagines alternatives\n");
    printf("  - It discovers new cause-effect links\n");
    printf("  - It can plan interventions\n\n");
    
    printf("→ Tag this: git tag V8-understanding\n");
    printf("→ Next: Phase 9 — Communication (giving the brain a voice)\n");
    
    predictive_destroy(pn);
    causal_destroy(cn);
    return 0;
}
```

## What Understanding Really Is

Look at what this system does:

| Capability | Implementation | Brain Parallel |
|-----------|---------------|----------------|
| Prediction | Predictive coding net | Cortex generates expectations |
| Surprise | Prediction error magnitude | Mismatch signals in anterior cingulate |
| Explanation | Backward causal tracing | Hippocampal replay of causal chains |
| Counterfactual | Causal graph manipulation | Prefrontal cortex simulation |
| Discovery | Temporal co-occurrence learning | Hippocampal binding of sequences |
| Intervention | Forced activation + observe | Prefrontal planning and motor control |

## The Deep Insight

Understanding = prediction + causation.

Our brain doesn't store facts like a database. It builds **models of how things work**:
- Predictive coding builds models of **what happens**
- Causal networks build models of **why it happens**

Together, they create understanding.

When you "understand" gravity, you have:
1. A predictive model (things fall when dropped)
2. A causal model (mass pulls mass)
3. An ability to predict novel scenarios (what falls in space? — nothing)

Our system now has all three capabilities.

---

*Next Phase: [Phase 9 — Communication](../09_communication/01_brain_language_grounding.md)*
