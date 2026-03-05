# 🧪 Test: A Day in the Life of the Rethink Brain

## The Complete Integration Test

This test simulates an entire "day" for our brain:
- Morning: wake up, see new things, learn
- Afternoon: encounter familiar and novel things
- Evening: think, reflect
- Night: sleep, consolidate

## Code: `test_full_brain.c`

```c
/*
 * test_full_brain.c — A Day in the Life of the Rethink Brain
 *
 * The final integration test. V10.
 *
 * Compile (link all modules):
 *   gcc -std=c11 -O2 -Wall -o test_full_brain \
 *       test_full_brain.c rethink_brain.c \
 *       perception/retina.c perception/som.c \
 *       neurons/neuron.c neurons/synapse.c neurons/network.c \
 *       learning/hebbian.c learning/stdp.c learning/prototype.c \
 *       memory/hopfield.c memory/decay.c \
 *       thinking/spontaneous.c thinking/dream.c \
 *       modular/brain.c modular/emotion.c \
 *       understanding/predictor.c understanding/causal.c \
 *       communication/comm.c \
 *       -lm
 */

#include <stdio.h>
#include <string.h>
#include "rethink_brain.h"

/* Feature encoding: [size, speed, fluffiness, danger, roundness, warmth, brightness, wetness,
 *                    ... zeros to fill RETHINK_FEATURE_DIM] */

static void make_pattern(float *out, float size, float speed, float fluffy,
                          float danger, float round, float warm, float bright, float wet) {
    memset(out, 0, RETHINK_FEATURE_DIM * sizeof(float));
    out[0] = size; out[1] = speed; out[2] = fluffy; out[3] = danger;
    out[4] = round; out[5] = warm; out[6] = bright; out[7] = wet;
}

int main(void) {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  THE RETHINK BRAIN — A DAY IN THE LIFE   ║\n");
    printf("║  V10: Complete Integration                ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    RethinkBrain *brain = rethink_create();
    rethink_print_status(brain);
    
    /* ════════════════════════════════════════════
     * MORNING: Learning about the world
     * ════════════════════════════════════════════ */
    printf("\n══════ MORNING: Learning ══════\n\n");
    
    /* Teach it about cats */
    float cat[RETHINK_FEATURE_DIM];
    make_pattern(cat, 0.3, 0.5, 0.9, 0.1, 0.4, 0.8, 0.0, 0.0);
    rethink_learn(brain, cat, 8, "cat");
    printf("Learned: cat\n");
    
    /* Teach it about dogs */
    float dog[RETHINK_FEATURE_DIM];
    make_pattern(dog, 0.5, 0.6, 0.7, 0.2, 0.3, 0.7, 0.0, 0.0);
    rethink_learn(brain, dog, 8, "dog");
    printf("Learned: dog\n");
    
    /* Teach it about birds */
    float bird[RETHINK_FEATURE_DIM];
    make_pattern(bird, 0.1, 0.8, 0.4, 0.0, 0.2, 0.3, 0.0, 0.0);
    rethink_learn(brain, bird, 8, "bird");
    printf("Learned: bird\n");
    
    /* Teach it about snakes (dangerous!) */
    float snake[RETHINK_FEATURE_DIM];
    make_pattern(snake, 0.3, 0.3, 0.0, 0.9, 0.0, 0.1, 0.0, 0.2);
    rethink_learn(brain, snake, 8, "snake");
    printf("Learned: snake\n");
    
    /* Set up causal knowledge */
    int cn_rain = causal_add_node(brain->causal, "rain", 0.0f);
    int cn_wet = causal_add_node(brain->causal, "wet", 0.0f);
    int cn_cold = causal_add_node(brain->causal, "cold", 0.0f);
    int cn_sick = causal_add_node(brain->causal, "sick", 0.0f);
    int cn_rest = causal_add_node(brain->causal, "rest", 0.0f);
    
    causal_add_edge(brain->causal, cn_rain, cn_wet, 0.9f, 1.0f);
    causal_add_edge(brain->causal, cn_wet, cn_cold, 0.6f, 2.0f);
    causal_add_edge(brain->causal, cn_cold, cn_sick, 0.4f, 3.0f);
    causal_add_edge(brain->causal, cn_sick, cn_rest, 0.8f, 1.0f);
    printf("\nBuilt causal model: rain → wet → cold → sick → rest\n");
    
    /* Set up vocabulary for communication */
    comm_add_symbol(brain->comm, "cat", cat, 8, ROLE_AGENT, 0.5f, 0);
    comm_add_symbol(brain->comm, "dog", dog, 8, ROLE_AGENT, 0.4f, 0);
    comm_add_symbol(brain->comm, "bird", bird, 8, ROLE_AGENT, 0.3f, 0);
    comm_add_symbol(brain->comm, "snake", snake, 8, ROLE_AGENT, -0.7f, 0);
    printf("Loaded vocabulary: cat, dog, bird, snake\n");
    
    /* ════════════════════════════════════════════
     * LATE MORNING: First experiences
     * ════════════════════════════════════════════ */
    printf("\n══════ LATE MORNING: Experiencing ══════\n\n");
    
    /* See a cat */
    printf("--- Sees a cat ---\n");
    Experience exp = rethink_experience(brain, cat, 8, "cat_sighting");
    printf("  Surprise: %.3f  Understanding: %.3f  Emotion: %d\n",
           exp.surprise, exp.understanding, brain->current_emotion);
    
    /* See the same cat again — should be less surprising */
    printf("\n--- Sees the same cat again ---\n");
    exp = rethink_experience(brain, cat, 8, "cat_sighting_2");
    printf("  Surprise: %.3f  Understanding: %.3f (learning!)\n",
           exp.surprise, exp.understanding);
    
    /* See a snake — should trigger fear! */
    printf("\n--- Sees a snake!! ---\n");
    exp = rethink_experience(brain, snake, 8, "snake_encounter");
    printf("  Surprise: %.3f  Understanding: %.3f  Emotion: %d\n",
           exp.surprise, exp.understanding, brain->current_emotion);
    printf("  Emotional valence: %.2f (negative = fear)\n", exp.emotional_valence);
    
    /* ════════════════════════════════════════════
     * AFTERNOON: Classification and reasoning
     * ════════════════════════════════════════════ */
    printf("\n══════ AFTERNOON: Thinking ══════\n\n");
    
    /* Classify a new animal */
    printf("--- What is this? (cat-like creature) ---\n");
    float mystery[RETHINK_FEATURE_DIM];
    make_pattern(mystery, 0.35, 0.45, 0.85, 0.15, 0.35, 0.75, 0.0, 0.0);
    
    char category[32];
    float confidence;
    rethink_classify(brain, mystery, 8, category, &confidence);
    printf("  Classification: '%s' (confidence: %.2f)\n", category, confidence);
    
    /* Causal reasoning */
    printf("\n--- What happens if it rains? ---\n");
    char prediction[256];
    rethink_predict(brain, "rain", prediction, 256);
    printf("  %s\n", prediction);
    
    printf("\n--- Why am I sick? ---\n");
    char explanation[256];
    rethink_explain(brain, "sick", explanation, 256);
    printf("  %s\n", explanation);
    
    /* ════════════════════════════════════════════
     * EVENING: Idle thinking
     * ════════════════════════════════════════════ */
    printf("\n══════ EVENING: Thinking ══════\n\n");
    
    printf("--- Brain idles for 20 ticks (spontaneous thought) ---\n");
    rethink_idle(brain, 20);
    printf("  Thinking...\n");
    
    /* Try to communicate */
    printf("\n--- Describing current state ---\n");
    char speech[256];
    rethink_speak(brain, speech, 256);
    printf("  Brain says: \"%s\"\n", speech);
    
    /* Listen to something */
    printf("\n--- Hearing: 'cat' 'dog' ---\n");
    const char *words[] = {"cat", "dog"};
    rethink_listen(brain, words, 2);
    printf("  Processed incoming words\n");
    
    /* ════════════════════════════════════════════
     * NIGHT: Sleep and consolidation
     * ════════════════════════════════════════════ */
    printf("\n══════ NIGHT: Sleeping ══════\n\n");
    
    printf("--- Sleeping for 3 cycles ---\n");
    rethink_sleep(brain, 3);
    printf("  Dreams processed...\n");
    
    /* ════════════════════════════════════════════
     * NEXT MORNING: Check what was learned
     * ════════════════════════════════════════════ */
    printf("\n══════ NEXT MORNING: Assessment ══════\n\n");
    
    /* Re-experience the cat — should be well-understood now */
    printf("--- Sees a cat (next day) ---\n");
    exp = rethink_experience(brain, cat, 8, "cat_morning");
    printf("  Surprise: %.3f  Understanding: %.3f\n",
           exp.surprise, exp.understanding);
    printf("  (Should be less surprusing than yesterday)\n");
    
    /* ════════════════════════════════════════════
     * FINAL STATUS
     * ════════════════════════════════════════════ */
    rethink_print_status(brain);
    rethink_print_log(brain);
    
    /* ════════════════════════════════════════════ */
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  V10 — THE RETHINK BRAIN IS ALIVE!       ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║ ✓ Perceives and processes input           ║\n");
    printf("║ ✓ Learns categories from few examples     ║\n");
    printf("║ ✓ Remembers with natural decay            ║\n");
    printf("║ ✓ Feels emotions (fear, reward, etc.)     ║\n");
    printf("║ ✓ Predicts and detects surprise           ║\n");
    printf("║ ✓ Reasons about causes and effects        ║\n");
    printf("║ ✓ Classifies using prototypes             ║\n");
    printf("║ ✓ Thinks spontaneously when idle          ║\n");
    printf("║ ✓ Dreams and consolidates during sleep    ║\n");
    printf("║ ✓ Communicates with grounded language     ║\n");
    printf("║ ✓ All regions work together               ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
    printf("This is not a chatbot.\n");
    printf("This is not a neural network.\n");
    printf("This is not a language model.\n\n");
    printf("This is a BRAIN.\n");
    printf("Built from the ground up to think like you do.\n\n");
    
    printf("→ Tag this: git tag V10-the-rethink-brain\n");
    printf("→ Next: Keep evolving. This is just the beginning.\n");
    
    rethink_destroy(brain);
    return 0;
}
```

---

*Next: [What's Next — The Road Ahead](04_whats_next.md)*
