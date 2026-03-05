/*
 * motor.c — Motor System Implementation
 *
 * Rethink AI — Phase 13 / V13
 *
 * Implements:
 *   - Basal ganglia action selection (GO/NO-GO competition)
 *   - SMA sequence planning (prepare → execute → complete)
 *   - Cerebellum forward model (predict outcomes)
 *   - Dopamine reward prediction error (RPE) learning
 *   - Exploration → exploitation transition
 */

#include "motor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ──── Lifecycle ──── */

void motor_init(MotorSystem *ms) {
    memset(ms, 0, sizeof(MotorSystem));
    ms->exploration_rate = 0.3f;    /* 30% random exploration (infant) */
    ms->learning_rate    = 0.1f;    /* Moderate learning speed */
    ms->discount         = 0.9f;    /* Value future rewards highly */
    ms->confidence       = 0.0f;    /* No experience yet */
}

/* ──── Action Management ──── */

int motor_add_action(MotorSystem *ms, const char *name,
                     const float *features, int dim) {
    if (ms->num_actions >= MOTOR_MAX_ACTIONS) return -1;

    MotorAction *a = &ms->actions[ms->num_actions];
    memset(a, 0, sizeof(MotorAction));
    strncpy(a->name, name, 31);
    a->name[31] = '\0';

    int d = dim < MOTOR_FEATURE_DIM ? dim : MOTOR_FEATURE_DIM;
    memcpy(a->features, features, d * sizeof(float));

    /* Start neutral: neither approach nor avoid */
    a->go_weight   = 0.5f;
    a->nogo_weight = 0.5f;
    a->reward_avg  = 0.0f;
    a->execution_count = 0;
    a->active = 1;

    /* Initialize forward model from features (weak prior) */
    for (int i = 0; i < MOTOR_FEATURE_DIM; i++)
        a->forward_model[i] = 0.01f * a->features[i];

    return ms->num_actions++;
}

int motor_find_action(const MotorSystem *ms, const char *name) {
    for (int i = 0; i < ms->num_actions; i++) {
        if (ms->actions[i].active && strcmp(ms->actions[i].name, name) == 0)
            return i;
    }
    return -1;
}

/* ──── Action Evaluation (Basal Ganglia) ──── */

float motor_evaluate_action(const MotorSystem *ms, int action_id,
                            const float *state, int dim) {
    if (action_id < 0 || action_id >= ms->num_actions) return -999.0f;
    const MotorAction *a = &ms->actions[action_id];
    if (!a->active) return -999.0f;

    int d = dim < MOTOR_FEATURE_DIM ? dim : MOTOR_FEATURE_DIM;

    /* 1. GO vs NO-GO competition (basal ganglia direct/indirect) */
    float net = a->go_weight - a->nogo_weight;

    /* 2. Context relevance (premotor cortex) — cosine similarity */
    float dot = 0.0f, norm_s = 0.0f, norm_a = 0.0f;
    for (int i = 0; i < d; i++) {
        dot    += state[i] * a->features[i];
        norm_s += state[i] * state[i];
        norm_a += a->features[i] * a->features[i];
    }
    norm_s = sqrtf(norm_s + 1e-8f);
    norm_a = sqrtf(norm_a + 1e-8f);
    float relevance = dot / (norm_s * norm_a);

    /* 3. Forward model prediction (cerebellum) */
    float predicted = 0.0f;
    for (int i = 0; i < d; i++)
        predicted += a->forward_model[i] * state[i];

    /* Combined: 40% basal ganglia + 30% context + 30% prediction */
    return net * 0.4f + relevance * 0.3f + predicted * 0.3f;
}

/* ──── Action Selection (Epsilon-Greedy) ──── */

int motor_select_action(MotorSystem *ms, const float *state, int dim) {
    if (ms->num_actions == 0) return -1;

    /* Epsilon-greedy exploration */
    float r = (float)(rand() % 1000) / 1000.0f;
    if (r < ms->exploration_rate) {
        return rand() % ms->num_actions;   /* Random action */
    }

    /* Greedy: evaluate all actions, pick best */
    int best = 0;
    float best_score = -999.0f;
    for (int i = 0; i < ms->num_actions; i++) {
        float score = motor_evaluate_action(ms, i, state, dim);
        if (score > best_score) {
            best_score = score;
            best = i;
        }
    }
    return best;
}

/* ──── Planning (SMA) ──── */

void motor_plan_action(MotorSystem *ms, const float *state, int dim,
                       float goal_reward, const char *label) {
    MotorPlan *plan = &ms->current_plan;
    memset(plan, 0, sizeof(MotorPlan));
    if (label) strncpy(plan->label, label, 63);
    plan->label[63] = '\0';

    /* Find or create action */
    int aid = motor_find_action(ms, label ? label : "unnamed");
    if (aid < 0) {
        aid = motor_add_action(ms, label ? label : "unnamed", state, dim);
    }
    if (aid < 0) return; /* Repertoire full */

    /* SMA: decompose into 3 sub-steps (prepare → execute → complete) */

    /* Step 1: Prepare — orient, ready posture */
    MotorStep *s0 = &plan->steps[0];
    s0->action_id = aid;
    s0->expected_effect = goal_reward * 0.2f;
    s0->completed = 0;
    snprintf(s0->name, 31, "prepare");

    /* Step 2: Execute — main motor command */
    MotorStep *s1 = &plan->steps[1];
    s1->action_id = aid;
    s1->expected_effect = goal_reward * 0.6f;
    s1->completed = 0;
    snprintf(s1->name, 31, "execute");

    /* Step 3: Complete — stabilize, release */
    MotorStep *s2 = &plan->steps[2];
    s2->action_id = aid;
    s2->expected_effect = goal_reward * 0.2f;
    s2->completed = 0;
    snprintf(s2->name, 31, "complete");

    plan->num_steps = 3;
    plan->expected_reward = goal_reward;
    plan->current_step = 0;
    plan->complete = 0;
}

/* ──── Execution (M1 + Cerebellum) ──── */

void motor_execute(MotorSystem *ms) {
    MotorPlan *plan = &ms->current_plan;
    if (plan->num_steps == 0) return;

    for (int i = 0; i < plan->num_steps; i++) {
        MotorStep *step = &plan->steps[i];
        int aid = step->action_id;

        if (aid >= 0 && aid < ms->num_actions) {
            MotorAction *a = &ms->actions[aid];

            /* Cerebellum: forward model predicts outcome */
            float predicted = 0.0f;
            for (int j = 0; j < MOTOR_FEATURE_DIM; j++)
                predicted += a->forward_model[j] * a->features[j];
            (void)predicted; /* Used conceptually; actual effect is simulated */

            /* Execute with motor noise (biologically realistic) */
            float noise = ((float)(rand() % 100) - 50.0f) / 500.0f;
            step->actual_effect = step->expected_effect * 0.8f + noise;
            step->completed = 1;

            a->execution_count++;
        }
        plan->current_step = i + 1;
    }

    /* Total actual reward = sum of step effects */
    plan->actual_reward = 0.0f;
    for (int i = 0; i < plan->num_steps; i++)
        plan->actual_reward += plan->steps[i].actual_effect;

    plan->complete = 1;
    ms->last_reward = plan->actual_reward;

    /* Record in history ring buffer */
    if (plan->num_steps > 0 && plan->steps[0].action_id >= 0) {
        int aid = plan->steps[0].action_id;
        MotorHistoryEntry *h = &ms->history[ms->history_idx];
        h->action_id = aid;
        h->reward = plan->actual_reward;
        h->tick = ms->history_count;
        memcpy(h->state_features, ms->actions[aid].features,
               MOTOR_FEATURE_DIM * sizeof(float));
        ms->history_idx = (ms->history_idx + 1) % MOTOR_HISTORY_SIZE;
        if (ms->history_count < MOTOR_HISTORY_SIZE) ms->history_count++;
    }
}

/* ──── Learning (Dopamine RPE) ──── */

void motor_learn(MotorSystem *ms, float actual_reward) {
    MotorPlan *plan = &ms->current_plan;
    if (!plan->complete || plan->num_steps == 0) return;

    int aid = plan->steps[0].action_id;
    if (aid < 0 || aid >= ms->num_actions) return;

    MotorAction *a = &ms->actions[aid];

    /* Reward Prediction Error: δ = actual - predicted */
    ms->prediction_error = actual_reward - a->reward_avg;
    ms->last_reward = actual_reward;

    /* Update running reward average (exponential moving average) */
    a->reward_avg = a->reward_avg * 0.9f + actual_reward * 0.1f;

    float rpe = ms->prediction_error;
    float lr  = ms->learning_rate;

    /* Dopamine-modulated GO/NO-GO update */
    if (rpe > 0.0f) {
        /* Better than expected → dopamine burst → strengthen GO, weaken NOGO */
        a->go_weight   += lr * rpe;
        a->nogo_weight -= lr * rpe * 0.5f;
    } else {
        /* Worse than expected → dopamine dip → weaken GO, strengthen NOGO */
        a->go_weight   += lr * rpe * 0.5f;    /* rpe < 0, so this weakens */
        a->nogo_weight -= lr * rpe;             /* double neg = strengthen */
    }

    /* Clamp to biologically reasonable range */
    if (a->go_weight   < 0.0f) a->go_weight   = 0.0f;
    if (a->go_weight   > 2.0f) a->go_weight   = 2.0f;
    if (a->nogo_weight < 0.0f) a->nogo_weight = 0.0f;
    if (a->nogo_weight > 2.0f) a->nogo_weight = 2.0f;

    /* Update cerebellar forward model */
    for (int i = 0; i < MOTOR_FEATURE_DIM; i++) {
        a->forward_model[i] += lr * rpe * a->features[i];
    }

    /* Update confidence: less error → more confident */
    ms->confidence = ms->confidence * 0.95f + (1.0f - fabsf(rpe)) * 0.05f;
    if (ms->confidence < 0.0f) ms->confidence = 0.0f;
    if (ms->confidence > 1.0f) ms->confidence = 1.0f;

    /* Exploration decays as confidence grows (development) */
    ms->exploration_rate = 0.3f * (1.0f - ms->confidence);
    if (ms->exploration_rate < 0.05f) ms->exploration_rate = 0.05f;
}

void motor_update_forward_model(MotorSystem *ms, int action_id,
                                const float *state, int dim, float outcome) {
    if (action_id < 0 || action_id >= ms->num_actions) return;
    MotorAction *a = &ms->actions[action_id];
    int d = dim < MOTOR_FEATURE_DIM ? dim : MOTOR_FEATURE_DIM;

    /* Predict */
    float predicted = 0.0f;
    for (int i = 0; i < d; i++)
        predicted += a->forward_model[i] * state[i];

    /* Error */
    float error = outcome - predicted;

    /* Update weights */
    for (int i = 0; i < d; i++)
        a->forward_model[i] += ms->learning_rate * error * state[i];
}

/* ──── Debug / Print ──── */

void motor_print_repertoire(const MotorSystem *ms) {
    if (ms->num_actions == 0) {
        printf("  (No actions learned yet)\n");
        return;
    }
    printf("  %-20s  GO     NOGO   NET    Reward  Runs\n", "Action");
    printf("  %-20s  -----  -----  -----  ------  ----\n", "------");
    for (int i = 0; i < ms->num_actions; i++) {
        const MotorAction *a = &ms->actions[i];
        if (!a->active) continue;
        float net = a->go_weight - a->nogo_weight;
        printf("  %-20s  %.3f  %.3f  %+.3f  %.3f   %d\n",
               a->name, a->go_weight, a->nogo_weight, net,
               a->reward_avg, a->execution_count);
    }
    printf("\n  Confidence: %.3f  Exploration: %.3f\n",
           ms->confidence, ms->exploration_rate);
}

void motor_print_plan(const MotorSystem *ms) {
    const MotorPlan *p = &ms->current_plan;
    if (p->num_steps == 0) {
        printf("  (No plan active)\n");
        return;
    }
    printf("  Plan: '%s' (%d steps)\n", p->label, p->num_steps);
    printf("  Expected reward: %.3f  Actual: %.3f\n",
           p->expected_reward, p->actual_reward);
    for (int i = 0; i < p->num_steps; i++) {
        const MotorStep *s = &p->steps[i];
        printf("  [%d] %-20s  expected=%.3f  actual=%.3f  %s\n",
               i, s->name, s->expected_effect, s->actual_effect,
               s->completed ? "DONE" : "pending");
    }
    printf("  Status: %s\n", p->complete ? "COMPLETE" : "IN PROGRESS");
}
