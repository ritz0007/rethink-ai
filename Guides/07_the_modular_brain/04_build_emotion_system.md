# 🔨 Build: The Emotion System — Why Feelings Are Computational

## Emotions Aren't a Bug — They're a Priority System

The amygdala doesn't make you "feel." It **tags information with importance**:

```
See a snake → Amygdala: DANGER (importance=0.99) → immediate action
See a chair → Amygdala: meh (importance=0.01) → ignore
See a friend → Amygdala: POSITIVE (importance=0.7) → engage
```

Without emotions, you'd treat everything equally. You'd spend as much time evaluating the pattern on the carpet as evaluating a speeding car coming at you.

**Emotions = computational priority.**

## Code: `src/brain/emotion.h`

```c
/*
 * emotion.h — The Amygdala: Emotional Evaluation and Priority
 *
 * Rethink AI — Phase 7
 *
 * Tags inputs with emotional significance:
 *   - Danger detection (fast, crude)
 *   - Reward prediction  
 *   - Novelty detection
 *   - Familiarity comfort
 *
 * Affects:
 *   - Memory importance (emotional memories last longer)
 *   - Attention priority (emotional things grab attention)
 *   - Learning rate (emotional events learn faster)
 */

#ifndef EMOTION_H
#define EMOTION_H

#define EMOTION_MEMORY_SIZE 32

typedef enum {
    EMO_NEUTRAL,
    EMO_FEAR,       /* Danger → avoid */
    EMO_REWARD,     /* Positive → approach */
    EMO_SURPRISE,   /* Unexpected → pay attention */
    EMO_FAMILIAR,   /* Known pattern → comfort */
    EMO_DISGUST     /* Bad pattern → reject */
} EmotionType;

static const char *EMOTION_NAMES[] = {
    "neutral", "fear", "reward", "surprise", "familiar", "disgust"
};

/* An emotional memory: links a pattern to an emotional response */
typedef struct {
    float pattern[128];
    int pattern_size;
    EmotionType emotion;
    float intensity;     /* 0 to 1 */
    int activations;     /* How many times triggered */
} EmotionalMemory;

typedef struct {
    EmotionalMemory memories[EMOTION_MEMORY_SIZE];
    int num_memories;
    
    /* Current emotional state */
    float valence;       /* -1 (negative) to +1 (positive) */
    float arousal;       /* 0 (calm) to 1 (excited) */
    EmotionType current_emotion;
    
    /* Parameters */
    float fear_threshold;      /* Similarity to danger patterns */
    float reward_threshold;    /* Similarity to reward patterns */
    float surprise_threshold;  /* Novelty level to trigger surprise */
    float decay_rate;          /* How fast emotions return to neutral */
} Amygdala;

/* Create the amygdala */
Amygdala *amygdala_create(void);
void amygdala_destroy(Amygdala *amy);

/* Associate a pattern with an emotion (learning) */
void amygdala_associate(Amygdala *amy, const float *pattern, int size,
                        EmotionType emotion, float intensity);

/* Evaluate a new input — returns emotional significance */
float amygdala_evaluate(Amygdala *amy, const float *input, int size,
                        EmotionType *emotion_out);

/* Fast danger detection (subcortical shortcut) */
int amygdala_danger_check(const Amygdala *amy, const float *input, int size);

/* Update emotional state (call each tick) */
void amygdala_tick(Amygdala *amy, float dt);

/* Get current emotional influence on other systems */
float amygdala_get_importance_modifier(const Amygdala *amy);
float amygdala_get_attention_urgency(const Amygdala *amy);
float amygdala_get_learning_rate_modifier(const Amygdala *amy);

/* Print status */
void amygdala_print_state(const Amygdala *amy);

#endif /* EMOTION_H */
```

## Code: `src/brain/emotion.c`

```c
#include "emotion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

Amygdala *amygdala_create(void) {
    Amygdala *amy = (Amygdala *)calloc(1, sizeof(Amygdala));
    amy->valence = 0.0f;
    amy->arousal = 0.0f;
    amy->current_emotion = EMO_NEUTRAL;
    amy->fear_threshold = 0.7f;
    amy->reward_threshold = 0.7f;
    amy->surprise_threshold = 0.6f;
    amy->decay_rate = 0.05f;
    return amy;
}

void amygdala_destroy(Amygdala *amy) {
    free(amy);
}

static float pattern_similarity(const float *a, const float *b, int size) {
    float dot = 0, na = 0, nb = 0;
    for (int i = 0; i < size; i++) {
        dot += a[i] * b[i];
        na += a[i] * a[i];
        nb += b[i] * b[i];
    }
    if (na < 1e-8f || nb < 1e-8f) return 0.0f;
    return dot / (sqrtf(na) * sqrtf(nb));
}

void amygdala_associate(Amygdala *amy, const float *pattern, int size,
                        EmotionType emotion, float intensity) {
    if (amy->num_memories >= EMOTION_MEMORY_SIZE) return;
    
    EmotionalMemory *m = &amy->memories[amy->num_memories++];
    int copy_size = (size < 128) ? size : 128;
    memcpy(m->pattern, pattern, copy_size * sizeof(float));
    m->pattern_size = copy_size;
    m->emotion = emotion;
    m->intensity = intensity;
    m->activations = 0;
}

float amygdala_evaluate(Amygdala *amy, const float *input, int size,
                        EmotionType *emotion_out) {
    float best_match = 0.0f;
    EmotionType best_emotion = EMO_NEUTRAL;
    float best_intensity = 0.0f;
    
    /* Check against all emotional memories */
    for (int i = 0; i < amy->num_memories; i++) {
        EmotionalMemory *m = &amy->memories[i];
        int cmp_size = (size < m->pattern_size) ? size : m->pattern_size;
        float sim = pattern_similarity(input, m->pattern, cmp_size);
        
        float score = sim * m->intensity;
        if (score > best_match) {
            best_match = score;
            best_emotion = m->emotion;
            best_intensity = m->intensity;
            m->activations++;
        }
    }
    
    /* Check for novelty (no match = surprise) */
    if (best_match < amy->surprise_threshold) {
        /* Nothing familiar → surprise! */
        best_emotion = EMO_SURPRISE;
        best_intensity = 1.0f - best_match;  /* More novel = more surprising */
    }
    
    /* Update emotional state */
    switch (best_emotion) {
        case EMO_FEAR:
        case EMO_DISGUST:
            amy->valence = fmaxf(-1.0f, amy->valence - best_intensity * 0.5f);
            amy->arousal = fminf(1.0f, amy->arousal + best_intensity * 0.8f);
            break;
        case EMO_REWARD:
        case EMO_FAMILIAR:
            amy->valence = fminf(1.0f, amy->valence + best_intensity * 0.4f);
            amy->arousal = fminf(1.0f, amy->arousal + best_intensity * 0.3f);
            break;
        case EMO_SURPRISE:
            amy->arousal = fminf(1.0f, amy->arousal + best_intensity * 0.6f);
            break;
        default:
            break;
    }
    amy->current_emotion = best_emotion;
    
    if (emotion_out) *emotion_out = best_emotion;
    return best_intensity;
}

int amygdala_danger_check(const Amygdala *amy, const float *input, int size) {
    /* Fast, crude check — amygdala responds before cortex finishes processing */
    for (int i = 0; i < amy->num_memories; i++) {
        if (amy->memories[i].emotion != EMO_FEAR) continue;
        int cmp_size = (size < amy->memories[i].pattern_size) ? 
                        size : amy->memories[i].pattern_size;
        float sim = pattern_similarity(input, amy->memories[i].pattern, cmp_size);
        if (sim > amy->fear_threshold) return 1;  /* DANGER! */
    }
    return 0;  /* Safe */
}

void amygdala_tick(Amygdala *amy, float dt) {
    /* Emotions decay toward neutral */
    amy->valence *= (1.0f - amy->decay_rate * dt);
    amy->arousal *= (1.0f - amy->decay_rate * dt);
    
    if (fabsf(amy->valence) < 0.01f && amy->arousal < 0.01f) {
        amy->current_emotion = EMO_NEUTRAL;
    }
}

float amygdala_get_importance_modifier(const Amygdala *amy) {
    /* High arousal = more important = remember better */
    return 0.3f + amy->arousal * 0.7f;
}

float amygdala_get_attention_urgency(const Amygdala *amy) {
    /* Fear and surprise demand immediate attention */
    if (amy->current_emotion == EMO_FEAR) return 0.95f;
    if (amy->current_emotion == EMO_SURPRISE) return 0.8f;
    return 0.3f + amy->arousal * 0.4f;
}

float amygdala_get_learning_rate_modifier(const Amygdala *amy) {
    /* Emotional events are learned faster */
    /* One-trial learning for fear (touch a hot stove once, never again) */
    if (amy->current_emotion == EMO_FEAR) return 3.0f;
    if (amy->current_emotion == EMO_REWARD) return 2.0f;
    return 1.0f + amy->arousal;
}

void amygdala_print_state(const Amygdala *amy) {
    printf("\n  Emotion: %s\n", EMOTION_NAMES[amy->current_emotion]);
    printf("  Valence: ");
    if (amy->valence < -0.3f) printf("😟 ");
    else if (amy->valence > 0.3f) printf("😊 ");
    else printf("😐 ");
    printf("%.2f  ", amy->valence);
    
    printf("Arousal: ");
    int bar = (int)(amy->arousal * 10);
    for (int i = 0; i < bar; i++) printf("!");
    for (int i = bar; i < 10; i++) printf(".");
    printf(" %.2f\n", amy->arousal);
    
    printf("  → Memory importance: ×%.1f\n", amygdala_get_importance_modifier(amy));
    printf("  → Attention urgency: %.2f\n", amygdala_get_attention_urgency(amy));
    printf("  → Learning rate:     ×%.1f\n", amygdala_get_learning_rate_modifier(amy));
}
```

## Why This Matters

### Without Emotions

```
See snake: process → classify → "reptile" → done
See flower: process → classify → "plant" → done
(Both treated identically)
```

### With Emotions

```
See snake: amygdala fires FIRST → DANGER → 
  attention spikes → full cortex engaged → remember this FOREVER
  
See flower: amygdala: "meh" →
  normal processing → classify → mild memory → may forget
```

Emotions create **asymmetric processing**:
- Dangerous things → fast response, deep memory, high attention
- Rewarding things → approach, strong memory, positive association
- Novel things → surprise, pay attention, learn quickly
- Familiar things → comfort, low processing, save energy

This is **computationally optimal**. You can't process everything deeply, so emotions help you prioritize.

---

*Next: [Milestone — The Modular Brain](05_milestone_modular_brain.md)*
