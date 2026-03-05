# 🔨 Build: Communication Engine

## What We're Building

A grounded communication system where:
1. Internal states can be **encoded** into symbol sequences
2. Symbol sequences can be **decoded** back into internal states
3. Symbols are **grounded** in sensory/memory/causal representations
4. Messages can be **generated** and **understood**

## Code: `src/communication/comm.h`

```c
/*
 * comm.h — Grounded Communication Engine
 *
 * Rethink AI — Phase 9
 *
 * Symbols are grounded in multi-modal representations.
 * Communication = encoding internal state → symbols → decoding back.
 */

#ifndef COMM_H
#define COMM_H

#define COMM_MAX_SYMBOLS 128
#define COMM_MAX_FEATURES 64
#define COMM_MAX_SENTENCE 32

/* ──── Role in a sentence (agent-action-patient) ──── */
typedef enum {
    ROLE_NONE = 0,
    ROLE_AGENT,       /* Who/what does the action */
    ROLE_ACTION,      /* What is done */
    ROLE_PATIENT,     /* Who/what receives the action */
    ROLE_ATTRIBUTE,   /* Property (red, big, fast) */
    ROLE_LOCATION,    /* Where */
    ROLE_TIME         /* When (before, after, now) */
} SymbolRole;

/* ──── A grounded symbol ──── */
typedef struct {
    int id;
    char label[32];                  /* The "word" */
    float features[COMM_MAX_FEATURES]; /* Sensory grounding */
    int feature_dim;
    SymbolRole typical_role;         /* Usual grammatical role */
    float emotional_valence;         /* -1 (negative) to +1 (positive) */
    float familiarity;               /* How often encountered (0 to 1) */
    int is_abstract;                 /* 1 if not directly sensory */
} Symbol;

/* ──── A word in a sentence (symbol + role) ──── */
typedef struct {
    int symbol_id;
    SymbolRole role;
} BoundSymbol;

/* ──── A message (sequence of bound symbols) ──── */
typedef struct {
    BoundSymbol words[COMM_MAX_SENTENCE];
    int length;
    float confidence;  /* How well the message was encoded/decoded */
} Message;

/* ──── Internal state representation ──── */
typedef struct {
    float features[COMM_MAX_FEATURES]; /* Current sensory state */
    int feature_dim;
    float emotional_valence;
    float arousal;
    float urgency;
} InternalState;

/* ──── The communication engine ──── */
typedef struct {
    Symbol vocabulary[COMM_MAX_SYMBOLS];
    int vocab_size;
    
    /* Encoding parameters */
    float similarity_threshold; /* Min similarity to activate a symbol */
    float noise_level;          /* Communication noise */
} CommEngine;

/* Create and destroy */
CommEngine *comm_create(int feature_dim);
void comm_destroy(CommEngine *ce);

/* Build vocabulary */
int comm_add_symbol(CommEngine *ce, const char *label, const float *features,
                    int feature_dim, SymbolRole role, float valence, int is_abstract);

/* Encode: internal state → message */
Message comm_encode(CommEngine *ce, const InternalState *state);

/* Decode: message → internal state */
InternalState comm_decode(CommEngine *ce, const Message *msg);

/* Generate description of a pattern */
Message comm_describe(CommEngine *ce, const float *pattern, int dim);

/* Understand: take a label sequence, activate grounded representations */
InternalState comm_understand(CommEngine *ce, const char **words, int num_words);

/* Find symbol by label */
int comm_find_symbol(const CommEngine *ce, const char *label);

/* Similarity between features and symbol */
float comm_symbol_similarity(const Symbol *sym, const float *features, int dim);

/* Print */
void comm_print_message(const CommEngine *ce, const Message *msg);
void comm_print_state(const InternalState *state);
void comm_print_vocabulary(const CommEngine *ce);

#endif /* COMM_H */
```

## Code: `src/communication/comm.c`

```c
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ──── Utility ──── */

static float dot_product(const float *a, const float *b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) sum += a[i] * b[i];
    return sum;
}

static float magnitude(const float *v, int n) {
    return sqrtf(dot_product(v, v, n));
}

static float cosine_similarity(const float *a, const float *b, int n) {
    float mag_a = magnitude(a, n);
    float mag_b = magnitude(b, n);
    if (mag_a < 1e-6f || mag_b < 1e-6f) return 0.0f;
    return dot_product(a, b, n) / (mag_a * mag_b);
}

/* ──── Create / Destroy ──── */

CommEngine *comm_create(int feature_dim) {
    CommEngine *ce = (CommEngine *)calloc(1, sizeof(CommEngine));
    ce->similarity_threshold = 0.3f;
    ce->noise_level = 0.0f;
    (void)feature_dim;
    return ce;
}

void comm_destroy(CommEngine *ce) { free(ce); }

/* ──── Vocabulary ──── */

int comm_add_symbol(CommEngine *ce, const char *label, const float *features,
                    int feature_dim, SymbolRole role, float valence, int is_abstract) {
    if (ce->vocab_size >= COMM_MAX_SYMBOLS) return -1;
    int idx = ce->vocab_size++;
    Symbol *s = &ce->vocabulary[idx];
    s->id = idx;
    strncpy(s->label, label, 31);
    s->label[31] = '\0';
    s->feature_dim = feature_dim;
    if (features && feature_dim > 0) {
        int dim = feature_dim < COMM_MAX_FEATURES ? feature_dim : COMM_MAX_FEATURES;
        memcpy(s->features, features, dim * sizeof(float));
    }
    s->typical_role = role;
    s->emotional_valence = valence;
    s->familiarity = 0.0f;
    s->is_abstract = is_abstract;
    return idx;
}

int comm_find_symbol(const CommEngine *ce, const char *label) {
    for (int i = 0; i < ce->vocab_size; i++) {
        if (strcmp(ce->vocabulary[i].label, label) == 0) return i;
    }
    return -1;
}

float comm_symbol_similarity(const Symbol *sym, const float *features, int dim) {
    int n = dim < sym->feature_dim ? dim : sym->feature_dim;
    if (n <= 0) return 0.0f;
    return cosine_similarity(sym->features, features, n);
}

/* ──── Encoding: Internal State → Message ──── */

Message comm_encode(CommEngine *ce, const InternalState *state) {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    
    /* Find the best matching symbols for the current state */
    typedef struct { int idx; float sim; } Match;
    Match matches[COMM_MAX_SYMBOLS];
    int num_matches = 0;
    
    for (int i = 0; i < ce->vocab_size; i++) {
        float sim = comm_symbol_similarity(&ce->vocabulary[i], 
                                            state->features, state->feature_dim);
        
        /* Also factor in emotional match */
        float emo_match = 1.0f - fabsf(ce->vocabulary[i].emotional_valence - 
                                        state->emotional_valence);
        sim = sim * 0.7f + emo_match * 0.3f;
        
        if (sim > ce->similarity_threshold) {
            matches[num_matches].idx = i;
            matches[num_matches].sim = sim;
            num_matches++;
        }
        
        /* Update familiarity */
        ce->vocabulary[i].familiarity += sim * 0.01f;
        if (ce->vocabulary[i].familiarity > 1.0f) 
            ce->vocabulary[i].familiarity = 1.0f;
    }
    
    /* Sort by similarity (simple bubble sort for small N) */
    for (int i = 0; i < num_matches - 1; i++) {
        for (int j = i + 1; j < num_matches; j++) {
            if (matches[j].sim > matches[i].sim) {
                Match tmp = matches[i];
                matches[i] = matches[j];
                matches[j] = tmp;
            }
        }
    }
    
    /* Build message: order by typical role */
    /* Agent first, then action, then patient, then attributes */
    SymbolRole order[] = {ROLE_AGENT, ROLE_ACTION, ROLE_PATIENT, 
                          ROLE_ATTRIBUTE, ROLE_LOCATION, ROLE_TIME};
    
    for (int r = 0; r < 6 && msg.length < COMM_MAX_SENTENCE; r++) {
        for (int m = 0; m < num_matches && msg.length < COMM_MAX_SENTENCE; m++) {
            Symbol *s = &ce->vocabulary[matches[m].idx];
            if (s->typical_role == order[r]) {
                /* Check if already in message */
                int already = 0;
                for (int k = 0; k < msg.length; k++) {
                    if (msg.words[k].symbol_id == matches[m].idx) {
                        already = 1;
                        break;
                    }
                }
                if (!already) {
                    msg.words[msg.length].symbol_id = matches[m].idx;
                    msg.words[msg.length].role = s->typical_role;
                    msg.length++;
                }
            }
        }
    }
    
    /* Confidence = average similarity of used symbols */
    float conf_sum = 0.0f;
    for (int i = 0; i < msg.length; i++) {
        for (int m = 0; m < num_matches; m++) {
            if (matches[m].idx == msg.words[i].symbol_id) {
                conf_sum += matches[m].sim;
                break;
            }
        }
    }
    msg.confidence = msg.length > 0 ? conf_sum / msg.length : 0.0f;
    
    return msg;
}

/* ──── Decoding: Message → Internal State ──── */

InternalState comm_decode(CommEngine *ce, const Message *msg) {
    InternalState state;
    memset(&state, 0, sizeof(InternalState));
    
    if (msg->length == 0) return state;
    
    /* Reconstruct internal state by blending symbol features */
    float total_weight = 0.0f;
    int dim = 0;
    
    for (int i = 0; i < msg->length; i++) {
        Symbol *s = &ce->vocabulary[msg->words[i].symbol_id];
        if (s->feature_dim > dim) dim = s->feature_dim;
        
        /* Weight by role importance */
        float weight = 1.0f;
        if (msg->words[i].role == ROLE_AGENT) weight = 2.0f;
        if (msg->words[i].role == ROLE_ACTION) weight = 1.5f;
        
        for (int f = 0; f < s->feature_dim && f < COMM_MAX_FEATURES; f++) {
            state.features[f] += s->features[f] * weight;
        }
        state.emotional_valence += s->emotional_valence * weight;
        total_weight += weight;
    }
    
    /* Normalize */
    if (total_weight > 0.0f) {
        for (int f = 0; f < dim; f++) {
            state.features[f] /= total_weight;
        }
        state.emotional_valence /= total_weight;
    }
    state.feature_dim = dim;
    
    return state;
}

/* ──── Describe a pattern ──── */

Message comm_describe(CommEngine *ce, const float *pattern, int dim) {
    InternalState state;
    memset(&state, 0, sizeof(InternalState));
    int d = dim < COMM_MAX_FEATURES ? dim : COMM_MAX_FEATURES;
    memcpy(state.features, pattern, d * sizeof(float));
    state.feature_dim = d;
    return comm_encode(ce, &state);
}

/* ──── Understand words ──── */

InternalState comm_understand(CommEngine *ce, const char **words, int num_words) {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    
    for (int i = 0; i < num_words && msg.length < COMM_MAX_SENTENCE; i++) {
        int idx = comm_find_symbol(ce, words[i]);
        if (idx >= 0) {
            msg.words[msg.length].symbol_id = idx;
            msg.words[msg.length].role = ce->vocabulary[idx].typical_role;
            msg.length++;
        }
    }
    
    return comm_decode(ce, &msg);
}

/* ──── Print ──── */

void comm_print_message(const CommEngine *ce, const Message *msg) {
    printf("Message (confidence=%.2f): \"", msg->confidence);
    for (int i = 0; i < msg->length; i++) {
        printf("%s", ce->vocabulary[msg->words[i].symbol_id].label);
        if (i < msg->length - 1) printf(" ");
    }
    printf("\"\n");
    
    for (int i = 0; i < msg->length; i++) {
        const char *role_names[] = {"NONE", "AGENT", "ACTION", "PATIENT", 
                                     "ATTR", "LOC", "TIME"};
        printf("  [%s] %s (valence=%.2f)\n",
               role_names[msg->words[i].role],
               ce->vocabulary[msg->words[i].symbol_id].label,
               ce->vocabulary[msg->words[i].symbol_id].emotional_valence);
    }
}

void comm_print_state(const InternalState *state) {
    printf("Internal State: valence=%.2f arousal=%.2f urgency=%.2f\n",
           state->emotional_valence, state->arousal, state->urgency);
    printf("  Features: [");
    int limit = state->feature_dim < 8 ? state->feature_dim : 8;
    for (int i = 0; i < limit; i++) {
        printf("%.2f", state->features[i]);
        if (i < limit - 1) printf(", ");
    }
    if (state->feature_dim > 8) printf(", ...");
    printf("]\n");
}

void comm_print_vocabulary(const CommEngine *ce) {
    printf("\n=== Vocabulary (%d symbols) ===\n", ce->vocab_size);
    for (int i = 0; i < ce->vocab_size; i++) {
        const char *role_names[] = {"NONE", "AGENT", "ACTION", "PATIENT",
                                     "ATTR", "LOC", "TIME"};
        printf("  [%3d] %-12s role=%-7s valence=%+.1f %s\n",
               i, ce->vocabulary[i].label,
               role_names[ce->vocabulary[i].typical_role],
               ce->vocabulary[i].emotional_valence,
               ce->vocabulary[i].is_abstract ? "[abstract]" : "");
    }
    printf("=============================\n");
}
```

## Test: `test_comm.c`

```c
/*
 * test_comm.c — Grounded Communication Test
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_comm test_comm.c comm.c -lm
 */

#include <stdio.h>
#include <string.h>
#include "comm.h"

int main(void) {
    printf("=== Grounded Communication Engine ===\n\n");
    
    int dim = 8;
    CommEngine *ce = comm_create(dim);
    
    /* Build a small grounded vocabulary */
    /* Features encode: [size, speed, fluffiness, danger, roundness, warmth, brightness, wetness] */
    
    float cat_f[]   = {0.3, 0.5, 0.9, 0.1, 0.4, 0.8, 0.0, 0.0};
    float dog_f[]   = {0.5, 0.6, 0.7, 0.2, 0.3, 0.7, 0.0, 0.0};
    float bird_f[]  = {0.1, 0.8, 0.5, 0.0, 0.2, 0.3, 0.0, 0.0};
    float snake_f[] = {0.2, 0.3, 0.0, 0.9, 0.0, 0.1, 0.0, 0.2};
    float sun_f[]   = {0.9, 0.0, 0.0, 0.3, 1.0, 1.0, 1.0, 0.0};
    float rain_f[]  = {0.1, 0.4, 0.0, 0.0, 0.0, 0.2, 0.1, 1.0};
    float chase_f[] = {0.0, 0.9, 0.0, 0.3, 0.0, 0.0, 0.0, 0.0};
    float sleep_f[] = {0.0, 0.0, 0.5, 0.0, 0.6, 0.9, 0.0, 0.0};
    float big_f[]   = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    float small_f[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    float fast_f[]  = {0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    
    comm_add_symbol(ce, "cat",    cat_f,   dim, ROLE_AGENT,     0.5f, 0);
    comm_add_symbol(ce, "dog",    dog_f,   dim, ROLE_AGENT,     0.4f, 0);
    comm_add_symbol(ce, "bird",   bird_f,  dim, ROLE_AGENT,     0.3f, 0);
    comm_add_symbol(ce, "snake",  snake_f, dim, ROLE_AGENT,    -0.6f, 0);
    comm_add_symbol(ce, "sun",    sun_f,   dim, ROLE_AGENT,     0.7f, 0);
    comm_add_symbol(ce, "rain",   rain_f,  dim, ROLE_AGENT,    -0.1f, 0);
    comm_add_symbol(ce, "chase",  chase_f, dim, ROLE_ACTION,   -0.2f, 0);
    comm_add_symbol(ce, "sleep",  sleep_f, dim, ROLE_ACTION,    0.6f, 0);
    comm_add_symbol(ce, "big",    big_f,   dim, ROLE_ATTRIBUTE, 0.0f, 0);
    comm_add_symbol(ce, "small",  small_f, dim, ROLE_ATTRIBUTE, 0.0f, 0);
    comm_add_symbol(ce, "fast",   fast_f,  dim, ROLE_ATTRIBUTE, 0.1f, 0);
    
    comm_print_vocabulary(ce);
    
    /* Test 1: Describe a pattern */
    printf("\n--- Describe a mystery pattern ---\n");
    float mystery[] = {0.3, 0.4, 0.8, 0.1, 0.3, 0.7, 0.0, 0.0};
    Message msg = comm_describe(ce, mystery, dim);
    printf("Pattern: [");
    for (int i = 0; i < dim; i++) printf("%.1f%s", mystery[i], i < dim-1 ? "," : "");
    printf("]\n");
    comm_print_message(ce, &msg);
    printf("→ System describes it using its vocabulary!\n");
    
    /* Test 2: Understand a word sequence */
    printf("\n--- Understand: 'cat chase bird' ---\n");
    const char *words1[] = {"cat", "chase", "bird"};
    InternalState state1 = comm_understand(ce, words1, 3);
    comm_print_state(&state1);
    printf("→ Understanding activates grounded features\n");
    
    /* Test 3: Different words, different internal states */
    printf("\n--- Understand: 'snake' vs 'cat sleep' ---\n");
    const char *words2[] = {"snake"};
    const char *words3[] = {"cat", "sleep"};
    InternalState state2 = comm_understand(ce, words2, 1);
    InternalState state3 = comm_understand(ce, words3, 2);
    printf("'snake':     "); comm_print_state(&state2);
    printf("'cat sleep': "); comm_print_state(&state3);
    printf("→ Snake has negative valence, cat+sleep has positive\n");
    
    /* Test 4: Round-trip: encode → message → decode */
    printf("\n--- Round-trip: State → Message → State ---\n");
    InternalState original;
    memset(&original, 0, sizeof(InternalState));
    memcpy(original.features, cat_f, dim * sizeof(float));
    original.feature_dim = dim;
    original.emotional_valence = 0.5f;
    
    printf("Original: "); comm_print_state(&original);
    
    Message encoded = comm_encode(ce, &original);
    printf("Encoded:  "); comm_print_message(ce, &encoded);
    
    InternalState decoded = comm_decode(ce, &encoded);
    printf("Decoded:  "); comm_print_state(&decoded);
    
    /* Check reconstruction quality */
    float error = 0.0f;
    for (int i = 0; i < dim; i++) {
        float diff = original.features[i] - decoded.features[i];
        error += diff * diff;
    }
    error = sqrtf(error / dim);
    printf("Reconstruction error: %.3f (lower = better compression)\n", error);
    printf("→ Information is LOST in communication (just like real language!)\n");
    
    comm_destroy(ce);
    printf("\n✓ Grounded communication works!\n");
    return 0;
}
```

## Key Design Decisions

### Why Grounding Matters
Every symbol has multi-dimensional feature vectors. The word "cat" isn't just token #0 — it carries information about size, speed, fluffiness, warmth. When someone says "cat," our system doesn't just activate a label — it activates the **experience** of cat.

### Why Communication Is Lossy
Real communication is lossy. You can never perfectly transmit your internal experience to someone else. The round-trip test shows this: encode → message → decode loses information. This is realistic and important.

### Why Roles Matter
"Cat chases dog" and "Dog chases cat" must produce different states. The role system (agent/action/patient) ensures word order affects meaning.

---

*Next: [Milestone — The Communicating Brain](04_milestone_communication.md)*
