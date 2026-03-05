/*
 * emotion.c — Amygdala Implementation
 * Rethink AI — Phase 7
 */

#include "emotion.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

Amygdala *amygdala_create(void) {
    Amygdala *a = (Amygdala *)calloc(1, sizeof(Amygdala));
    a->fear_threshold = 0.6f;
    a->reward_threshold = 0.6f;
    return a;
}

void amygdala_destroy(Amygdala *a) { free(a); }

static float pattern_sim(const float *a, const float *b, int n) {
    float dot = 0, ma = 0, mb = 0;
    for (int i = 0; i < n; i++) {
        dot += a[i] * b[i]; ma += a[i]*a[i]; mb += b[i]*b[i];
    }
    if (ma < 1e-8f || mb < 1e-8f) return 0.0f;
    return dot / (sqrtf(ma) * sqrtf(mb));
}

void amygdala_learn(Amygdala *a, const float *pattern, int dim, EmotionType emotion, float intensity) {
    if (a->count >= EMO_MAX_MEMORIES) return;
    int d = dim < EMO_MAX_DIM ? dim : EMO_MAX_DIM;
    EmotionalMemory *m = &a->memories[a->count++];
    memcpy(m->pattern, pattern, d * sizeof(float));
    m->dim = d;
    m->emotion = emotion;
    m->intensity = intensity;
}

EmotionalResponse amygdala_evaluate(const Amygdala *a, const float *pattern, int dim) {
    EmotionalResponse resp;
    resp.emotion = EMOTION_NEUTRAL;
    resp.valence = 0.0f;
    resp.arousal = 0.0f;
    resp.importance = 0.5f;
    resp.attention = 1.0f;
    resp.learning = 1.0f;
    
    float best_sim = 0.0f;
    int best_idx = -1;
    
    for (int i = 0; i < a->count; i++) {
        int d = dim < a->memories[i].dim ? dim : a->memories[i].dim;
        float sim = pattern_sim(pattern, a->memories[i].pattern, d);
        if (sim > best_sim) {
            best_sim = sim;
            best_idx = i;
        }
    }
    
    if (best_idx >= 0 && best_sim > 0.4f) {
        EmotionalMemory *m = &a->memories[best_idx];
        resp.emotion = m->emotion;
        resp.arousal = best_sim * m->intensity;
        
        switch (m->emotion) {
            case EMOTION_FEAR:
                resp.valence = -0.8f;
                resp.importance = 0.9f;
                resp.attention = 2.0f;
                resp.learning = 1.5f;
                break;
            case EMOTION_REWARD:
                resp.valence = 0.7f;
                resp.importance = 0.7f;
                resp.attention = 1.3f;
                resp.learning = 1.3f;
                break;
            case EMOTION_FAMILIAR:
                resp.valence = 0.2f;
                resp.importance = 0.3f;
                resp.attention = 0.8f;
                resp.learning = 0.8f;
                break;
            case EMOTION_DISGUST:
                resp.valence = -0.5f;
                resp.importance = 0.6f;
                resp.attention = 1.2f;
                resp.learning = 1.1f;
                break;
            default:
                break;
        }
    }
    
    return resp;
}
