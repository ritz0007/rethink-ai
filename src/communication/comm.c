/*
 * comm.c — Communication Engine Implementation
 * Rethink AI — Phase 9
 */

#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static float dot_prod(const float *a, const float *b, int n) {
    float sum = 0; for (int i = 0; i < n; i++) sum += a[i] * b[i]; return sum;
}

static float mag(const float *v, int n) { return sqrtf(dot_prod(v, v, n)); }

static float cos_sim(const float *a, const float *b, int n) {
    float ma = mag(a, n), mb = mag(b, n);
    if (ma < 1e-6f || mb < 1e-6f) return 0.0f;
    return dot_prod(a, b, n) / (ma * mb);
}

CommEngine *comm_create(int feature_dim) {
    CommEngine *ce = (CommEngine *)calloc(1, sizeof(CommEngine));
    ce->similarity_threshold = 0.3f;
    (void)feature_dim;
    return ce;
}

void comm_destroy(CommEngine *ce) { free(ce); }

int comm_add_symbol(CommEngine *ce, const char *label, const float *features,
                    int feature_dim, SymbolRole role, float valence, int is_abstract) {
    if (ce->vocab_size >= COMM_MAX_SYMBOLS) return -1;
    int idx = ce->vocab_size++;
    Symbol *s = &ce->vocabulary[idx];
    s->id = idx;
    strncpy(s->label, label, 31); s->label[31] = '\0';
    s->feature_dim = feature_dim;
    if (features && feature_dim > 0) {
        int d = feature_dim < COMM_MAX_FEATURES ? feature_dim : COMM_MAX_FEATURES;
        memcpy(s->features, features, d * sizeof(float));
    }
    s->typical_role = role;
    s->emotional_valence = valence;
    s->is_abstract = is_abstract;
    return idx;
}

int comm_find_symbol(const CommEngine *ce, const char *label) {
    for (int i = 0; i < ce->vocab_size; i++)
        if (strcmp(ce->vocabulary[i].label, label) == 0) return i;
    return -1;
}

float comm_symbol_similarity(const Symbol *sym, const float *features, int dim) {
    int n = dim < sym->feature_dim ? dim : sym->feature_dim;
    if (n <= 0) return 0.0f;
    return cos_sim(sym->features, features, n);
}

Message comm_encode(CommEngine *ce, const InternalState *state) {
    Message msg; memset(&msg, 0, sizeof(Message));
    
    typedef struct { int idx; float sim; } Match;
    Match matches[COMM_MAX_SYMBOLS];
    int num = 0;
    
    for (int i = 0; i < ce->vocab_size; i++) {
        float sim = comm_symbol_similarity(&ce->vocabulary[i], state->features, state->feature_dim);
        float emo_match = 1.0f - fabsf(ce->vocabulary[i].emotional_valence - state->emotional_valence);
        sim = sim * 0.7f + emo_match * 0.3f;
        if (sim > ce->similarity_threshold) { matches[num].idx = i; matches[num].sim = sim; num++; }
    }
    
    /* Sort by similarity */
    for (int i = 0; i < num - 1; i++)
        for (int j = i + 1; j < num; j++)
            if (matches[j].sim > matches[i].sim) { Match t = matches[i]; matches[i] = matches[j]; matches[j] = t; }
    
    /* Build by role order */
    SymbolRole order[] = {ROLE_AGENT, ROLE_ACTION, ROLE_PATIENT, ROLE_ATTRIBUTE, ROLE_LOCATION, ROLE_TIME};
    for (int r = 0; r < 6 && msg.length < COMM_MAX_SENTENCE; r++) {
        for (int m = 0; m < num && msg.length < COMM_MAX_SENTENCE; m++) {
            if (ce->vocabulary[matches[m].idx].typical_role == order[r]) {
                int already = 0;
                for (int k = 0; k < msg.length; k++)
                    if (msg.words[k].symbol_id == matches[m].idx) { already = 1; break; }
                if (!already) {
                    msg.words[msg.length].symbol_id = matches[m].idx;
                    msg.words[msg.length].role = ce->vocabulary[matches[m].idx].typical_role;
                    msg.length++;
                }
            }
        }
    }
    
    float conf = 0;
    for (int i = 0; i < msg.length; i++)
        for (int m = 0; m < num; m++)
            if (matches[m].idx == msg.words[i].symbol_id) { conf += matches[m].sim; break; }
    msg.confidence = msg.length > 0 ? conf / msg.length : 0.0f;
    return msg;
}

InternalState comm_decode(CommEngine *ce, const Message *msg) {
    InternalState state; memset(&state, 0, sizeof(InternalState));
    if (msg->length == 0) return state;
    
    float total_w = 0; int dim = 0;
    for (int i = 0; i < msg->length; i++) {
        Symbol *s = &ce->vocabulary[msg->words[i].symbol_id];
        if (s->feature_dim > dim) dim = s->feature_dim;
        float w = 1.0f;
        if (msg->words[i].role == ROLE_AGENT) w = 2.0f;
        if (msg->words[i].role == ROLE_ACTION) w = 1.5f;
        for (int f = 0; f < s->feature_dim && f < COMM_MAX_FEATURES; f++)
            state.features[f] += s->features[f] * w;
        state.emotional_valence += s->emotional_valence * w;
        total_w += w;
    }
    if (total_w > 0) {
        for (int f = 0; f < dim; f++) state.features[f] /= total_w;
        state.emotional_valence /= total_w;
    }
    state.feature_dim = dim;
    return state;
}

Message comm_describe(CommEngine *ce, const float *pattern, int dim) {
    InternalState s; memset(&s, 0, sizeof(InternalState));
    int d = dim < COMM_MAX_FEATURES ? dim : COMM_MAX_FEATURES;
    memcpy(s.features, pattern, d * sizeof(float));
    s.feature_dim = d;
    return comm_encode(ce, &s);
}

InternalState comm_understand(CommEngine *ce, const char **words, int num_words) {
    Message msg; memset(&msg, 0, sizeof(Message));
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

void comm_print_message(const CommEngine *ce, const Message *msg) {
    printf("Message (confidence=%.2f): \"", msg->confidence);
    for (int i = 0; i < msg->length; i++) {
        printf("%s", ce->vocabulary[msg->words[i].symbol_id].label);
        if (i < msg->length - 1) printf(" ");
    }
    printf("\"\n");
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
        const char *roles[] = {"NONE","AGENT","ACTION","PATIENT","ATTR","LOC","TIME"};
        printf("  [%3d] %-12s role=%-7s valence=%+.1f %s\n",
               i, ce->vocabulary[i].label, roles[ce->vocabulary[i].typical_role],
               ce->vocabulary[i].emotional_valence, ce->vocabulary[i].is_abstract ? "[abstract]" : "");
    }
}
