# Phase 13 — Building the Motor System: Header Design

> The data structures and API for action, planning, and learning

---

## Design Principles

1. **Actions are feature vectors** — just like perceptual inputs, actions live in 32-dim space
2. **GO/NO-GO competition** — every action has competing pathways (basal ganglia model)
3. **Forward model** — predict outcome before executing (cerebellum)
4. **Reward learning** — RPE updates weights (dopamine)
5. **Sequence planning** — ordered steps with expected effects (SMA)
6. **No external dependencies** — stack-allocated where possible, same C11 patterns

---

## Header: `motor.h`

```c
#ifndef MOTOR_H
#define MOTOR_H

#define MOTOR_MAX_ACTIONS   32      /* Max actions in repertoire */
#define MOTOR_MAX_STEPS     8       /* Max steps in a sequence  */
#define MOTOR_FEATURE_DIM   32      /* Action feature dimensions */
#define MOTOR_HISTORY_SIZE  64      /* Remember last N actions   */

/* ──── Action Primitive ──── */
typedef struct {
    char name[32];                          /* Human-readable name */
    float features[MOTOR_FEATURE_DIM];      /* Action feature vector */
    float go_weight;                        /* Direct pathway strength (approach) */
    float nogo_weight;                      /* Indirect pathway strength (avoid) */
    float reward_avg;                       /* Running average of rewards received */
    float forward_model[MOTOR_FEATURE_DIM]; /* Cerebellum: predict outcome */
    int   execution_count;                  /* How many times executed */
    int   active;                           /* Is this action slot in use? */
} MotorAction;
```

### Design Decisions

- **go_weight / nogo_weight**: These mimic the direct and indirect pathways of the basal ganglia. The net "desirability" of an action is `go_weight - nogo_weight`.
- **forward_model[]**: Per-action weights that predict reward from state features. This is the cerebellar forward model — it learns to predict what will happen if this action is executed.
- **reward_avg**: Exponential moving average of rewards. Used for exploitation (choose what worked before).

---

```c
/* ──── Motor Plan (Sequence) ──── */
typedef struct {
    char name[32];                  /* Step name */
    int  action_id;                 /* Index into action repertoire */
    float expected_effect;          /* Predicted outcome */
    float actual_effect;            /* What actually happened */
    int  completed;                 /* Has this step been done? */
} MotorStep;

typedef struct {
    MotorStep steps[MOTOR_MAX_STEPS];
    int   num_steps;
    float expected_reward;          /* Sum of expected effects */
    float actual_reward;            /* What we actually got */
    char  label[64];                /* Plan description */
    int   current_step;             /* Which step we're on */
    int   complete;                 /* Is the whole plan done? */
} MotorPlan;
```

### Design Decisions

- **MotorPlan** is the SMA analog — it orders primitive actions into a sequence
- **expected_effect per step** allows the forward model to predict each sub-action independently
- **current_step** tracks progress through the sequence (like a program counter)

---

```c
/* ──── Action History (for temporal learning) ──── */
typedef struct {
    int    action_id;
    float  reward;
    float  state_features[MOTOR_FEATURE_DIM]; /* State when action was taken */
    int    tick;
} MotorHistoryEntry;

/* ──── The Motor System ──── */
typedef struct {
    /* Action repertoire */
    MotorAction actions[MOTOR_MAX_ACTIONS];
    int num_actions;
    
    /* Current plan */
    MotorPlan current_plan;
    
    /* State */
    float confidence;        /* How confident in current actions (0-1) */
    float exploration_rate;  /* Epsilon for explore/exploit (0=greedy, 1=random) */
    float learning_rate;     /* Alpha for weight updates */
    float discount;          /* Gamma for reward prediction */
    float last_reward;       /* Most recent reward signal */
    float prediction_error;  /* RPE: actual - predicted */
    
    /* History */
    MotorHistoryEntry history[MOTOR_HISTORY_SIZE];
    int history_count;
    int history_idx;         /* Ring buffer index */
    
    /* Forward model (global) */
    float global_forward[MOTOR_FEATURE_DIM]; /* General outcome predictor */
    
} MotorSystem;
```

---

## API Design

```c
/* ──── Lifecycle ──── */
void motor_init(MotorSystem *ms);

/* ──── Action Management ──── */
int  motor_add_action(MotorSystem *ms, const char *name,
                      const float *features, int dim);
int  motor_find_action(const MotorSystem *ms, const char *name);

/* ──── Action Selection (Basal Ganglia) ──── */
int  motor_select_action(MotorSystem *ms, const float *state, int dim);
float motor_evaluate_action(const MotorSystem *ms, int action_id,
                            const float *state, int dim);

/* ──── Planning (SMA) ──── */
void motor_plan_action(MotorSystem *ms, const float *state, int dim,
                       float goal_reward, const char *label);
void motor_execute(MotorSystem *ms);

/* ──── Learning (Dopamine + Cerebellum) ──── */
void motor_learn(MotorSystem *ms, float actual_reward);
void motor_update_forward_model(MotorSystem *ms, int action_id,
                                const float *state, int dim, float outcome);

/* ──── Query ──── */
void motor_print_repertoire(const MotorSystem *ms);
void motor_print_plan(const MotorSystem *ms);

#endif /* MOTOR_H */
```

---

## Connection Diagram

```
                    ┌─────────────┐
                    │   Cortex    │ (state features)
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │  Striatum   │
                    │  (GO/NOGO)  │
                    └──┬──────┬───┘
                       │      │
                ┌──────▼──┐ ┌─▼────────┐
                │ Direct  │ │ Indirect │
                │ (GO)    │ │ (NO-GO)  │
                └──────┬──┘ └──┬───────┘
                       │       │
                    ┌──▼───────▼──┐
                    │     GPi     │ (net = GO - NOGO)
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐       ┌─────────────┐
                    │  Thalamus   │───────│ Cerebellum  │
                    │  (execute)  │       │ (predict &  │
                    └──────┬──────┘       │  correct)   │
                           │              └──────┬──────┘
                    ┌──────▼──────┐              │
                    │   Action    │◀─────────────┘
                    │  (output)   │ prediction error
                    └──────┬──────┘
                           │
                    ┌──────▼──────┐
                    │   Reward    │──→ RPE → update GO/NOGO
                    │  (outcome)  │
                    └─────────────┘
```
