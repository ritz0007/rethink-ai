/*
 * dream.h — Dream Engine (Memory Consolidation)
 * Rethink AI — Phase 6
 */

#ifndef DREAM_H
#define DREAM_H

#include "../memory/decay.h"

#define DREAM_MAX_DIM 256

typedef enum {
    SLEEP_AWAKE = 0,
    SLEEP_NREM,
    SLEEP_REM
} SleepStage;

typedef struct {
    float pattern[DREAM_MAX_DIM];
    int dim;
    float novelty;
    float stability;
} Insight;

typedef struct {
    DecayMemory *memory;    /* Shared memory reference */
    SleepStage stage;
    float replay_fidelity;  /* How faithfully memories replay */
    float recombine_rate;   /* How much REM recombines */
    Insight last_insight;
    int has_insight;
} DreamEngine;

DreamEngine *dream_create(DecayMemory *memory);
void dream_destroy(DreamEngine *de);

void dream_nrem_replay(DreamEngine *de);
void dream_rem_recombine(DreamEngine *de);
int dream_check_insight(DreamEngine *de, Insight *out);

#endif /* DREAM_H */
