/*
 * rethink_brain.h — The Complete Rethink Brain
 *
 * Rethink AI — Phase 10 / V10
 *
 * Connects every module:
 *   - Retina (perception)
 *   - SOM (spatial mapping)
 *   - Spiking Network (neural substrate)
 *   - Hebbian/STDP (learning)
 *   - Hopfield + Decay Memory (storage)
 *   - Prototype Learner (categorization)
 *   - Spontaneous Activity + Dream Engine (thinking)
 *   - Brain Architecture + Emotion (modularity)
 *   - Predictive Coding + Causal Net (understanding)
 *   - Communication Engine (language)
 */

#ifndef RETHINK_BRAIN_H
#define RETHINK_BRAIN_H

/* Include all module headers */
#include "neurons/neuron.h"
#include "neurons/synapse.h"
#include "neurons/network.h"
#include "learning/hebbian.h"
#include "learning/stdp.h"
#include "perception/retina.h"
#include "perception/som.h"
#include "memory/hopfield.h"
#include "memory/decay.h"
#include "learning/prototype.h"
#include "thinking/spontaneous.h"
#include "thinking/dream.h"
#include "modular/brain.h"
#include "modular/emotion.h"
#include "understanding/predictor.h"
#include "understanding/causal.h"
#include "communication/comm.h"

#define RETHINK_FEATURE_DIM 32
#define RETHINK_LOG_SIZE 256

/* ──── Brain State ──── */
typedef enum {
    BRAIN_STATE_IDLE,       /* Default mode, spontaneous thought */
    BRAIN_STATE_PERCEIVING, /* Processing sensory input */
    BRAIN_STATE_LEARNING,   /* Actively updating models */
    BRAIN_STATE_THINKING,   /* Internal reasoning */
    BRAIN_STATE_DREAMING,   /* Sleep consolidation */
    BRAIN_STATE_COMMUNICATING /* Encoding/decoding language */
} BrainState;

/* ──── Experience record ──── */
typedef struct {
    float features[RETHINK_FEATURE_DIM];
    int dim;
    char description[64];
    float emotional_valence;
    float surprise;
    float understanding;
    int category;
    int timestamp;
} Experience;

/* ──── The Rethink Brain ──── */
typedef struct {
    /* State */
    BrainState state;
    int tick;                      /* Global time step */
    int experiences_count;
    
    /* Core modules */
    Retina retina;
    /* SOM som; */  /* Initialized separately due to size */
    HopfieldNet *memory;
    DecayMemory *decay_mem;
    PrototypeNet *proto;
    SpontaneousNet *spontaneous;
    DreamEngine *dream;
    Amygdala *amygdala;
    PredictiveNet *predictor;
    CausalNet *causal;
    CommEngine *comm;
    
    /* Current state */
    float current_features[RETHINK_FEATURE_DIM];
    float attention_level;       /* 0=zoned_out, 1=hyper_focused */
    float overall_understanding; /* Running average of understanding */
    float overall_surprise;      /* Running average of surprise */
    EmotionType current_emotion;
    
    /* Log */
    char log[RETHINK_LOG_SIZE][128];
    int log_count;
} RethinkBrain;

/* ──── Lifecycle ──── */
RethinkBrain *rethink_create(void);
void rethink_destroy(RethinkBrain *rb);

/* ──── Core Operations ──── */

/* Process a sensory experience through the full pipeline */
Experience rethink_experience(RethinkBrain *rb, const float *input, int dim, 
                               const char *label);

/* Learn a category from a labeled example */
void rethink_learn(RethinkBrain *rb, const float *features, int dim, 
                   const char *category);

/* Ask: What is this? (classify) */
int rethink_classify(RethinkBrain *rb, const float *features, int dim, 
                     char *out_category, float *out_confidence);

/* Ask: Why did this happen? (causal explanation) */
int rethink_explain(RethinkBrain *rb, const char *event, 
                    char *explanation, int max_len);

/* Ask: What happens if? (causal prediction) */
int rethink_predict(RethinkBrain *rb, const char *cause,
                    char *prediction, int max_len);

/* Communicate: describe current state */
void rethink_speak(RethinkBrain *rb, char *output, int max_len);

/* Communicate: understand a word sequence */
void rethink_listen(RethinkBrain *rb, const char **words, int num_words);

/* Idle processing: think, consolidate */
void rethink_idle(RethinkBrain *rb, int ticks);

/* Sleep: run dream engine for consolidation */
void rethink_sleep(RethinkBrain *rb, int cycles);

/* Tick: advance brain by one time step */
void rethink_tick(RethinkBrain *rb);

/* ──── Query ──── */
float rethink_understanding(const RethinkBrain *rb);
float rethink_surprise(const RethinkBrain *rb);
EmotionType rethink_emotion(const RethinkBrain *rb);
BrainState rethink_state(const RethinkBrain *rb);

/* ──── Printing ──── */
void rethink_print_status(const RethinkBrain *rb);
void rethink_print_log(const RethinkBrain *rb);

#endif /* RETHINK_BRAIN_H */
