/*
 * comm.h — Grounded Communication Engine
 * Rethink AI — Phase 9
 */

#ifndef COMM_H
#define COMM_H

#define COMM_MAX_SYMBOLS 128
#define COMM_MAX_FEATURES 64
#define COMM_MAX_SENTENCE 32

typedef enum {
    ROLE_NONE = 0,
    ROLE_AGENT,
    ROLE_ACTION,
    ROLE_PATIENT,
    ROLE_ATTRIBUTE,
    ROLE_LOCATION,
    ROLE_TIME
} SymbolRole;

typedef struct {
    int id;
    char label[32];
    float features[COMM_MAX_FEATURES];
    int feature_dim;
    SymbolRole typical_role;
    float emotional_valence;
    float familiarity;
    int is_abstract;
} Symbol;

typedef struct {
    int symbol_id;
    SymbolRole role;
} BoundSymbol;

typedef struct {
    BoundSymbol words[COMM_MAX_SENTENCE];
    int length;
    float confidence;
} Message;

typedef struct {
    float features[COMM_MAX_FEATURES];
    int feature_dim;
    float emotional_valence;
    float arousal;
    float urgency;
} InternalState;

typedef struct {
    Symbol vocabulary[COMM_MAX_SYMBOLS];
    int vocab_size;
    float similarity_threshold;
    float noise_level;
} CommEngine;

CommEngine *comm_create(int feature_dim);
void comm_destroy(CommEngine *ce);

int comm_add_symbol(CommEngine *ce, const char *label, const float *features,
                    int feature_dim, SymbolRole role, float valence, int is_abstract);
int comm_find_symbol(const CommEngine *ce, const char *label);
float comm_symbol_similarity(const Symbol *sym, const float *features, int dim);

Message comm_encode(CommEngine *ce, const InternalState *state);
InternalState comm_decode(CommEngine *ce, const Message *msg);
Message comm_describe(CommEngine *ce, const float *pattern, int dim);
InternalState comm_understand(CommEngine *ce, const char **words, int num_words);

void comm_print_message(const CommEngine *ce, const Message *msg);
void comm_print_state(const InternalState *state);
void comm_print_vocabulary(const CommEngine *ce);

#endif /* COMM_H */
