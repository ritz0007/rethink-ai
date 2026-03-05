# Phase 13 — Motor System Implementation Walkthrough

> Line-by-line explanation of the motor system code

---

## motor_init — Setting Defaults

```c
void motor_init(MotorSystem *ms) {
    memset(ms, 0, sizeof(MotorSystem));
    ms->exploration_rate = 0.3f;  /* 30% random exploration */
    ms->learning_rate = 0.1f;     /* Moderate learning speed */
    ms->discount = 0.9f;          /* Value future rewards */
    ms->confidence = 0.0f;        /* Start with no confidence */
}
```

**Brain connection**: A newborn has no confidence in its motor system. Exploration rate starts high (babies flail randomly). As actions get rewarded, confidence grows and exploration decreases.

---

## motor_add_action — Building the Repertoire

```c
int motor_add_action(MotorSystem *ms, const char *name,
                     const float *features, int dim) {
    if (ms->num_actions >= MOTOR_MAX_ACTIONS) return -1;
    
    MotorAction *a = &ms->actions[ms->num_actions];
    strncpy(a->name, name, 31);
    int d = dim < MOTOR_FEATURE_DIM ? dim : MOTOR_FEATURE_DIM;
    memcpy(a->features, features, d * sizeof(float));
    a->go_weight = 0.5f;    /* Start neutral */
    a->nogo_weight = 0.5f;  /* Neither approach nor avoid */
    a->reward_avg = 0.0f;
    a->execution_count = 0;
    a->active = 1;
    
    /* Initialize forward model to small random values */
    for (int i = 0; i < MOTOR_FEATURE_DIM; i++)
        a->forward_model[i] = 0.01f * features[i];
    
    return ms->num_actions++;
}
```

**Brain connection**: When you learn a new action (e.g., "throwing"), it starts with neutral GO/NO-GO weights. You don't know yet whether throwing is good or bad. The forward model starts weak — you can barely predict what throwing will do.

---

## motor_evaluate_action — Basal Ganglia Evaluation

```c
float motor_evaluate_action(const MotorSystem *ms, int action_id,
                            const float *state, int dim) {
    const MotorAction *a = &ms->actions[action_id];
    if (!a->active) return -1.0f;
    
    /* GO vs NO-GO competition */
    float net = a->go_weight - a->nogo_weight;
    
    /* Context modulation: how relevant is this action to current state? */
    float relevance = 0.0f;
    int d = dim < MOTOR_FEATURE_DIM ? dim : MOTOR_FEATURE_DIM;
    float norm_s = 0.0f, norm_a = 0.0f, dot = 0.0f;
    for (int i = 0; i < d; i++) {
        dot += state[i] * a->features[i];
        norm_s += state[i] * state[i];
        norm_a += a->features[i] * a->features[i];
    }
    norm_s = sqrtf(norm_s + 1e-8f);
    norm_a = sqrtf(norm_a + 1e-8f);
    relevance = dot / (norm_s * norm_a);
    
    /* Forward model: predicted reward */
    float predicted = 0.0f;
    for (int i = 0; i < d; i++)
        predicted += a->forward_model[i] * state[i];
    
    /* Combined score: GO-NOGO + context + prediction */
    return net * 0.4f + relevance * 0.3f + predicted * 0.3f;
}
```

**Brain connection**: The basal ganglia doesn't just look at GO vs NO-GO — it considers:
1. **Net strength** (disinhibition): direct vs indirect pathway
2. **Contextual relevance** (premotor cortex): is this action appropriate now?
3. **Predicted outcome** (cerebellum): what reward do I expect?

---

## motor_select_action — Epsilon-Greedy Selection

```c
int motor_select_action(MotorSystem *ms, const float *state, int dim) {
    if (ms->num_actions == 0) return -1;
    
    /* Epsilon-greedy: explore or exploit */
    float r = (float)(rand() % 1000) / 1000.0f;
    if (r < ms->exploration_rate) {
        /* Random exploration — try something new */
        return rand() % ms->num_actions;
    }
    
    /* Greedy: choose best action */
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
```

**Brain connection**: The brain balances exploitation (do what worked) with exploration (try something new). Dopamine levels modulate this — high dopamine encourages exploration, low dopamine encourages what's safe.

---

## motor_plan_action — SMA Sequence Planning

```c
void motor_plan_action(MotorSystem *ms, const float *state, int dim,
                       float goal_reward, const char *label) {
    MotorPlan *plan = &ms->current_plan;
    memset(plan, 0, sizeof(MotorPlan));
    if (label) strncpy(plan->label, label, 63);
    
    /* Find or create the action */
    int aid = motor_find_action(ms, label);
    if (aid < 0) {
        aid = motor_add_action(ms, label ? label : "unnamed", state, dim);
    }
    if (aid < 0) return;
    
    /* Build a simple plan: decompose into sub-steps */
    /* Step 1: Prepare (orient toward target) */
    plan->steps[0] = (MotorStep){
        .action_id = aid, .expected_effect = goal_reward * 0.2f
    };
    snprintf(plan->steps[0].name, 31, "prepare_%s", label ? label : "act");
    
    /* Step 2: Execute (main action) */
    plan->steps[1] = (MotorStep){
        .action_id = aid, .expected_effect = goal_reward * 0.6f
    };
    snprintf(plan->steps[1].name, 31, "execute_%s", label ? label : "act");
    
    /* Step 3: Complete (finalize) */
    plan->steps[2] = (MotorStep){
        .action_id = aid, .expected_effect = goal_reward * 0.2f
    };
    snprintf(plan->steps[2].name, 31, "complete_%s", label ? label : "act");
    
    plan->num_steps = 3;
    plan->expected_reward = goal_reward;
    plan->current_step = 0;
    plan->complete = 0;
}
```

**Brain connection**: The SMA breaks complex actions into sub-steps. Even "reach for a cup" decomposes into prepare → reach → grasp. Our simple plan always uses 3 steps (prepare/execute/complete). Future versions could learn variable-length sequences.

---

## motor_execute — Running the Plan

```c
void motor_execute(MotorSystem *ms) {
    MotorPlan *plan = &ms->current_plan;
    if (plan->num_steps == 0) return;
    
    /* Execute all steps in sequence */
    for (int i = 0; i < plan->num_steps; i++) {
        MotorStep *step = &plan->steps[i];
        int aid = step->action_id;
        
        if (aid >= 0 && aid < ms->num_actions) {
            MotorAction *a = &ms->actions[aid];
            
            /* Predict outcome using forward model */
            float predicted = 0.0f;
            for (int j = 0; j < MOTOR_FEATURE_DIM; j++)
                predicted += a->forward_model[j] * a->features[j];
            
            /* Simulate execution — actual effect is step's expected +/- noise */
            float noise = ((float)(rand() % 100) - 50.0f) / 500.0f;
            step->actual_effect = step->expected_effect * 0.8f + noise;
            step->completed = 1;
            
            a->execution_count++;
        }
        plan->current_step = i + 1;
    }
    
    /* Compute total actual reward */
    plan->actual_reward = 0.0f;
    for (int i = 0; i < plan->num_steps; i++)
        plan->actual_reward += plan->steps[i].actual_effect;
    
    plan->complete = 1;
    ms->last_reward = plan->actual_reward;
    
    /* Record in history */
    if (plan->num_steps > 0) {
        int aid = plan->steps[0].action_id;
        if (aid >= 0) {
            MotorHistoryEntry *h = &ms->history[ms->history_idx % MOTOR_HISTORY_SIZE];
            h->action_id = aid;
            h->reward = plan->actual_reward;
            h->tick = ms->history_count;
            memcpy(h->state_features, ms->actions[aid].features,
                   MOTOR_FEATURE_DIM * sizeof(float));
            ms->history_idx = (ms->history_idx + 1) % MOTOR_HISTORY_SIZE;
            if (ms->history_count < MOTOR_HISTORY_SIZE) ms->history_count++;
        }
    }
}
```

---

## motor_learn — Dopamine RPE Update

```c
void motor_learn(MotorSystem *ms, float actual_reward) {
    MotorPlan *plan = &ms->current_plan;
    if (!plan->complete || plan->num_steps == 0) return;
    
    int aid = plan->steps[0].action_id;
    if (aid < 0 || aid >= ms->num_actions) return;
    
    MotorAction *a = &ms->actions[aid];
    
    /* Compute Reward Prediction Error (RPE) */
    ms->prediction_error = actual_reward - a->reward_avg;
    ms->last_reward = actual_reward;
    
    /* Update reward average (exponential moving average) */
    a->reward_avg = a->reward_avg * 0.9f + actual_reward * 0.1f;
    
    /* Dopamine-modulated learning */
    float rpe = ms->prediction_error;
    float lr = ms->learning_rate;
    
    if (rpe > 0) {
        /* Better than expected: strengthen GO, weaken NO-GO */
        a->go_weight += lr * rpe;
        a->nogo_weight -= lr * rpe * 0.5f;
    } else {
        /* Worse than expected: weaken GO, strengthen NO-GO */
        a->go_weight += lr * rpe * 0.5f;  /* rpe is negative */
        a->nogo_weight -= lr * rpe;         /* double negative = increase */
    }
    
    /* Clamp weights */
    if (a->go_weight < 0.0f) a->go_weight = 0.0f;
    if (a->go_weight > 2.0f) a->go_weight = 2.0f;
    if (a->nogo_weight < 0.0f) a->nogo_weight = 0.0f;
    if (a->nogo_weight > 2.0f) a->nogo_weight = 2.0f;
    
    /* Update forward model (cerebellum) */
    for (int i = 0; i < MOTOR_FEATURE_DIM; i++) {
        a->forward_model[i] += lr * rpe * a->features[i];
    }
    
    /* Update overall confidence */
    ms->confidence = ms->confidence * 0.95f + (1.0f - fabsf(rpe)) * 0.05f;
    
    /* Decay exploration as confidence grows */
    ms->exploration_rate = 0.3f * (1.0f - ms->confidence);
    if (ms->exploration_rate < 0.05f) ms->exploration_rate = 0.05f;
}
```

**Brain connection**: This is the core dopamine learning loop:
1. Compute RPE = actual - predicted
2. Positive RPE → dopamine burst → strengthen direct pathway
3. Negative RPE → dopamine dip → strengthen indirect pathway
4. Update the cerebellar forward model
5. As confidence grows, exploration decreases (like a child maturing)

---

## Full Pipeline

```
1. motor_plan_action()    — SMA plans the sequence
2. motor_execute()        — M1 executes and cerebellum predicts
3. motor_learn()          — Dopamine RPE updates GO/NOGO weights
4. Repeat                 — Each cycle, the brain gets better
```

After many cycles, commonly-rewarded actions will have high GO weights and low NO-GO weights, making them the default choice.
