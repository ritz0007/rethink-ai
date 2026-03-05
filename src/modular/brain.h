/*
 * brain.h — Modular Brain Architecture (Signal Bus)
 * Rethink AI — Phase 7
 *
 * Note: This is the modular routing layer, separate from rethink_brain.h
 * which integrates all modules.
 */

#ifndef BRAIN_H
#define BRAIN_H

#define BRAIN_MAX_SIGNALS 64

typedef enum {
    REGION_VISUAL = 0,
    REGION_MEMORY,
    REGION_LEARNING,
    REGION_THINKING,
    REGION_EMOTION,
    REGION_EXECUTIVE,
    REGION_COUNT
} RegionType;

typedef enum {
    SIG_VISUAL_INPUT = 0,
    SIG_PATTERN_MATCH,
    SIG_NOVELTY,
    SIG_EMOTIONAL,
    SIG_MEMORY_RECALL,
    SIG_PREDICTION,
    SIG_CAUSAL,
    SIG_ATTENTION,
    SIG_LEARN,
    SIG_MOTOR
} SignalType;

typedef struct {
    SignalType type;
    RegionType source;
    RegionType target;
    float data[32];
    int data_dim;
    float priority;
    float surprise;
} Signal;

typedef struct {
    float gain[REGION_COUNT];
    float priority[REGION_COUNT];
    float surprise_threshold;
} Thalamus;

typedef struct {
    Signal bus[BRAIN_MAX_SIGNALS];
    int num_signals;
    Thalamus thalamus;
} BrainBus;

void brain_bus_init(BrainBus *bb);
int brain_bus_send(BrainBus *bb, Signal sig);
int brain_bus_receive(BrainBus *bb, RegionType target, Signal *out);
void brain_bus_clear(BrainBus *bb);
void thalamus_update_gain(Thalamus *t, RegionType region, float surprise);

#endif /* BRAIN_H */
