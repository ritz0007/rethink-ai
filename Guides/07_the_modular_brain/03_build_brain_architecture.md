# 🔨 Build: Brain Architecture — Modular Regions with Central Routing

## What We're Building

The master architecture that ties everything together:
- **Brain regions** as independent modules
- **Thalamus** as central router / attention gate
- **Signal bus** for inter-region communication
- **Executive controller** for high-level decision making

## Code: `src/brain/brain.h`

```c
/*
 * brain.h — The Modular Brain Architecture
 *
 * Rethink AI — Phase 7
 *
 * Connects all modules through a central routing system.
 * Each brain region operates independently.
 * The thalamus routes and filters signals between regions.
 */

#ifndef BRAIN_H
#define BRAIN_H

#include "../perception/retina.h"
#include "../perception/som.h"
#include "../memory/hopfield.h"
#include "../memory/decay.h"
#include "../learning/prototype.h"
#include "../thinking/spontaneous.h"
#include "../thinking/dream.h"

#define MAX_SIGNALS 64
#define SIGNAL_DATA_SIZE 128
#define MAX_LOG_ENTRIES 256

/* ─── Brain Regions ─── */

typedef enum {
    REGION_VISUAL,       /* Retina + SOM */
    REGION_MEMORY,       /* Hopfield + Decay */
    REGION_LEARNING,     /* Prototype learner */
    REGION_THINKING,     /* Spontaneous + Dreams */
    REGION_EMOTION,      /* Importance tagging */
    REGION_EXECUTIVE,    /* Decision making */
    NUM_REGIONS
} RegionType;

static const char *REGION_NAMES[] = {
    "Visual", "Memory", "Learning", "Thinking", "Emotion", "Executive"
};

/* ─── Signal Bus ─── */

typedef enum {
    SIG_VISUAL_FEATURES,   /* Visual cortex produced features */
    SIG_MEMORY_MATCH,      /* Memory found a match */
    SIG_MEMORY_MISS,       /* Memory found nothing */
    SIG_CATEGORY,          /* Learning identified a category */
    SIG_NOVELTY,           /* Learning detected something new */
    SIG_EMOTION_TAG,       /* Emotion assessed importance */
    SIG_THOUGHT,           /* Spontaneous thought emerged */
    SIG_INSIGHT,           /* Dream discovered something */
    SIG_DECISION,          /* Executive made a decision */
    SIG_ATTENTION_SHIFT,   /* Change attention focus */
    NUM_SIGNAL_TYPES
} SignalType;

typedef struct {
    RegionType source;
    RegionType target;         /* NUM_REGIONS = broadcast to all */
    SignalType type;
    float data[SIGNAL_DATA_SIZE];
    int data_size;
    float urgency;             /* 0 to 1 */
    float timestamp;
    int active;                /* 1 if this slot is in use */
} Signal;

/* ─── Thalamus (Router) ─── */

typedef struct {
    float gain[NUM_REGIONS];           /* Attention gain per region */
    float priority[NUM_REGIONS];       /* Top-down attention priority */
    float surprise[NUM_REGIONS];       /* Bottom-up surprise signal */
    float attention_decay;             /* How fast attention returns to baseline */
    float surprise_threshold;          /* Signal strength to trigger surprise */
} Thalamus;

/* ─── Activity Log ─── */

typedef struct {
    float timestamp;
    RegionType region;
    SignalType signal_type;
    char description[64];
} LogEntry;

/* ─── The Brain ─── */

typedef struct {
    /* Modules */
    Retina *visual;
    SOM *organizer;
    DecayMemory *memory;
    ProtoLearner *learner;
    SpontaneousNet *mind;
    DreamEngine *dreams;
    
    /* Infrastructure */
    Thalamus thalamus;
    Signal signals[MAX_SIGNALS];
    int signal_count;
    
    /* State */
    float time;
    int awake;                         /* 1 = awake, 0 = sleeping */
    
    /* Logging */
    LogEntry log[MAX_LOG_ENTRIES];
    int log_count;
    int verbose;                       /* Print activity as it happens */
} Brain;

/* ─── Lifecycle ─── */
Brain *brain_create(void);
void brain_destroy(Brain *b);

/* ─── Core Loop ─── */

/* Process visual input (image) */
void brain_see(Brain *b, const float *pixels, int width, int height);

/* Learn a category from features + label */
void brain_learn(Brain *b, const float *features, int dim, const char *label);

/* Classify: brain looks at features and returns its best guess */
int brain_classify(Brain *b, const float *features, int dim, 
                   char *label_out, float *confidence);

/* One "tick" of brain time — runs spontaneous activity, routes signals */
void brain_tick(Brain *b, float dt);

/* Sleep for N cycles */
void brain_sleep(Brain *b, int cycles);

/* ─── Thalamus Controls ─── */
void brain_set_attention(Brain *b, RegionType region, float priority);
void brain_shift_attention(Brain *b, RegionType towards);

/* ─── Signal Bus ─── */
void brain_emit_signal(Brain *b, RegionType source, RegionType target,
                       SignalType type, const float *data, int size, float urgency);
void brain_process_signals(Brain *b);

/* ─── Logging ─── */
void brain_log(Brain *b, RegionType region, SignalType type, const char *desc);
void brain_print_status(const Brain *b);
void brain_print_log(const Brain *b, int last_n);

#endif /* BRAIN_H */
```

## Code: `src/brain/brain.c`

```c
#include "brain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─── Creation & Destruction ─── */

Brain *brain_create(void) {
    Brain *b = (Brain *)calloc(1, sizeof(Brain));
    
    /* Create all modules */
    b->visual = retina_create(32);
    b->organizer = som_create(8, 8, 64);
    b->memory = decay_memory_create(64);
    b->learner = proto_create(16);
    b->mind = spontaneous_create(64);
    b->dreams = dream_create(b->mind, b->memory);
    
    /* Initialize thalamus */
    for (int r = 0; r < NUM_REGIONS; r++) {
        b->thalamus.gain[r] = 1.0f / NUM_REGIONS;
        b->thalamus.priority[r] = 0.5f;
        b->thalamus.surprise[r] = 0.0f;
    }
    b->thalamus.attention_decay = 0.1f;
    b->thalamus.surprise_threshold = 0.7f;
    
    b->time = 0.0f;
    b->awake = 1;
    b->signal_count = 0;
    b->log_count = 0;
    b->verbose = 1;
    
    return b;
}

void brain_destroy(Brain *b) {
    dream_destroy(b->dreams);
    spontaneous_destroy(b->mind);
    proto_destroy(b->learner);
    decay_memory_destroy(b->memory);
    som_destroy(b->organizer);
    retina_destroy(b->visual);
    free(b);
}

/* ─── Logging ─── */

void brain_log(Brain *b, RegionType region, SignalType type, const char *desc) {
    if (b->log_count >= MAX_LOG_ENTRIES) return;
    LogEntry *e = &b->log[b->log_count++];
    e->timestamp = b->time;
    e->region = region;
    e->signal_type = type;
    strncpy(e->description, desc, 63);
    e->description[63] = '\0';
    
    if (b->verbose) {
        printf("  [%.1f] %s: %s\n", b->time, REGION_NAMES[region], desc);
    }
}

/* ─── Signal Bus ─── */

void brain_emit_signal(Brain *b, RegionType source, RegionType target,
                       SignalType type, const float *data, int size, float urgency) {
    if (b->signal_count >= MAX_SIGNALS) return;
    
    Signal *s = &b->signals[b->signal_count++];
    s->source = source;
    s->target = target;
    s->type = type;
    s->data_size = (size < SIGNAL_DATA_SIZE) ? size : SIGNAL_DATA_SIZE;
    if (data && s->data_size > 0) {
        memcpy(s->data, data, s->data_size * sizeof(float));
    }
    s->urgency = urgency;
    s->timestamp = b->time;
    s->active = 1;
}

void brain_process_signals(Brain *b) {
    Thalamus *th = &b->thalamus;
    
    for (int i = 0; i < b->signal_count; i++) {
        Signal *s = &b->signals[i];
        if (!s->active) continue;
        
        /* Apply thalamic gating */
        float gate = th->gain[s->target] * s->urgency;
        
        if (gate < 0.1f) {
            /* Blocked by attention gate */
            s->active = 0;
            continue;
        }
        
        /* Route signal to target */
        switch (s->type) {
            case SIG_VISUAL_FEATURES:
                /* Send to memory for matching */
                if (s->target == REGION_MEMORY && s->data_size > 0) {
                    float recalled[256];
                    float sim = decay_memory_recall(b->memory, s->data, recalled, 50);
                    if (sim > 0.7f) {
                        brain_log(b, REGION_MEMORY, SIG_MEMORY_MATCH, "Pattern recognized!");
                        brain_emit_signal(b, REGION_MEMORY, REGION_EXECUTIVE,
                                         SIG_MEMORY_MATCH, &sim, 1, 0.6f);
                    } else {
                        brain_log(b, REGION_MEMORY, SIG_MEMORY_MISS, "Pattern unknown");
                        brain_emit_signal(b, REGION_MEMORY, REGION_LEARNING,
                                         SIG_NOVELTY, s->data, s->data_size, 0.8f);
                    }
                }
                break;
                
            case SIG_NOVELTY:
                /* New thing detected → heighten attention to learning */
                th->surprise[REGION_LEARNING] = 0.9f;
                brain_log(b, REGION_LEARNING, SIG_NOVELTY, "Novel input detected!");
                break;
                
            case SIG_EMOTION_TAG:
                /* Emotion says "this is important" → boost memory */
                brain_log(b, REGION_EMOTION, SIG_EMOTION_TAG, "Importance tagged");
                break;
                
            case SIG_THOUGHT:
                brain_log(b, REGION_THINKING, SIG_THOUGHT, "Spontaneous thought emerged");
                break;
                
            default:
                break;
        }
        
        s->active = 0;  /* Signal consumed */
    }
    
    /* Clean up consumed signals */
    b->signal_count = 0;
    
    /* Update thalamic attention */
    for (int r = 0; r < NUM_REGIONS; r++) {
        /* Bottom-up: surprise boosts attention */
        if (th->surprise[r] > th->surprise_threshold) {
            th->gain[r] = fminf(1.0f, th->gain[r] + 0.2f);
        }
        
        /* Blend with top-down priority */
        th->gain[r] = th->gain[r] * 0.8f + th->priority[r] * 0.2f;
        
        /* Decay surprise */
        th->surprise[r] *= (1.0f - th->attention_decay);
    }
    
    /* Normalize gains (zero-sum attention) */
    float total = 0;
    for (int r = 0; r < NUM_REGIONS; r++) total += th->gain[r];
    if (total > 0) {
        for (int r = 0; r < NUM_REGIONS; r++) th->gain[r] /= total;
    }
}

/* ─── Core Loop ─── */

void brain_see(Brain *b, const float *pixels, int width, int height) {
    brain_log(b, REGION_VISUAL, SIG_VISUAL_FEATURES, "Processing visual input");
    
    /* Step 1: Retina processes edges */
    retina_load_pixels(b->visual, pixels, width, height);
    retina_detect_edges(b->visual, EDGE_ALL);
    retina_apply_inhibition(b->visual);
    
    /* Step 2: Extract features */
    float features[PROTO_MAX_FEATURES];
    int fdim = retina_to_features(b->visual, features, PROTO_MAX_FEATURES);
    
    /* Step 3: Emit to other regions */
    brain_emit_signal(b, REGION_VISUAL, REGION_MEMORY,
                     SIG_VISUAL_FEATURES, features, fdim, 0.7f);
    brain_emit_signal(b, REGION_VISUAL, REGION_LEARNING,
                     SIG_VISUAL_FEATURES, features, fdim, 0.5f);
}

void brain_learn(Brain *b, const float *features, int dim, const char *label) {
    char msg[64];
    snprintf(msg, 63, "Learning '%s'", label);
    brain_log(b, REGION_LEARNING, SIG_CATEGORY, msg);
    
    proto_teach(b->learner, features, label);
    
    /* Also store in memory */
    float mem_pattern[256] = {0};
    int copy_dim = dim < 256 ? dim : 256;
    memcpy(mem_pattern, features, copy_dim * sizeof(float));
    decay_memory_store(b->memory, mem_pattern, 0.5f);
}

int brain_classify(Brain *b, const float *features, int dim, 
                   char *label_out, float *confidence) {
    float sim;
    int idx = proto_classify(b->learner, features, &sim);
    
    if (idx >= 0) {
        const char *label = proto_get_label(b->learner, idx);
        if (label_out) strncpy(label_out, label, 31);
        if (confidence) *confidence = sim;
        
        char msg[64];
        snprintf(msg, 63, "Classified as '%s' (%.2f)", label, sim);
        brain_log(b, REGION_LEARNING, SIG_CATEGORY, msg);
    } else {
        if (label_out) strcpy(label_out, "unknown");
        if (confidence) *confidence = sim;
        brain_log(b, REGION_LEARNING, SIG_NOVELTY, "Unknown input");
    }
    
    return idx;
}

void brain_tick(Brain *b, float dt) {
    b->time += dt;
    
    if (b->awake) {
        /* Process pending signals */
        brain_process_signals(b);
        
        /* Run spontaneous activity */
        spontaneous_step(b->mind);
        
        /* Decay memories */
        decay_memory_tick(b->memory, dt);
    }
}

void brain_sleep(Brain *b, int cycles) {
    brain_log(b, REGION_THINKING, SIG_THOUGHT, "Going to sleep...");
    b->awake = 0;
    
    for (int c = 0; c < cycles; c++) {
        dream_sleep_cycle(b->dreams);
        b->time += 30.0f;  /* Each cycle ≈ 30 time units */
    }
    
    b->awake = 1;
    brain_log(b, REGION_THINKING, SIG_THOUGHT, "Waking up.");
}

/* ─── Attention Controls ─── */

void brain_set_attention(Brain *b, RegionType region, float priority) {
    b->thalamus.priority[region] = priority;
}

void brain_shift_attention(Brain *b, RegionType towards) {
    for (int r = 0; r < NUM_REGIONS; r++) {
        b->thalamus.priority[r] = (r == towards) ? 0.8f : 0.2f;
    }
    char msg[64];
    snprintf(msg, 63, "Attention → %s", REGION_NAMES[towards]);
    brain_log(b, REGION_EXECUTIVE, SIG_ATTENTION_SHIFT, msg);
}

/* ─── Status ─── */

void brain_print_status(const Brain *b) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║        Brain Status (t=%.1f)          \n", b->time);
    printf("╠══════════════════════════════════════╣\n");
    printf("║  State: %s                            \n", b->awake ? "AWAKE" : "SLEEPING");
    printf("║  Memories: %d alive                   \n", decay_memory_alive_count(b->memory));
    printf("║  Categories: %d learned               \n", b->learner->num_categories);
    printf("║  Dream cycles: %d                     \n", b->dreams->cycle_count);
    printf("║  Insights: %d discovered              \n", b->dreams->num_insights);
    printf("║                                        \n");
    printf("║  Attention (thalamic gain):            \n");
    for (int r = 0; r < NUM_REGIONS; r++) {
        int bar = (int)(b->thalamus.gain[r] * 30);
        printf("║    %-10s ", REGION_NAMES[r]);
        for (int i = 0; i < bar; i++) printf("█");
        for (int i = bar; i < 30; i++) printf("░");
        printf(" %.2f\n", b->thalamus.gain[r]);
    }
    printf("╚══════════════════════════════════════╝\n");
}

void brain_print_log(const Brain *b, int last_n) {
    int start = b->log_count - last_n;
    if (start < 0) start = 0;
    printf("\n--- Brain Log (last %d entries) ---\n", b->log_count - start);
    for (int i = start; i < b->log_count; i++) {
        const LogEntry *e = &b->log[i];
        printf("  [%.1f] %-10s %s\n", e->timestamp, REGION_NAMES[e->region], e->description);
    }
    printf("--- End Log ---\n");
}
```

## Key Design Decisions

### 1. Regions Don't Know About Each Other
Visual cortex doesn't call `memory_recall()`. It emits a `SIG_VISUAL_FEATURES` signal, and the thalamus routes it.

### 2. Attention is Competitive (Zero-Sum)
When you focus on one region, others get less bandwidth. This is biologically accurate and prevents the system from being overwhelmed.

### 3. Surprise Breaks Through
Even if attention is elsewhere, a surprising signal (high urgency + bottom-up surprise) can break through the gate. This is why a loud noise grabs your attention even when reading.

### 4. Logging Everything
The brain keeps a log of all activity. This is invaluable for debugging ("why did it classify that as a dog?").

---

*Next: [Build — The Emotion System](04_build_emotion_system.md)*
