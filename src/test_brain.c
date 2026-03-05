/*
 * test_brain.c — A Day in the Life of the Rethink Brain
 *
 * Rethink AI — Phase 10 Test
 *
 * Compile:   cd src && make test
 * Run:       ./test_brain
 */

#include "rethink_brain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Helper: create a simple feature pattern */
static void make_pattern(float *out, int dim, float base, float variation) {
    for (int i = 0; i < dim; i++) {
        out[i] = base + variation * ((float)(i % 7) / 7.0f - 0.5f);
    }
}

int main(void) {
    srand((unsigned)time(NULL));
    
    printf("=== RETHINK BRAIN — A Day in the Life ===\n\n");
    
    /* 1. Create the brain */
    printf("--- Creating brain ---\n");
    RethinkBrain *brain = rethink_create();
    rethink_print_status(brain);
    
    /* 2. Teach it some categories */
    printf("\n--- Learning phase ---\n");
    float cat_pattern[RETHINK_FEATURE_DIM];
    float dog_pattern[RETHINK_FEATURE_DIM];
    float bird_pattern[RETHINK_FEATURE_DIM];
    
    make_pattern(cat_pattern, RETHINK_FEATURE_DIM, 0.8f, 0.3f);
    make_pattern(dog_pattern, RETHINK_FEATURE_DIM, 0.2f, 0.5f);
    make_pattern(bird_pattern, RETHINK_FEATURE_DIM, 0.5f, 0.8f);
    
    rethink_learn(brain, cat_pattern, RETHINK_FEATURE_DIM, "cat");
    rethink_learn(brain, dog_pattern, RETHINK_FEATURE_DIM, "dog");
    rethink_learn(brain, bird_pattern, RETHINK_FEATURE_DIM, "bird");
    printf("Learned 3 categories: cat, dog, bird\n");
    
    /* 3. Experience things */
    printf("\n--- Experience phase ---\n");
    
    float input1[RETHINK_FEATURE_DIM];
    make_pattern(input1, RETHINK_FEATURE_DIM, 0.78f, 0.32f);  /* cat-like */
    Experience e1 = rethink_experience(brain, input1, RETHINK_FEATURE_DIM, "fluffy_thing");
    printf("Saw 'fluffy_thing': surprise=%.3f, understanding=%.3f, valence=%.2f\n",
           e1.surprise, e1.understanding, e1.emotional_valence);
    
    float input2[RETHINK_FEATURE_DIM];
    make_pattern(input2, RETHINK_FEATURE_DIM, 0.22f, 0.48f);  /* dog-like */
    Experience e2 = rethink_experience(brain, input2, RETHINK_FEATURE_DIM, "barking_thing");
    printf("Saw 'barking_thing': surprise=%.3f, understanding=%.3f, valence=%.2f\n",
           e2.surprise, e2.understanding, e2.emotional_valence);
    
    /* 4. Classify */
    printf("\n--- Classification ---\n");
    char category[32];
    float confidence;
    rethink_classify(brain, input1, RETHINK_FEATURE_DIM, category, &confidence);
    printf("fluffy_thing -> '%s' (confidence=%.2f)\n", category, confidence);
    
    rethink_classify(brain, input2, RETHINK_FEATURE_DIM, category, &confidence);
    printf("barking_thing -> '%s' (confidence=%.2f)\n", category, confidence);
    
    /* 5. Build causal knowledge */
    printf("\n--- Causal reasoning ---\n");
    int rain = causal_add_node(brain->causal, "rain", 0.3f);
    int wet = causal_add_node(brain->causal, "wet_ground", 0.0f);
    int slip = causal_add_node(brain->causal, "slipping", 0.0f);
    int umbrella = causal_add_node(brain->causal, "umbrella", 0.0f);
    causal_add_edge(brain->causal, rain, wet, 0.9f, 1.0f);
    causal_add_edge(brain->causal, wet, slip, 0.6f, 1.0f);
    causal_add_edge(brain->causal, rain, umbrella, 0.7f, 1.0f);
    
    char explanation[256];
    rethink_explain(brain, "slipping", explanation, 256);
    printf("Why slipping? %s\n", explanation);
    
    char prediction[256];
    rethink_predict(brain, "rain", prediction, 256);
    printf("What if rain? %s\n", prediction);
    
    /* 6. Add vocabulary and communicate */
    printf("\n--- Communication ---\n");
    comm_add_symbol(brain->comm, "cat", cat_pattern, RETHINK_FEATURE_DIM,
                    ROLE_PATIENT, 0.3f, 0);
    comm_add_symbol(brain->comm, "dog", dog_pattern, RETHINK_FEATURE_DIM,
                    ROLE_PATIENT, 0.2f, 0);
    comm_add_symbol(brain->comm, "see", NULL, 0, ROLE_ACTION, 0.0f, 1);
    comm_add_symbol(brain->comm, "I", NULL, 0, ROLE_AGENT, 0.0f, 1);
    
    char speech[128] = {0};
    /* Set current features to cat-like to trigger cat word */
    memcpy(brain->current_features, cat_pattern, RETHINK_FEATURE_DIM * sizeof(float));
    rethink_speak(brain, speech, 128);
    printf("Brain says: \"%s\"\n", speech);
    
    /* 7. Idle thinking */
    printf("\n--- Idle thinking ---\n");
    rethink_idle(brain, 50);
    printf("Brain idled for 50 ticks\n");
    
    /* 8. Sleep */
    printf("\n--- Sleep cycle ---\n");
    rethink_sleep(brain, 10);
    printf("Slept for 10 cycles\n");
    
    /* 9. Final status */
    rethink_print_status(brain);
    rethink_print_log(brain);
    
    /* Cleanup */
    rethink_destroy(brain);
    printf("\n=== Brain destroyed. Session complete. ===\n");
    
    return 0;
}
