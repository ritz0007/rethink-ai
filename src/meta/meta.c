/*
 * meta.c — Metacognition System Implementation
 *
 * Rethink AI — Phase 15 / V15
 *
 * Brain regions modeled:
 *   - aPFC (BA10): confidence monitoring — multi-source calibrated confidence
 *   - AIC: self-awareness — internal capability model
 *   - dlPFC: strategy selection — softmax with adaptive temperature
 *   - ACC: error/conflict monitoring — streak detection, cognitive load
 *   - vmPFC: strategy value — success/effort ratio
 *
 * Also implements meta-learning (adaptive learning rate) and introspection
 * (the recursive loop that closes the metacognitive cycle).
 */

#include "meta.h"

/* ──── Utility ──── */
static float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

/* ================================================================
 * Lifecycle
 * ================================================================ */

void meta_init(MetaSystem *ms) {
    memset(ms, 0, sizeof(MetaSystem));

    /* Confidence source weights (aPFC weighting of evidence) */
    ms->confidence.weights[0] = 0.40f;  /* consistency */
    ms->confidence.weights[1] = 0.35f;  /* prediction accuracy */
    ms->confidence.weights[2] = 0.25f;  /* memory strength */
    ms->confidence.current = 0.5f;      /* start uncertain */

    /* Strategy selection — high temperature initially (explore) */
    ms->temperature = 1.5f;
    ms->current_strategy = 0;

    /* Meta-learning */
    ms->base_learning_rate = 0.1f;
    ms->effective_learning_rate = 0.1f;
    ms->learning_trajectory = 0.0f;

    /* Self-model — start at 0.5 (uncertain about own abilities) */
    ms->self.perception_accuracy = 0.5f;
    ms->self.memory_reliability  = 0.5f;
    ms->self.reasoning_skill     = 0.5f;
    ms->self.learning_speed      = 0.5f;
    ms->self.social_skill        = 0.5f;
    ms->self.motor_skill         = 0.5f;
    ms->self.overall_capability  = 0.5f;
    ms->self.cognitive_stamina   = 1.0f;
    ms->self.meta_accuracy       = 0.5f;

    /* Curiosity — moderate at start */
    ms->curiosity = 0.5f;

    /* Default cognitive strategies (dlPFC repertoire) */
    meta_add_strategy(ms, "memory_lookup");    /* 0: check memory first */
    meta_add_strategy(ms, "causal_reason");    /* 1: use causal inference */
    meta_add_strategy(ms, "pattern_match");    /* 2: use pattern recognition */
    meta_add_strategy(ms, "exploration");      /* 3: try something new */
}

/* ================================================================
 * Confidence Monitoring (aPFC / BA10)
 * ================================================================ */

float meta_confidence(const MetaSystem *ms) {
    return ms->confidence.current;
}

void meta_update_confidence(MetaSystem *ms, float consistency,
                            float prediction_acc, float memory_strength) {
    ms->confidence.consistency = clampf(consistency, 0.0f, 1.0f);
    ms->confidence.prediction  = clampf(prediction_acc, 0.0f, 1.0f);
    ms->confidence.memory      = clampf(memory_strength, 0.0f, 1.0f);

    /* Weighted sum of confidence sources */
    float raw = ms->confidence.weights[0] * ms->confidence.consistency
              + ms->confidence.weights[1] * ms->confidence.prediction
              + ms->confidence.weights[2] * ms->confidence.memory;

    /* Apply calibration adjustment */
    raw += ms->confidence.adjustment;

    ms->confidence.current = clampf(raw, 0.0f, 1.0f);
}

void meta_calibrate(MetaSystem *ms, float predicted_conf, float actual_outcome) {
    float error = actual_outcome - predicted_conf;

    /* Slowly adjust systematic bias */
    ms->confidence.adjustment += 0.05f * error;
    ms->confidence.adjustment = clampf(ms->confidence.adjustment, -0.3f, 0.3f);

    /* Update ECE (simplified moving average) */
    ms->confidence.calibration_error =
        ms->confidence.calibration_error * 0.95f + fabsf(error) * 0.05f;
}

/* ================================================================
 * Error & Conflict Monitoring (ACC)
 * ================================================================ */

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
    float threshold = ms->errors.error_baseline * 1.5f;
    if (threshold < 0.1f) threshold = 0.1f;  /* minimum threshold */
    if (error_magnitude > threshold)
        ms->errors.error_streak++;
    else
        ms->errors.error_streak = 0;

    /* Update cognitive load from recent errors */
    float load = 0.0f;
    int count = ms->errors.error_count;
    for (int i = 0; i < count; i++)
        load += ms->errors.recent_errors[i];
    if (count > 0) load /= (float)count;
    ms->errors.cognitive_load = clampf(load, 0.0f, 1.0f);
}

int meta_error_detected(const MetaSystem *ms) {
    return ms->errors.error_streak >= 2 ||
           ms->errors.cognitive_load > 0.7f;
}

float meta_cognitive_load(const MetaSystem *ms) {
    return ms->errors.cognitive_load;
}

/* ================================================================
 * Strategy Selection (dlPFC)
 * ================================================================ */

int meta_add_strategy(MetaSystem *ms, const char *name) {
    if (ms->num_strategies >= META_MAX_STRATEGIES) return -1;
    int idx = ms->num_strategies;

    memset(&ms->strategies[idx], 0, sizeof(CognitiveStrategy));
    strncpy(ms->strategies[idx].name, name, 31);
    ms->strategies[idx].name[31] = '\0';
    ms->strategies[idx].success_rate = 0.5f; /* prior: uncertain */
    ms->strategies[idx].avg_effort   = 0.5f;
    ms->strategies[idx].value        = 1.0f;
    ms->strategies[idx].active       = 1;

    ms->num_strategies++;
    return idx;
}

int meta_select_strategy(MetaSystem *ms) {
    if (ms->num_strategies == 0) return -1;

    /* Compute values for all active strategies */
    for (int i = 0; i < ms->num_strategies; i++) {
        if (!ms->strategies[i].active) continue;
        float sr  = ms->strategies[i].success_rate;
        float eff = ms->strategies[i].avg_effort + 0.01f;
        ms->strategies[i].value = sr / eff;
    }

    /* Adapt temperature based on metacognitive state */
    if (meta_error_detected(ms))
        ms->temperature = 1.2f;    /* Medium: explore after errors */
    else if (ms->confidence.current > 0.7f)
        ms->temperature = 0.5f;    /* Low: exploit when confident */
    else
        ms->temperature = 1.0f;    /* Default */

    /* Softmax selection — find max value for numerical stability */
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

    if (sum_exp < 1e-12f) {
        /* Fallback: uniform */
        ms->current_strategy = 0;
        ms->strategies[0].usage_count++;
        return 0;
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
    ms->strategies[0].usage_count++;
    return 0;
}

void meta_update_strategy(MetaSystem *ms, int strategy_id,
                          float outcome, float effort) {
    if (strategy_id < 0 || strategy_id >= ms->num_strategies) return;
    CognitiveStrategy *s = &ms->strategies[strategy_id];
    if (!s->active) return;

    float alpha = 0.15f;
    s->success_rate = s->success_rate + alpha * (clampf(outcome, 0.0f, 1.0f) - s->success_rate);
    s->avg_effort   = s->avg_effort   + alpha * (clampf(effort, 0.0f, 1.0f) - s->avg_effort);
    s->value = s->success_rate / (s->avg_effort + 0.01f);
}

const char *meta_current_strategy_name(const MetaSystem *ms) {
    if (ms->num_strategies == 0) return "none";
    if (ms->current_strategy < 0 || ms->current_strategy >= ms->num_strategies)
        return "none";
    return ms->strategies[ms->current_strategy].name;
}

/* ================================================================
 * Meta-Learning — Adaptive Learning Rate
 * ================================================================ */

float meta_effective_lr(const MetaSystem *ms) {
    return ms->effective_learning_rate;
}

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

/* ================================================================
 * Self-Model (aPFC + AIC)
 * ================================================================ */

void meta_update_self(MetaSystem *ms, const char *domain, float performance) {
    performance = clampf(performance, 0.0f, 1.0f);
    float alpha = 0.1f;
    float *target = NULL;

    if      (strcmp(domain, "perception") == 0) target = &ms->self.perception_accuracy;
    else if (strcmp(domain, "memory") == 0)     target = &ms->self.memory_reliability;
    else if (strcmp(domain, "reasoning") == 0)  target = &ms->self.reasoning_skill;
    else if (strcmp(domain, "learning") == 0)   target = &ms->self.learning_speed;
    else if (strcmp(domain, "social") == 0)     target = &ms->self.social_skill;
    else if (strcmp(domain, "motor") == 0)      target = &ms->self.motor_skill;

    if (target) {
        *target += alpha * (performance - *target);
    }

    /* Recompute overall */
    ms->self.overall_capability =
        (ms->self.perception_accuracy + ms->self.memory_reliability +
         ms->self.reasoning_skill + ms->self.learning_speed +
         ms->self.social_skill + ms->self.motor_skill) / 6.0f;
}

float meta_self_assessment(const MetaSystem *ms, const char *domain) {
    if      (strcmp(domain, "perception") == 0) return ms->self.perception_accuracy;
    else if (strcmp(domain, "memory") == 0)     return ms->self.memory_reliability;
    else if (strcmp(domain, "reasoning") == 0)  return ms->self.reasoning_skill;
    else if (strcmp(domain, "learning") == 0)   return ms->self.learning_speed;
    else if (strcmp(domain, "social") == 0)     return ms->self.social_skill;
    else if (strcmp(domain, "motor") == 0)      return ms->self.motor_skill;
    else if (strcmp(domain, "overall") == 0)    return ms->self.overall_capability;
    return 0.0f;
}

float meta_meta_accuracy(const MetaSystem *ms) {
    /* How well does the self-model match actual performance? */
    /* We use the calibration error as a proxy: lower cal error → higher meta-accuracy */
    float acc = 1.0f - ms->confidence.calibration_error;
    return clampf(acc, 0.0f, 1.0f);
}

/* ================================================================
 * Performance Recording
 * ================================================================ */

void meta_record_performance(MetaSystem *ms, float confidence,
                             float outcome, float effort) {
    PerformanceRecord *rec = &ms->history[ms->history_idx];
    rec->predicted_confidence = clampf(confidence, 0.0f, 1.0f);
    rec->actual_outcome       = clampf(outcome, 0.0f, 1.0f);
    rec->strategy_used        = ms->current_strategy;
    rec->effort               = clampf(effort, 0.0f, 1.0f);

    ms->history_idx = (ms->history_idx + 1) % META_HISTORY_SIZE;
    if (ms->history_count < META_HISTORY_SIZE)
        ms->history_count++;

    /* Calibrate confidence using this observation */
    meta_calibrate(ms, confidence, outcome);

    /* Update current strategy */
    meta_update_strategy(ms, ms->current_strategy, outcome, effort);

    /* Adapt learning rate */
    float surprise = fabsf(outcome - confidence); /* surprised if outcome != expected */
    meta_adapt_learning(ms, surprise, outcome);
}

/* ================================================================
 * Introspection — The Full Metacognitive Loop
 * ================================================================ */

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
        int window = ms->history_count < 10 ? ms->history_count : 10;
        int start_idx = (ms->history_idx - window + META_HISTORY_SIZE) % META_HISTORY_SIZE;
        for (int i = 0; i < window; i++) {
            int idx = (start_idx + i) % META_HISTORY_SIZE;
            sum += ms->history[idx].actual_outcome;
        }
        pred_acc = sum / (float)window;
    }

    /* 3. Update confidence with all sources */
    meta_update_confidence(ms, consistency, pred_acc, ms->self.memory_reliability);

    /* 4. Check for errors → potentially switch strategy */
    if (meta_error_detected(ms)) {
        meta_select_strategy(ms);
    }

    /* 5. Adapt curiosity based on trajectory */
    if (ms->learning_trajectory > 0.1f)
        ms->curiosity = clampf(ms->curiosity * 0.95f, 0.1f, 1.0f);
    else if (ms->learning_trajectory < -0.1f)
        ms->curiosity = clampf(ms->curiosity + 0.05f, 0.0f, 1.0f);

    /* 6. Stamina decay (cognitive fatigue) */
    ms->self.cognitive_stamina -= 0.01f;
    if (ms->self.cognitive_stamina < 0.1f)
        ms->self.cognitive_stamina = 0.1f;

    /* 7. Update meta-accuracy estimate */
    ms->self.meta_accuracy = meta_meta_accuracy(ms);
}

/* ================================================================
 * Printing / Debug
 * ================================================================ */

void meta_print_status(const MetaSystem *ms) {
    printf("  === Metacognitive Status ===\n");
    printf("  Confidence:     %.2f (consist=%.2f, predict=%.2f, memory=%.2f)\n",
           ms->confidence.current,
           ms->confidence.consistency,
           ms->confidence.prediction,
           ms->confidence.memory);
    printf("  Calibration:    ECE=%.3f, adjustment=%.3f\n",
           ms->confidence.calibration_error,
           ms->confidence.adjustment);
    printf("  Error monitor:  baseline=%.3f, streak=%d, load=%.3f, conflict=%.3f\n",
           ms->errors.error_baseline,
           ms->errors.error_streak,
           ms->errors.cognitive_load,
           ms->errors.conflict_level);
    printf("  Strategy:       '%s' (#%d) | temp=%.2f | switches=%d\n",
           meta_current_strategy_name(ms),
           ms->current_strategy,
           ms->temperature,
           ms->strategy_switches);
    printf("  Learning rate:  %.4f (base=%.2f) | trajectory=%+.3f\n",
           ms->effective_learning_rate,
           ms->base_learning_rate,
           ms->learning_trajectory);
    printf("  Curiosity:      %.2f | stamina=%.2f | introspections=%d\n",
           ms->curiosity,
           ms->self.cognitive_stamina,
           ms->introspection_count);
    printf("  Performance:    %d records logged\n", ms->history_count);
}

void meta_print_strategies(const MetaSystem *ms) {
    printf("  === Cognitive Strategies (dlPFC) ===\n");
    printf("  %-4s %-20s %8s %8s %8s %6s\n",
           "#", "Name", "Success", "Effort", "Value", "Used");
    printf("  %-4s %-20s %8s %8s %8s %6s\n",
           "----", "--------------------", "--------", "--------", "--------", "------");
    for (int i = 0; i < ms->num_strategies; i++) {
        if (!ms->strategies[i].active) continue;
        const char *marker = (i == ms->current_strategy) ? " *" : "  ";
        printf("  [%d]%s %-18s %8.3f %8.3f %8.3f %6d\n",
               i, marker,
               ms->strategies[i].name,
               ms->strategies[i].success_rate,
               ms->strategies[i].avg_effort,
               ms->strategies[i].value,
               ms->strategies[i].usage_count);
    }
    printf("  (* = currently active strategy)\n");
    printf("  Temperature: %.2f (low=exploit, high=explore)\n", ms->temperature);
}

void meta_print_self(const MetaSystem *ms) {
    printf("  === Self-Model (aPFC + AIC) ===\n");
    printf("  Perception accuracy: %.2f\n", ms->self.perception_accuracy);
    printf("  Memory reliability:  %.2f\n", ms->self.memory_reliability);
    printf("  Reasoning skill:     %.2f\n", ms->self.reasoning_skill);
    printf("  Learning speed:      %.2f\n", ms->self.learning_speed);
    printf("  Social skill:        %.2f\n", ms->self.social_skill);
    printf("  Motor skill:         %.2f\n", ms->self.motor_skill);
    printf("  ──────────────────────────────\n");
    printf("  Overall capability:  %.2f\n", ms->self.overall_capability);
    printf("  Cognitive stamina:   %.2f\n", ms->self.cognitive_stamina);
    printf("  Meta-accuracy:       %.2f (how well I know my limits)\n",
           ms->self.meta_accuracy);
}
