# Building Metacognition — Implementation

> Rethink AI — Phase 15, Guide 4 of 5

---

## Implementation Walkthrough

### 1. Initialization — A Brain That Doesn't Know Itself Yet

```c
void meta_init(MetaSystem *ms) {
    memset(ms, 0, sizeof(MetaSystem));

    /* Confidence sources — equally weighted at birth */
    ms->confidence.weights[0] = 0.40f;  // consistency
    ms->confidence.weights[1] = 0.35f;  // prediction
    ms->confidence.weights[2] = 0.25f;  // memory

    /* Strategy selection temperature — high initially (explore) */
    ms->temperature = 1.5f;

    /* Meta-learning */
    ms->base_learning_rate = 0.1f;
    ms->effective_learning_rate = 0.1f;

    /* Self-model — start at 0.5 (uncertain about own abilities) */
    ms->self.perception_accuracy = 0.5f;
    ms->self.memory_reliability = 0.5f;
    ms->self.reasoning_skill = 0.5f;
    ms->self.learning_speed = 0.5f;
    ms->self.social_skill = 0.5f;
    ms->self.motor_skill = 0.5f;
    ms->self.overall_capability = 0.5f;
    ms->self.cognitive_stamina = 1.0f;
    ms->self.meta_accuracy = 0.5f;

    /* Curiosity — moderate at start */
    ms->curiosity = 0.5f;

    /* Default strategies */
    meta_add_strategy(ms, "memory_lookup");   // 0: check memory first
    meta_add_strategy(ms, "causal_reason");   // 1: use causal inference
    meta_add_strategy(ms, "pattern_match");   // 2: use pattern recognition
    meta_add_strategy(ms, "exploration");     // 3: try something new
}
```

**Brain connection**: A newborn has maximum uncertainty about its own capabilities (0.5 everywhere — no idea if it's good or bad). Temperature is high (explore different strategies). Four built-in cognitive strategies mirror how the real prefrontal cortex develops.

---

### 2. Confidence Monitoring — How Sure Am I?

```c
void meta_update_confidence(MetaSystem *ms, float consistency,
                            float prediction_acc, float memory_strength) {
    ms->confidence.consistency = clampf(consistency, 0.0f, 1.0f);
    ms->confidence.prediction = clampf(prediction_acc, 0.0f, 1.0f);
    ms->confidence.memory = clampf(memory_strength, 0.0f, 1.0f);

    /* Weighted sum of confidence sources */
    float raw = ms->confidence.weights[0] * ms->confidence.consistency
              + ms->confidence.weights[1] * ms->confidence.prediction
              + ms->confidence.weights[2] * ms->confidence.memory;

    /* Apply calibration adjustment */
    raw += ms->confidence.adjustment;

    ms->confidence.current = clampf(raw, 0.0f, 1.0f);
}
```

**Brain connection**: The aPFC receives convergent input from multiple systems. Consistency comes from recent history variance. Prediction accuracy from the predictive coding module (V8). Memory strength from the decay memory system (V4). The weighted sum + calibration adjustment ensures confidence tracks actual performance.

### Calibration — Am I Over/Under-Confident?

```c
void meta_calibrate(MetaSystem *ms, float predicted_conf, float actual_outcome) {
    /* Track running calibration error */
    float error = actual_outcome - predicted_conf;

    /* Slowly adjust systematic bias */
    ms->confidence.adjustment += 0.05f * error;
    ms->confidence.adjustment = clampf(ms->confidence.adjustment, -0.3f, 0.3f);

    /* Update ECE (simplified moving average) */
    ms->confidence.calibration_error =
        ms->confidence.calibration_error * 0.95f + fabsf(error) * 0.05f;
}
```

If the brain systematically says "I'm 90% sure" but is only right 60% of the time, the adjustment gradually pulls confidence down. This is exactly how humans become better calibrated with experience.

---

### 3. Error Monitoring — Something Went Wrong

```c
void meta_record_error(MetaSystem *ms, float error_magnitude) {
    error_magnitude = fabsf(error_magnitude);

    /* Store in ring buffer */
    ms->errors.recent_errors[ms->errors.error_idx] = error_magnitude;
    ms->errors.error_idx = (ms->errors.error_idx + 1) % META_ERROR_WINDOW;
    if (ms->errors.error_count < META_ERROR_WINDOW)
        ms->errors.error_count++;

    /* Update baseline (running average) */
    ms->errors.error_baseline =
        ms->errors.error_baseline * 0.9f + error_magnitude * 0.1f;

    /* Track error streak */
    if (error_magnitude > ms->errors.error_baseline * 1.5f)
        ms->errors.error_streak++;
    else
        ms->errors.error_streak = 0;

    /* Update cognitive load */
    float load = 0.0f;
    int count = ms->errors.error_count;
    for (int i = 0; i < count; i++)
        load += ms->errors.recent_errors[i];
    if (count > 0) load /= count;
    ms->errors.cognitive_load = clampf(load, 0.0f, 1.0f);
}
```

**Brain connection**: The ACC fires proportionally to error magnitude. The error streak counter is crucial — one error might be noise, but three consecutive errors strongly signal that the current approach is failing.

```c
int meta_error_detected(const MetaSystem *ms) {
    return ms->errors.error_streak >= 2 ||
           ms->errors.cognitive_load > 0.7f;
}
```

---

### 4. Strategy Selection — How Should I Think?

```c
int meta_select_strategy(MetaSystem *ms) {
    if (ms->num_strategies == 0) return -1;

    /* Compute values for all active strategies */
    for (int i = 0; i < ms->num_strategies; i++) {
        if (!ms->strategies[i].active) continue;
        float sr = ms->strategies[i].success_rate;
        float eff = ms->strategies[i].avg_effort + 0.01f;
        ms->strategies[i].value = sr / eff;
    }

    /* Adapt temperature based on metacognitive state */
    if (meta_error_detected(ms))
        ms->temperature = 1.2f;    // Medium: explore after errors
    else if (ms->confidence.current > 0.7f)
        ms->temperature = 0.5f;    // Low: exploit when confident
    else
        ms->temperature = 1.0f;    // Default

    /* Softmax selection */
    float max_val = -1e9f;
    for (int i = 0; i < ms->num_strategies; i++) {
        if (ms->strategies[i].active && ms->strategies[i].value > max_val)
            max_val = ms->strategies[i].value;
    }

    float sum_exp = 0.0f;
    float exps[META_MAX_STRATEGIES];
    for (int i = 0; i < ms->num_strategies; i++) {
        if (!ms->strategies[i].active) { exps[i] = 0.0f; continue; }
        exps[i] = expf((ms->strategies[i].value - max_val) / ms->temperature);
        sum_exp += exps[i];
    }

    /* Sample from distribution */
    float roll = (float)rand() / (float)RAND_MAX * sum_exp;
    float cum = 0.0f;
    for (int i = 0; i < ms->num_strategies; i++) {
        if (!ms->strategies[i].active) continue;
        cum += exps[i];
        if (roll <= cum) {
            if (ms->current_strategy != i) ms->strategy_switches++;
            ms->current_strategy = i;
            ms->strategies[i].usage_count++;
            return i;
        }
    }

    /* Fallback */
    ms->current_strategy = 0;
    return 0;
}
```

**Brain connection**: The dlPFC selects between cognitive strategies using a value-based softmax. Temperature is controlled by the ACC's error signal and the aPFC's confidence level. This creates the natural pattern: exploit when things are going well, explore when they're not.

---

### 5. Meta-Learning — Adapting the Learning Rate

```c
void meta_adapt_learning(MetaSystem *ms, float surprise, float outcome) {
    /* Surprise factor: surprising events → learn faster */
    float surprise_factor = 1.0f + 0.5f * clampf(surprise, 0.0f, 1.0f);

    /* Uncertainty factor: less confident → learn faster */
    float uncertainty = 1.0f - ms->confidence.current;
    float uncertainty_factor = 0.5f + 0.5f * uncertainty;

    /* Compute effective learning rate */
    ms->effective_learning_rate =
        ms->base_learning_rate * surprise_factor * uncertainty_factor;
    ms->effective_learning_rate =
        clampf(ms->effective_learning_rate, 0.01f, 0.5f);

    /* Track learning trajectory */
    ms->learning_trajectory =
        ms->learning_trajectory * 0.9f + (outcome - 0.5f) * 0.1f;
}
```

When confident and unsurprised → learning rate shrinks to ~50% of base (protect existing knowledge). When uncertain and surprised → learning rate can reach 5× base (rapidly absorb new data). This is learning-to-learn.

---

### 6. Self-Model — Knowing Thyself

```c
void meta_update_self(MetaSystem *ms, const char *domain, float performance) {
    performance = clampf(performance, 0.0f, 1.0f);
    float alpha = 0.1f;

    if      (strcmp(domain, "perception") == 0)
        ms->self.perception_accuracy += alpha * (performance - ms->self.perception_accuracy);
    else if (strcmp(domain, "memory") == 0)
        ms->self.memory_reliability += alpha * (performance - ms->self.memory_reliability);
    else if (strcmp(domain, "reasoning") == 0)
        ms->self.reasoning_skill += alpha * (performance - ms->self.reasoning_skill);
    else if (strcmp(domain, "learning") == 0)
        ms->self.learning_speed += alpha * (performance - ms->self.learning_speed);
    else if (strcmp(domain, "social") == 0)
        ms->self.social_skill += alpha * (performance - ms->self.social_skill);
    else if (strcmp(domain, "motor") == 0)
        ms->self.motor_skill += alpha * (performance - ms->self.motor_skill);

    /* Recompute overall */
    ms->self.overall_capability =
        (ms->self.perception_accuracy + ms->self.memory_reliability +
         ms->self.reasoning_skill + ms->self.learning_speed +
         ms->self.social_skill + ms->self.motor_skill) / 6.0f;
}
```

---

### 7. Introspection — The Metacognitive Loop

```c
void meta_introspect(MetaSystem *ms) {
    ms->introspection_count++;

    /* 1. Compute error variance → consistency confidence */
    float var = 0.0f;
    int n = ms->errors.error_count;
    if (n > 1) {
        float mean = ms->errors.error_baseline;
        for (int i = 0; i < n; i++) {
            float diff = ms->errors.recent_errors[i] - mean;
            var += diff * diff;
        }
        var /= (float)n;
    }
    float consistency = clampf(1.0f - var, 0.0f, 1.0f);

    /* 2. Prediction confidence from recent performance */
    float pred_acc = 0.5f;
    if (ms->history_count > 0) {
        float sum = 0.0f;
        int start = ms->history_count > 10 ? ms->history_count - 10 : 0;
        int count = ms->history_count - start;
        for (int i = start; i < ms->history_count; i++) {
            int idx = i % META_HISTORY_SIZE;
            sum += ms->history[idx].actual_outcome;
        }
        pred_acc = sum / (float)count;
    }

    /* 3. Update confidence */
    meta_update_confidence(ms, consistency, pred_acc, ms->self.memory_reliability);

    /* 4. Check for errors → potentially switch strategy */
    if (meta_error_detected(ms)) {
        meta_select_strategy(ms);
    }

    /* 5. Adapt curiosity based on trajectory */
    if (ms->learning_trajectory > 0.1f)
        ms->curiosity *= 0.95f;      // Doing well → reduce curiosity
    else if (ms->learning_trajectory < -0.1f)
        ms->curiosity = clampf(ms->curiosity + 0.05f, 0.0f, 1.0f); // Declining → explore more

    /* 6. Stamina decay */
    ms->self.cognitive_stamina -= 0.01f;
    if (ms->self.cognitive_stamina < 0.1f)
        ms->self.cognitive_stamina = 0.1f;  // Can't go below 10%
}
```

This function runs the full metacognitive loop: assess consistency → check predictions → update confidence → detect errors → adapt strategies → adjust curiosity → track stamina. It's the brain watching itself.

---

*Next: [05_milestone_meta.md](05_milestone_meta.md) — Verification checklist*
