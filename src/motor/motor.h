/*
 * motor.h — Motor System (Action Selection, Planning, and Learning)
 *
 * Rethink AI — Phase 13 / V13
 *
 * Brain regions modeled:
 *   - Primary Motor Cortex (M1): action execution
 *   - Premotor Cortex (PMC): contextual action selection
 *   - Supplementary Motor Area (SMA): sequence planning
 *   - Basal Ganglia: GO/NO-GO competition via disinhibition
 *   - Cerebellum: forward model (predict outcomes), error correction
 *   - Substantia Nigra (SNc): dopamine reward prediction error
 *
 * Key principles:
 *   - Actions compete via GO/NO-GO pathways (basal ganglia)
 *   - Reward Prediction Error drives learning (dopamine)
 *   - Forward model predicts outcomes before execution (cerebellum)
 *   - Sequences are planned as ordered steps (SMA)
 *   - Exploration decays as confidence grows (development)
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <string.h>
#include <math.h>

#define MOTOR_MAX_ACTIONS   32
#define MOTOR_MAX_STEPS     8
#define MOTOR_FEATURE_DIM   32
#define MOTOR_HISTORY_SIZE  64

/* ──── Action Primitive ──── */
typedef struct {
    char  name[32];                         /* Human-readable label */
    float features[MOTOR_FEATURE_DIM];      /* Action feature vector */
    float go_weight;                        /* Direct pathway (approach) */
    float nogo_weight;                      /* Indirect pathway (avoid) */
    float reward_avg;                       /* Running reward average */
    float forward_model[MOTOR_FEATURE_DIM]; /* Cerebellum: predict outcome */
    int   execution_count;                  /* How often executed */
    int   active;                           /* Slot in use? */
} MotorAction;

/* ──── Motor Step (part of a sequence) ──── */
typedef struct {
    char  name[32];
    int   action_id;
    float expected_effect;
    float actual_effect;
    int   completed;
} MotorStep;

/* ──── Motor Plan (SMA sequence) ──── */
typedef struct {
    MotorStep steps[MOTOR_MAX_STEPS];
    int   num_steps;
    float expected_reward;
    float actual_reward;
    char  label[64];
    int   current_step;
    int   complete;
} MotorPlan;

/* ──── Action History Entry ──── */
typedef struct {
    int   action_id;
    float reward;
    float state_features[MOTOR_FEATURE_DIM];
    int   tick;
} MotorHistoryEntry;

/* ──── The Motor System ──── */
typedef struct {
    /* Repertoire */
    MotorAction actions[MOTOR_MAX_ACTIONS];
    int num_actions;

    /* Current plan */
    MotorPlan current_plan;

    /* Parameters */
    float confidence;           /* 0=no experience, 1=expert */
    float exploration_rate;     /* Epsilon: prob of random action */
    float learning_rate;        /* Alpha for weight updates */
    float discount;             /* Gamma for future reward */
    float last_reward;          /* Most recent reward */
    float prediction_error;     /* RPE: actual - predicted */

    /* History (ring buffer) */
    MotorHistoryEntry history[MOTOR_HISTORY_SIZE];
    int history_count;
    int history_idx;

    /* Global forward model */
    float global_forward[MOTOR_FEATURE_DIM];
} MotorSystem;

/* ──── Lifecycle ──── */
void motor_init(MotorSystem *ms);

/* ──── Action Management ──── */
int  motor_add_action(MotorSystem *ms, const char *name,
                      const float *features, int dim);
int  motor_find_action(const MotorSystem *ms, const char *name);

/* ──── Selection (Basal Ganglia) ──── */
int   motor_select_action(MotorSystem *ms, const float *state, int dim);
float motor_evaluate_action(const MotorSystem *ms, int action_id,
                            const float *state, int dim);

/* ──── Planning (SMA) ──── */
void motor_plan_action(MotorSystem *ms, const float *state, int dim,
                       float goal_reward, const char *label);

/* ──── Execution (M1 + Cerebellum) ──── */
void motor_execute(MotorSystem *ms);

/* ──── Learning (Dopamine RPE) ──── */
void motor_learn(MotorSystem *ms, float actual_reward);
void motor_update_forward_model(MotorSystem *ms, int action_id,
                                const float *state, int dim, float outcome);

/* ──── Query / Debug ──── */
void motor_print_repertoire(const MotorSystem *ms);
void motor_print_plan(const MotorSystem *ms);

#endif /* MOTOR_H */
