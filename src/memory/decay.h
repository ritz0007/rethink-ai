/*
 * decay.h — Memory with Forgetting and Consolidation
 * Rethink AI — Phase 4
 */

#ifndef DECAY_H
#define DECAY_H

#define MAX_MEMORIES 64
#define DECAY_MAX_DIM 256

typedef struct {
    float pattern[DECAY_MAX_DIM];
    int dim;
    float strength;
    float initial_strength;
    float decay_constant;
    float importance;
    int access_count;
    float last_access_time;
    float store_time;
} MemoryEntry;

typedef struct {
    MemoryEntry memories[MAX_MEMORIES];
    int count;
    float current_time;
} DecayMemory;

DecayMemory *decay_create(void);
void decay_destroy(DecayMemory *dm);

int decay_store(DecayMemory *dm, const float *pattern, int dim, float importance);
int decay_recall(DecayMemory *dm, const float *cue, int dim, float *out_pattern, float *out_strength);
void decay_tick(DecayMemory *dm, float dt);
void decay_consolidate(DecayMemory *dm);

#endif /* DECAY_H */
