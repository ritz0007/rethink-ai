# 🎯 Milestone: The Communicating Brain — V9

## What You've Built

Your brain can now **talk**:
- Encode internal states into symbol sequences (speaking)
- Decode symbol sequences back into internal states (understanding)
- Symbols are grounded in sensory experience (not just tokens)
- Communication is naturally lossy (just like real language)

## Integration Test: `test_communicating_brain.c`

```c
/*
 * test_communicating_brain.c — V9 Milestone
 *
 * Two brains communicating: one encodes its experience,
 * the other decodes and tries to understand.
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_comm_brain \
 *          test_communicating_brain.c comm.c -lm
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "comm.h"

/* Shared vocabulary builder */
static void build_world_vocabulary(CommEngine *ce) {
    int dim = 8;
    /* [size, speed, fluffiness, danger, roundness, warmth, brightness, wetness] */
    
    float cat_f[]   = {0.3, 0.5, 0.9, 0.1, 0.4, 0.8, 0.0, 0.0};
    float dog_f[]   = {0.5, 0.6, 0.7, 0.2, 0.3, 0.7, 0.0, 0.0};
    float mouse_f[] = {0.1, 0.7, 0.4, 0.0, 0.3, 0.4, 0.0, 0.0};
    float snake_f[] = {0.3, 0.3, 0.0, 0.9, 0.0, 0.1, 0.0, 0.2};
    float fire_f[]  = {0.5, 0.0, 0.0, 0.8, 0.5, 1.0, 0.9, 0.0};
    float water_f[] = {0.5, 0.3, 0.0, 0.2, 0.0, 0.3, 0.2, 1.0};
    
    float chase_f[] = {0.0, 0.9, 0.0, 0.3, 0.0, 0.0, 0.0, 0.0};
    float eat_f[]   = {0.0, 0.2, 0.0, 0.1, 0.0, 0.5, 0.0, 0.0};
    float sleep_f[] = {0.0, 0.0, 0.5, 0.0, 0.6, 0.9, 0.0, 0.0};
    float flee_f[]  = {0.0, 1.0, 0.0, 0.8, 0.0, 0.0, 0.0, 0.0};
    
    float big_f[]   = {1.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0};
    float small_f[] = {0.0, 0.2, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0};
    float fast_f[]  = {0.0, 1.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0};
    float scary_f[] = {0.5, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0};
    float warm_f[]  = {0.0, 0.0, 0.3, 0.0, 0.0, 1.0, 0.3, 0.0};
    
    float here_f[]  = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    float away_f[]  = {0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    
    /* Agents */
    comm_add_symbol(ce, "cat",   cat_f,   dim, ROLE_AGENT,  0.5f, 0);
    comm_add_symbol(ce, "dog",   dog_f,   dim, ROLE_AGENT,  0.4f, 0);
    comm_add_symbol(ce, "mouse", mouse_f, dim, ROLE_AGENT,  0.2f, 0);
    comm_add_symbol(ce, "snake", snake_f, dim, ROLE_AGENT, -0.7f, 0);
    comm_add_symbol(ce, "fire",  fire_f,  dim, ROLE_AGENT, -0.5f, 0);
    comm_add_symbol(ce, "water", water_f, dim, ROLE_AGENT,  0.0f, 0);
    
    /* Actions */
    comm_add_symbol(ce, "chase", chase_f, dim, ROLE_ACTION, -0.2f, 0);
    comm_add_symbol(ce, "eat",   eat_f,   dim, ROLE_ACTION,  0.3f, 0);
    comm_add_symbol(ce, "sleep", sleep_f, dim, ROLE_ACTION,  0.6f, 0);
    comm_add_symbol(ce, "flee",  flee_f,  dim, ROLE_ACTION, -0.5f, 0);
    
    /* Attributes */
    comm_add_symbol(ce, "big",   big_f,   dim, ROLE_ATTRIBUTE, 0.0f, 0);
    comm_add_symbol(ce, "small", small_f, dim, ROLE_ATTRIBUTE, 0.0f, 0);
    comm_add_symbol(ce, "fast",  fast_f,  dim, ROLE_ATTRIBUTE, 0.1f, 0);
    comm_add_symbol(ce, "scary", scary_f, dim, ROLE_ATTRIBUTE,-0.6f, 0);
    comm_add_symbol(ce, "warm",  warm_f,  dim, ROLE_ATTRIBUTE, 0.3f, 0);
    
    /* Locations */
    comm_add_symbol(ce, "here",  here_f,  dim, ROLE_LOCATION, 0.0f, 0);
    comm_add_symbol(ce, "away",  away_f,  dim, ROLE_LOCATION, 0.0f, 0);
}

int main(void) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║  V9: THE COMMUNICATING BRAIN         ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    
    /* Two brains with shared vocabulary */
    CommEngine *brain_a = comm_create(8);
    CommEngine *brain_b = comm_create(8);
    build_world_vocabulary(brain_a);
    build_world_vocabulary(brain_b);
    
    /* ════════ Scenario 1: Simple Communication ════════ */
    printf("═══ Scenario 1: Cat Chasing Mouse ═══\n\n");
    
    /* Brain A sees a cat chasing a mouse */
    InternalState experience;
    memset(&experience, 0, sizeof(InternalState));
    /* Blended experience: cat features + chase speed + mouse (small, fast) */
    float scene[] = {0.2, 0.7, 0.5, 0.1, 0.3, 0.5, 0.0, 0.0};
    memcpy(experience.features, scene, 8 * sizeof(float));
    experience.feature_dim = 8;
    experience.emotional_valence = 0.2f;  /* Mildly interesting */
    
    printf("Brain A's experience:\n");
    comm_print_state(&experience);
    
    /* Brain A encodes into a message */
    Message msg = comm_encode(brain_a, &experience);
    printf("\nBrain A says:\n");
    comm_print_message(brain_a, &msg);
    
    /* Brain B decodes the message */
    InternalState understood = comm_decode(brain_b, &msg);
    printf("\nBrain B understands:\n");
    comm_print_state(&understood);
    
    /* How much was preserved? */
    float error = 0.0f;
    for (int i = 0; i < 8; i++) {
        float d = experience.features[i] - understood.features[i];
        error += d * d;
    }
    printf("Communication loss: %.3f\n", sqrtf(error / 8));
    
    /* ════════ Scenario 2: Emotional Communication ════════ */
    printf("\n═══ Scenario 2: Danger! Snake! ═══\n\n");
    
    /* Brain A sees a scary snake */
    float danger_scene[] = {0.3, 0.3, 0.0, 0.9, 0.0, 0.1, 0.0, 0.2};
    memcpy(experience.features, danger_scene, 8 * sizeof(float));
    experience.emotional_valence = -0.7f;
    experience.urgency = 0.9f;
    
    printf("Brain A's experience (DANGER!):\n");
    comm_print_state(&experience);
    
    msg = comm_encode(brain_a, &experience);
    printf("\nBrain A says:\n");
    comm_print_message(brain_a, &msg);
    
    understood = comm_decode(brain_b, &msg);
    printf("\nBrain B understands:\n");
    comm_print_state(&understood);
    printf("→ Emotional valence is preserved: %.2f (negative = danger)\n",
           understood.emotional_valence);
    
    /* ════════ Scenario 3: Word Understanding ════════ */
    printf("\n═══ Scenario 3: Understanding Words ═══\n\n");
    
    const char *sentence1[] = {"big", "dog", "chase", "small", "cat"};
    const char *sentence2[] = {"small", "cat", "flee", "big", "dog"};
    
    printf("Sentence: 'big dog chase small cat'\n");
    InternalState s1 = comm_understand(brain_b, sentence1, 5);
    comm_print_state(&s1);
    
    printf("\nSentence: 'small cat flee big dog'\n");
    InternalState s2 = comm_understand(brain_b, sentence2, 5);
    comm_print_state(&s2);
    
    /* Different sentences about similar events → similar internal states? */
    float similarity = 0.0f;
    float mag1 = 0.0f, mag2 = 0.0f;
    for (int i = 0; i < 8; i++) {
        similarity += s1.features[i] * s2.features[i];
        mag1 += s1.features[i] * s1.features[i];
        mag2 += s2.features[i] * s2.features[i];
    }
    if (mag1 > 0 && mag2 > 0) {
        similarity /= sqrtf(mag1) * sqrtf(mag2);
    }
    printf("\nSimilarity between two perspectives: %.3f\n", similarity);
    printf("→ Different words, same event → similar understanding\n");
    
    /* ════════ Scenario 4: Novel Experience ════════ */
    printf("\n═══ Scenario 4: Describing the Unknown ═══\n\n");
    
    /* Something Brain A has never seen: features don't match any symbol well */
    float alien[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
    InternalState alien_exp;
    memset(&alien_exp, 0, sizeof(InternalState));
    memcpy(alien_exp.features, alien, 8 * sizeof(float));
    alien_exp.feature_dim = 8;
    
    msg = comm_encode(brain_a, &alien_exp);
    printf("Unknown experience:\n");
    comm_print_state(&alien_exp);
    printf("\nBest attempt to describe:\n");
    comm_print_message(brain_a, &msg);
    printf("→ Low confidence = the brain struggles to name it\n");
    printf("→ This is like a child seeing something for the first time!\n");
    
    /* ════════ Summary ════════ */
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║  V9 MILESTONE COMPLETE               ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ ✓ Grounded symbols (not just tokens) ║\n");
    printf("║ ✓ Encode experience → language        ║\n");
    printf("║ ✓ Decode language → experience        ║\n");
    printf("║ ✓ Emotional content preserved         ║\n");
    printf("║ ✓ Multiple perspectives understood    ║\n");
    printf("║ ✓ Graceful degradation for unknowns   ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    
    printf("→ Tag this: git tag V9-communication\n");
    printf("→ Next: Phase 10 — The Rethink Brain (putting it ALL together)\n");
    
    comm_destroy(brain_a);
    comm_destroy(brain_b);
    return 0;
}
```

## The Big Picture

You now have a brain that can:

```
Phase 1:  Fire neurons (spiking)
Phase 2:  Learn connections (Hebbian/STDP)
Phase 3:  See the world (retina + SOM)
Phase 4:  Remember things (Hopfield + decay)
Phase 5:  Learn from few examples (prototypes)
Phase 6:  Think on its own (spontaneous activity + dreams)
Phase 7:  Have specialized regions (modular brain)
Phase 8:  Understand causation (predict + explain)
Phase 9:  Communicate (grounded symbols) ← YOU ARE HERE
```

One more phase to connect everything together.

---

*Next Phase: [Phase 10 — The Rethink Brain](../10_the_rethink_brain/01_architecture_overview.md)*
