/*
 * meta.h — Metacognition System
 *
 * Rethink AI — Phase 15 / V15
 *
 * Brain regions modeled:
 *   - Anterior Prefrontal Cortex (aPFC/BA10): confidence monitoring
 *   - Anterior Insular Cortex (AIC): self-awareness
 *   - Dorsolateral PFC (dlPFC): strategy selection
 *   - Anterior Cingulate Cortex (ACC): error & conflict monitoring
 *   - Ventromedial PFC (vmPFC): strategy value
 *
 * Features:
 *   - Multi-source confidence monitoring (consistency, prediction, memory)
 *   - Error & conflict detection with streak tracking
 *   - Value-based strategy selection (softmax with adaptive temperature)
 *   - Meta-learning: adaptive learning rate (surprise × uncertainty)
 *   - Self-model: per-domain skill tracking
 *   - Introspection loop
 */

#ifndef META_H
#define META_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ──── Constants ──── */
#define META_MAX_STRATEGIES      8   /* Cognitive strategy slots */
#define META_HISTORY_SIZE       64   /* Performance history ring buffer */
#define META_ERROR_WINDOW       16   /* Recent errors for variance calc */
#define META_CONFIDENCE_SOURCES  3   /* consistency, prediction, memory */

/* ──── Cognitive Strategy (dlPFC) ──── */
typedef struct {
    char  name[32];          /* "memory_lookup", "causal_reason", etc. */
    float success_rate;      /* Running avg success [0-1] */
    float avg_effort;        /* Running avg cognitive cost [0-1] */
    float value;             /* Computed: success / (effort + eps) */
    int   usage_count;       /* Times this strategy was selected */
    int   active;            /* Slot in use? */
} CognitiveStrategy;

/* ──── Confidence Monitor (aPFC) ──── */
typedef struct {
    float current;           /* Current overall confidence [0-1] */
    float consistency;       /* Consistency source */
    float prediction;        /* Prediction accuracy source */
    float memory;            /* Memory strength source */
    float weights[META_CONFIDENCE_SOURCES]; /* Source weights */
    float calibration_error; /* Expected Calibration Error */
    float adjustment;        /* Systematic over/under-confidence correction */
} ConfidenceMonitor;

/* ──── Error Monitor (ACC) ──── */
typedef struct {
    float recent_errors[META_ERROR_WINDOW]; /* Ring buffer */
    int   error_idx;
    int   error_count;
    float error_baseline;    /* Running average of |error| */
    float conflict_level;    /* Strategy disagreement [0-1] */
    float cognitive_load;    /* Current effort level [0-1] */
    int   error_streak;      /* Consecutive above-threshold errors */
} ErrorMonitor;

/* ──── Self-Model (aPFC + AIC) ──── */
typedef struct {
    float perception_accuracy;  /* V3: How well do I see/hear/feel? */
    float memory_reliability;   /* V4: How reliable is my recall? */
    float reasoning_skill;      /* V8: How good at prediction/causal? */
    float learning_speed;       /* V2/V5: How fast do I learn? */
    float social_skill;         /* V14: How well do I model others? */
    float motor_skill;          /* V13: How good are my actions? */
    float overall_capability;   /* Aggregate self-assessment */
    float cognitive_stamina;    /* How long can I maintain focus? */
    float meta_accuracy;        /* How well do I know my own limits? */
} SelfModel;

/* ──── Performance Record ──── */
typedef struct {
    float predicted_confidence;  /* How confident was I *before*? */
    float actual_outcome;        /* Was I right? [0=wrong, 1=right] */
    int   strategy_used;         /* Which strategy? */
    float effort;                /* How much effort? */
} PerformanceRecord;

/* ──── The Metacognition System ──── */
typedef struct {
    /* Confidence monitoring (aPFC) */
    ConfidenceMonitor confidence;

    /* Error & conflict monitoring (ACC) */
    ErrorMonitor errors;

    /* Strategy selection (dlPFC) */
    CognitiveStrategy strategies[META_MAX_STRATEGIES];
    int num_strategies;
    int current_strategy;          /* Currently active strategy index */
    float temperature;             /* Softmax temperature for selection */

    /* Meta-learning */
    float base_learning_rate;      /* alpha_base */
    float effective_learning_rate; /* alpha_effective (adapted) */
    float learning_trajectory;     /* Performance trend */

    /* Self-model (aPFC + AIC) */
    SelfModel self;

    /* Performance history (ring buffer) */
    PerformanceRecord history[META_HISTORY_SIZE];
    int history_count;
    int history_idx;

    /* State */
    int   introspection_count;     /* Times metacognition was invoked */
    int   strategy_switches;       /* Times strategy was changed */
    float curiosity;               /* Drive to explore vs. exploit */
} MetaSystem;

/* ──── Lifecycle ──── */
void meta_init(MetaSystem *ms);

/* ──── Confidence (aPFC) ──── */
float meta_confidence(const MetaSystem *ms);
void  meta_update_confidence(MetaSystem *ms, float consistency,
                             float prediction_acc, float memory_strength);
void  meta_calibrate(MetaSystem *ms, float predicted_conf, float actual_outcome);

/* ──── Error Monitoring (ACC) ──── */
void  meta_record_error(MetaSystem *ms, float error_magnitude);
int   meta_error_detected(const MetaSystem *ms);
float meta_cognitive_load(const MetaSystem *ms);

/* ──── Strategy Selection (dlPFC) ──── */
int   meta_add_strategy(MetaSystem *ms, const char *name);
int   meta_select_strategy(MetaSystem *ms);
void  meta_update_strategy(MetaSystem *ms, int strategy_id,
                           float outcome, float effort);
const char *meta_current_strategy_name(const MetaSystem *ms);

/* ──── Meta-Learning ──── */
float meta_effective_lr(const MetaSystem *ms);
void  meta_adapt_learning(MetaSystem *ms, float surprise, float outcome);

/* ──── Self-Model ──── */
void  meta_update_self(MetaSystem *ms, const char *domain, float performance);
float meta_self_assessment(const MetaSystem *ms, const char *domain);
float meta_meta_accuracy(const MetaSystem *ms);

/* ──── Performance Recording ──── */
void  meta_record_performance(MetaSystem *ms, float confidence,
                              float outcome, float effort);

/* ──── Introspection ──── */
void  meta_introspect(MetaSystem *ms);

/* ──── Query / Debug ──── */
void  meta_print_status(const MetaSystem *ms);
void  meta_print_strategies(const MetaSystem *ms);
void  meta_print_self(const MetaSystem *ms);

#endif /* META_H */
