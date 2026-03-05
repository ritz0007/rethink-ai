/*
 * emotion.h — Emotional Processing (Amygdala)
 * Rethink AI — Phase 7
 */

#ifndef EMOTION_H
#define EMOTION_H

#define EMO_MAX_MEMORIES 32
#define EMO_MAX_DIM 128

typedef enum {
    EMOTION_NEUTRAL = 0,
    EMOTION_FEAR,
    EMOTION_REWARD,
    EMOTION_SURPRISE,
    EMOTION_FAMILIAR,
    EMOTION_DISGUST
} EmotionType;

typedef struct {
    float pattern[EMO_MAX_DIM];
    int dim;
    EmotionType emotion;
    float intensity;
} EmotionalMemory;

typedef struct {
    EmotionType emotion;
    float valence;    /* -1 to +1 */
    float arousal;    /* 0 to 1 */
    float importance; /* How much to boost memory storage */
    float attention;  /* How much to boost attention */
    float learning;   /* How much to boost learning rate */
} EmotionalResponse;

typedef struct {
    EmotionalMemory memories[EMO_MAX_MEMORIES];
    int count;
    float fear_threshold;
    float reward_threshold;
} Amygdala;

Amygdala *amygdala_create(void);
void amygdala_destroy(Amygdala *a);

void amygdala_learn(Amygdala *a, const float *pattern, int dim, EmotionType emotion, float intensity);
EmotionalResponse amygdala_evaluate(const Amygdala *a, const float *pattern, int dim);

#endif /* EMOTION_H */
