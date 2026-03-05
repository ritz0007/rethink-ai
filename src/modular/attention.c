/*
 * attention.c — Brain-Inspired Attention System Implementation
 * Rethink AI — Phase 12 / V12
 *
 * Implements biased competition with gain modulation,
 * top-down (dorsal) and bottom-up (ventral) attention,
 * and a limited attention budget.
 */

#include "attention.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* ──── Lifecycle ──── */

void attention_init(AttentionSystem *as) {
    memset(as, 0, sizeof(AttentionSystem));
    as->budget          = 1.0f;
    as->budget_max      = 1.0f;
    as->budget_recovery = 0.05f;
    as->winner_gain      = 2.0f;
    as->suppression_gain = 0.3f;
    as->inhibition_strength = 0.15f;
    as->salience_weight  = 0.3f;
    as->relevance_weight = 0.4f;
    as->emotion_weight   = 0.3f;
    as->capture_threshold = 0.7f;
    as->winner_idx       = -1;
    as->total_attention  = 0.5f;
}

/* ──── Goal Management ──── */

int attention_set_goal(AttentionSystem *as, AttentionType type,
                       const float *target, int dim, float strength, int ttl) {
    /* Try to find an inactive slot first */
    int slot = -1;
    for (int i = 0; i < ATT_MAX_GOALS; i++) {
        if (!as->goals[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        if (as->num_goals >= ATT_MAX_GOALS) return -1;
        slot = as->num_goals;
    }

    AttentionGoal *g = &as->goals[slot];
    g->type = type;
    int copy = dim < ATT_FEATURE_DIM ? dim : ATT_FEATURE_DIM;
    if (target && copy > 0)
        memcpy(g->target, target, copy * sizeof(float));
    g->target_dim = copy;
    g->strength = strength;
    g->ttl = ttl;
    g->active = 1;

    if (slot >= as->num_goals) as->num_goals = slot + 1;
    return slot;
}

void attention_clear_goals(AttentionSystem *as) {
    for (int i = 0; i < ATT_MAX_GOALS; i++) as->goals[i].active = 0;
    as->num_goals = 0;
}

/* ──── Item Management ──── */

int attention_add_item(AttentionSystem *as, const float *features, int dim,
                       float activation, float emotional_boost, const char *label) {
    if (as->num_items >= ATT_MAX_ITEMS) return -1;

    AttentionItem *it = &as->items[as->num_items];
    memset(it, 0, sizeof(AttentionItem));
    int copy = dim < ATT_FEATURE_DIM ? dim : ATT_FEATURE_DIM;
    if (features && copy > 0)
        memcpy(it->features, features, copy * sizeof(float));
    it->feature_dim = copy;
    it->raw_activation = activation;
    it->emotional_boost = emotional_boost;
    it->gain = 1.0f;
    it->active = 1;
    if (label) strncpy(it->label, label, 31);

    return as->num_items++;
}

void attention_clear_items(AttentionSystem *as) {
    as->num_items = 0;
    as->winner_idx = -1;
}

/* ──── Step 1: Salience (Bottom-Up) ──── */

void attention_compute_salience(AttentionSystem *as) {
    if (as->num_items == 0) return;

    /* Average activation = "background" level */
    float avg = 0.0f;
    int count = 0;
    for (int i = 0; i < as->num_items; i++) {
        if (as->items[i].active) {
            avg += as->items[i].raw_activation;
            count++;
        }
    }
    if (count > 0) avg /= count;

    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) { as->items[i].salience = 0; continue; }

        /* Difference from average (pop-out by activation) */
        float diff = fabsf(as->items[i].raw_activation - avg);

        /* Feature uniqueness: average distance to other items */
        float feat_dist = 0.0f;
        int pairs = 0;
        for (int j = 0; j < as->num_items; j++) {
            if (i == j || !as->items[j].active) continue;
            float d = 0.0f;
            int mind = as->items[i].feature_dim < as->items[j].feature_dim
                     ? as->items[i].feature_dim : as->items[j].feature_dim;
            for (int f = 0; f < mind; f++) {
                float fd = as->items[i].features[f] - as->items[j].features[f];
                d += fd * fd;
            }
            feat_dist += sqrtf(d);
            pairs++;
        }
        if (pairs > 0) feat_dist /= pairs;

        as->items[i].salience = 0.5f * diff + 0.5f * fminf(feat_dist, 1.0f);
    }
}

/* ──── Step 2: Relevance (Top-Down) ──── */

void attention_compute_relevance(AttentionSystem *as) {
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) { as->items[i].relevance = 0; continue; }

        float max_match = 0.0f;
        for (int g = 0; g < as->num_goals; g++) {
            if (!as->goals[g].active) continue;

            /* Cosine similarity: item vs goal target */
            float dot = 0.0f, mag_i = 0.0f, mag_g = 0.0f;
            int mind = as->items[i].feature_dim < as->goals[g].target_dim
                     ? as->items[i].feature_dim : as->goals[g].target_dim;
            for (int f = 0; f < mind; f++) {
                dot   += as->items[i].features[f] * as->goals[g].target[f];
                mag_i += as->items[i].features[f] * as->items[i].features[f];
                mag_g += as->goals[g].target[f]   * as->goals[g].target[f];
            }
            mag_i = sqrtf(mag_i);
            mag_g = sqrtf(mag_g);

            float sim = (mag_i > 1e-6f && mag_g > 1e-6f)
                      ? dot / (mag_i * mag_g) : 0.0f;
            sim = (sim + 1.0f) / 2.0f; /* [-1,1] → [0,1] */

            float match = sim * as->goals[g].strength;
            if (match > max_match) max_match = match;
        }
        as->items[i].relevance = max_match;
    }
}

/* ──── Step 3: Compete (Biased Competition) ──── */

void attention_compete(AttentionSystem *as) {
    if (as->num_items == 0) { as->winner_idx = -1; return; }

    /* Build priority map: salience + relevance + emotion + raw */
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) { as->items[i].priority = 0; continue; }
        as->items[i].priority =
            as->salience_weight  * as->items[i].salience +
            as->relevance_weight * as->items[i].relevance +
            as->emotion_weight   * as->items[i].emotional_boost +
            0.1f * as->items[i].raw_activation;
    }

    /* Lateral inhibition */
    float suppressed[ATT_MAX_ITEMS];
    for (int i = 0; i < as->num_items; i++) {
        suppressed[i] = as->items[i].priority;
        if (!as->items[i].active) continue;
        for (int j = 0; j < as->num_items; j++) {
            if (i == j || !as->items[j].active) continue;
            suppressed[i] -= as->inhibition_strength * as->items[j].priority;
        }
        if (suppressed[i] < 0.0f) suppressed[i] = 0.0f;
    }

    /* Winner = highest post-inhibition priority */
    int best = -1;
    float best_pri = -1.0f;
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) continue;
        if (suppressed[i] > best_pri) {
            best_pri = suppressed[i];
            best = i;
        }
    }
    as->winner_idx = best;
}

/* ──── Step 4: Modulate (Gain Control) ──── */

void attention_modulate(AttentionSystem *as) {
    if (as->winner_idx < 0) return;

    float effective_gain = as->winner_gain * as->budget;
    float effective_supp = as->suppression_gain + (1.0f - as->budget) * 0.3f;

    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) continue;
        as->items[i].gain = (i == as->winner_idx)
                          ? effective_gain : effective_supp;
    }

    /* Output: gain-modulated winner features */
    AttentionItem *w = &as->items[as->winner_idx];
    for (int f = 0; f < w->feature_dim; f++) {
        as->attended_features[f] = w->features[f] * w->gain;
    }
    as->attended_dim = w->feature_dim;

    /* Budget cost: more items = more competition = more cost */
    float cost = 0.02f * as->num_items;
    as->budget -= cost;
    if (as->budget < 0.1f) as->budget = 0.1f;

    /* Overall attention level (replaces old simple float) */
    as->total_attention = as->budget *
        (as->winner_idx >= 0 ? fminf(as->items[as->winner_idx].priority + 0.3f, 1.0f)
                             : 0.5f);
    if (as->total_attention > 1.0f) as->total_attention = 1.0f;

    /* Tick down goal TTLs */
    for (int g = 0; g < as->num_goals; g++) {
        if (as->goals[g].active && as->goals[g].ttl > 0) {
            if (--as->goals[g].ttl <= 0)
                as->goals[g].active = 0;
        }
    }

    /* Budget recovery */
    as->budget += as->budget_recovery;
    if (as->budget > as->budget_max) as->budget = as->budget_max;
}

/* ──── Full Pipeline ──── */

void attention_process(AttentionSystem *as) {
    attention_compute_salience(as);
    attention_compute_relevance(as);
    attention_compete(as);
    attention_modulate(as);
}

/* ──── Capture (Ventral / Bottom-Up Override) ──── */

int attention_check_capture(AttentionSystem *as) {
    int captured = -1;
    float max_urgency = as->capture_threshold;

    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) continue;
        float urgency = as->items[i].salience + as->items[i].emotional_boost;
        if (urgency > max_urgency) {
            max_urgency = urgency;
            captured = i;
        }
    }

    if (captured >= 0 && captured != as->winner_idx) {
        as->winner_idx = captured;
        attention_modulate(as);
    }

    return captured;
}

/* ──── Query ──── */

float attention_get_gain(const AttentionSystem *as, int idx) {
    if (idx < 0 || idx >= as->num_items) return 1.0f;
    return as->items[idx].gain;
}

int attention_winner(const AttentionSystem *as) {
    return as->winner_idx;
}

float attention_level(const AttentionSystem *as) {
    return as->total_attention;
}

/* ──── Debug ──── */

void attention_print(const AttentionSystem *as) {
    printf("\n=== Attention System ===\n");
    printf("  Budget: %.2f / %.2f\n", as->budget, as->budget_max);
    printf("  Total attention: %.3f\n", as->total_attention);
    printf("  Items: %d  Goals: %d\n", as->num_items, as->num_goals);
    printf("  Winner: %s (#%d)\n",
           as->winner_idx >= 0 ? as->items[as->winner_idx].label : "none",
           as->winner_idx);

    for (int g = 0; g < as->num_goals; g++) {
        if (!as->goals[g].active) continue;
        printf("  Goal %d: type=%d strength=%.2f ttl=%d\n",
               g, as->goals[g].type, as->goals[g].strength, as->goals[g].ttl);
    }
}

void attention_print_competition(const AttentionSystem *as) {
    printf("\n=== Attention Competition ===\n");
    printf("  %-16s %7s %7s %7s %7s %7s %6s\n",
           "Item", "Raw", "Sal", "Rel", "Emo", "Pri", "Gain");
    printf("  %-16s %7s %7s %7s %7s %7s %6s\n",
           "────", "───", "───", "───", "───", "───", "────");

    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) continue;
        const AttentionItem *it = &as->items[i];
        printf("  %-16s %7.3f %7.3f %7.3f %7.3f %7.3f %6.2f %s\n",
               it->label[0] ? it->label : "(unnamed)",
               it->raw_activation, it->salience, it->relevance,
               it->emotional_boost, it->priority, it->gain,
               i == as->winner_idx ? " ← WINNER" : "");
    }
    printf("  Budget: %.2f  Total attention: %.3f\n",
           as->budget, as->total_attention);
}
