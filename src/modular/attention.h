/*
 * attention.h — Brain-Inspired Attention System
 * Rethink AI — Phase 12 / V12
 *
 * Models:
 *   - Biased competition (Desimone & Duncan, 1995)
 *   - Dorsal attention (top-down, goal-driven)
 *   - Ventral attention (bottom-up, salience capture)
 *   - Gain modulation (thalamic amplification)
 *   - Limited capacity (finite attention budget)
 */

#ifndef ATTENTION_H
#define ATTENTION_H

#define ATT_MAX_ITEMS    16
#define ATT_MAX_GOALS     4
#define ATT_FEATURE_DIM  32

/* ──── Attention Types ──── */
typedef enum {
    ATT_SPATIAL,
    ATT_FEATURE,
    ATT_TEMPORAL,
    ATT_OBJECT
} AttentionType;

/* ──── Attention Goal (Top-Down Bias) ──── */
typedef struct {
    AttentionType type;
    float target[ATT_FEATURE_DIM];
    int   target_dim;
    float strength;      /* 0..1 bias strength */
    int   active;
    int   ttl;           /* Ticks until goal expires (0 = permanent) */
} AttentionGoal;

/* ──── Competing Item ──── */
typedef struct {
    float features[ATT_FEATURE_DIM];
    int   feature_dim;
    float raw_activation;    /* Bottom-up input strength */
    float salience;          /* How much it stands out */
    float relevance;         /* Goal match score */
    float emotional_boost;   /* Amygdala-driven priority */
    float priority;          /* Combined priority */
    float gain;              /* Current gain modulation */
    int   active;
    char  label[32];
} AttentionItem;

/* ──── The Attention System ──── */
typedef struct {
    /* Items competing for attention */
    AttentionItem items[ATT_MAX_ITEMS];
    int num_items;

    /* Top-down goals */
    AttentionGoal goals[ATT_MAX_GOALS];
    int num_goals;

    /* Winner state */
    int   winner_idx;
    float winner_gain;
    float suppression_gain;

    /* Budget (limited resource) */
    float budget;
    float budget_max;
    float budget_recovery;

    /* Competition parameters */
    float inhibition_strength;
    float salience_weight;
    float relevance_weight;
    float emotion_weight;
    float capture_threshold;

    /* Output */
    float attended_features[ATT_FEATURE_DIM];
    int   attended_dim;
    float total_attention;   /* Replaces old attention_level float */
} AttentionSystem;

/* ──── Lifecycle ──── */
void attention_init(AttentionSystem *as);

/* ──── Goal Management (Dorsal / Top-Down) ──── */
int  attention_set_goal(AttentionSystem *as, AttentionType type,
                        const float *target, int dim, float strength, int ttl);
void attention_clear_goals(AttentionSystem *as);

/* ──── Item Management ──── */
int  attention_add_item(AttentionSystem *as, const float *features, int dim,
                        float activation, float emotional_boost, const char *label);
void attention_clear_items(AttentionSystem *as);

/* ──── Core Processing ──── */
void attention_compute_salience(AttentionSystem *as);
void attention_compute_relevance(AttentionSystem *as);
void attention_compete(AttentionSystem *as);
void attention_modulate(AttentionSystem *as);
void attention_process(AttentionSystem *as);

/* ──── Capture (Ventral / Bottom-Up) ──── */
int  attention_check_capture(AttentionSystem *as);

/* ──── Query ──── */
float attention_get_gain(const AttentionSystem *as, int item_idx);
int   attention_winner(const AttentionSystem *as);
float attention_level(const AttentionSystem *as);

/* ──── Debug ──── */
void attention_print(const AttentionSystem *as);
void attention_print_competition(const AttentionSystem *as);

#endif /* ATTENTION_H */
