# Phase 12 — Build Guide: `attention.c` Implementation

## The Attention Pipeline

Each tick, the attention system runs this cycle:

```
1. compute_salience()   → how much each item stands out (bottom-up)
2. compute_relevance()  → how much each item matches goals (top-down)
3. compete()            → items suppress each other, winner emerges
4. modulate()           → winner gets gain boost, losers get suppressed
5. check_capture()      → did something grab attention involuntarily?
```

---

## Code: `src/modular/attention.c`

```c
#include "attention.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

void attention_init(AttentionSystem *as) {
    memset(as, 0, sizeof(AttentionSystem));
    as->budget = 1.0f;
    as->budget_max = 1.0f;
    as->budget_recovery = 0.05f;
    as->winner_gain = 2.0f;
    as->suppression_gain = 0.3f;
    as->inhibition_strength = 0.15f;
    as->salience_weight = 0.3f;
    as->relevance_weight = 0.4f;
    as->emotion_weight = 0.3f;
    as->capture_threshold = 0.7f;
    as->winner_idx = -1;
    as->total_attention = 0.5f;
}
```

### Why These Defaults?
- **Relevance weight (0.4)** is highest because goal-driven attention is usually dominant
- **Budget recovery (0.05)** means full recovery takes ~20 ticks of no load
- **Winner gain (2.0)** doubles the winner's signal — strong but not overwhelming
- **Suppression (0.3)** reduces losers to 30% — still processed, but weakly

---

### Goal Management

```c
int attention_set_goal(AttentionSystem *as, AttentionType type,
                       const float *target, int dim, float strength, int ttl) {
    if (as->num_goals >= ATT_MAX_GOALS) return -1;
    
    AttentionGoal *g = &as->goals[as->num_goals];
    g->type = type;
    int copy = dim < ATT_FEATURE_DIM ? dim : ATT_FEATURE_DIM;
    memcpy(g->target, target, copy * sizeof(float));
    g->target_dim = copy;
    g->strength = strength;
    g->ttl = ttl;
    g->active = 1;
    
    return as->num_goals++;
}

void attention_clear_goals(AttentionSystem *as) {
    for (int i = 0; i < ATT_MAX_GOALS; i++) as->goals[i].active = 0;
    as->num_goals = 0;
}
```

### Item Management

```c
int attention_add_item(AttentionSystem *as, const float *features, int dim,
                       float activation, float emotional_boost, const char *label) {
    if (as->num_items >= ATT_MAX_ITEMS) return -1;
    
    AttentionItem *it = &as->items[as->num_items];
    memset(it, 0, sizeof(AttentionItem));
    int copy = dim < ATT_FEATURE_DIM ? dim : ATT_FEATURE_DIM;
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
```

---

### Step 1: Compute Salience (Bottom-Up)

Salience measures how much an item stands out from the background:

```c
void attention_compute_salience(AttentionSystem *as) {
    if (as->num_items == 0) return;
    
    /* Compute average activation as "background" */
    float avg = 0.0f;
    for (int i = 0; i < as->num_items; i++) {
        if (as->items[i].active) avg += as->items[i].raw_activation;
    }
    avg /= as->num_items;
    
    /* Salience = how different from average (pop-out) */
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) { as->items[i].salience = 0; continue; }
        float diff = fabsf(as->items[i].raw_activation - avg);
        /* Also consider feature uniqueness */
        float feature_dist = 0.0f;
        for (int j = 0; j < as->num_items; j++) {
            if (i == j || !as->items[j].active) continue;
            float d = 0.0f;
            int mind = as->items[i].feature_dim < as->items[j].feature_dim
                     ? as->items[i].feature_dim : as->items[j].feature_dim;
            for (int f = 0; f < mind; f++) {
                float fd = as->items[i].features[f] - as->items[j].features[f];
                d += fd * fd;
            }
            feature_dist += sqrtf(d);
        }
        if (as->num_items > 1) feature_dist /= (as->num_items - 1);
        
        as->items[i].salience = 0.5f * diff + 0.5f * fminf(feature_dist, 1.0f);
    }
}
```

### Step 2: Compute Relevance (Top-Down)

Relevance measures how well an item matches the current goal:

```c
void attention_compute_relevance(AttentionSystem *as) {
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) { as->items[i].relevance = 0; continue; }
        
        float max_match = 0.0f;
        for (int g = 0; g < as->num_goals; g++) {
            if (!as->goals[g].active) continue;
            
            /* Cosine similarity between item and goal */
            float dot = 0.0f, mag_i = 0.0f, mag_g = 0.0f;
            int mind = as->items[i].feature_dim < as->goals[g].target_dim
                     ? as->items[i].feature_dim : as->goals[g].target_dim;
            for (int f = 0; f < mind; f++) {
                dot   += as->items[i].features[f] * as->goals[g].target[f];
                mag_i += as->items[i].features[f] * as->items[i].features[f];
                mag_g += as->goals[g].target[f]   * as->goals[g].target[f];
            }
            mag_i = sqrtf(mag_i); mag_g = sqrtf(mag_g);
            float sim = (mag_i > 1e-6f && mag_g > 1e-6f) ? dot / (mag_i * mag_g) : 0.0f;
            sim = (sim + 1.0f) / 2.0f; /* map [-1,1] to [0,1] */
            
            float match = sim * as->goals[g].strength;
            if (match > max_match) max_match = match;
        }
        as->items[i].relevance = max_match;
    }
}
```

### Step 3: Compete (Biased Competition)

```c
void attention_compete(AttentionSystem *as) {
    if (as->num_items == 0) { as->winner_idx = -1; return; }
    
    /* Build priority map */
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) { as->items[i].priority = 0; continue; }
        as->items[i].priority =
            as->salience_weight  * as->items[i].salience +
            as->relevance_weight * as->items[i].relevance +
            as->emotion_weight   * as->items[i].emotional_boost +
            as->items[i].raw_activation * 0.1f; /* small base from raw input */
    }
    
    /* Lateral inhibition: each item suppresses others */
    float suppressed[ATT_MAX_ITEMS];
    for (int i = 0; i < as->num_items; i++) {
        suppressed[i] = as->items[i].priority;
        for (int j = 0; j < as->num_items; j++) {
            if (i == j || !as->items[j].active) continue;
            suppressed[i] -= as->inhibition_strength * as->items[j].priority;
        }
        if (suppressed[i] < 0.0f) suppressed[i] = 0.0f;
    }
    
    /* Find winner (highest post-inhibition priority) */
    int best = -1; float best_pri = -1.0f;
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) continue;
        if (suppressed[i] > best_pri) {
            best_pri = suppressed[i];
            best = i;
        }
    }
    as->winner_idx = best;
}
```

### Step 4: Modulate (Gain Control)

```c
void attention_modulate(AttentionSystem *as) {
    if (as->winner_idx < 0) return;
    
    /* Budget-dependent gain */
    float effective_gain = as->winner_gain * as->budget;
    float effective_supp = as->suppression_gain + (1.0f - as->budget) * 0.3f;
    
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) continue;
        if (i == as->winner_idx) {
            as->items[i].gain = effective_gain;
        } else {
            as->items[i].gain = effective_supp;
        }
    }
    
    /* Copy winner's gain-modulated features to output */
    AttentionItem *w = &as->items[as->winner_idx];
    for (int f = 0; f < w->feature_dim; f++) {
        as->attended_features[f] = w->features[f] * w->gain;
    }
    as->attended_dim = w->feature_dim;
    
    /* Update budget (attending costs resources) */
    as->budget -= 0.02f * as->num_items;
    if (as->budget < 0.1f) as->budget = 0.1f;
    
    /* Update overall attention level */
    as->total_attention = as->budget * (as->winner_idx >= 0 ?
                          as->items[as->winner_idx].priority : 0.5f);
    if (as->total_attention > 1.0f) as->total_attention = 1.0f;
    
    /* Tick down goal TTLs */
    for (int g = 0; g < as->num_goals; g++) {
        if (as->goals[g].active && as->goals[g].ttl > 0) {
            if (--as->goals[g].ttl <= 0) as->goals[g].active = 0;
        }
    }
    
    /* Recover budget slightly */
    as->budget += as->budget_recovery;
    if (as->budget > as->budget_max) as->budget = as->budget_max;
}
```

---

### Full Pipeline

```c
void attention_process(AttentionSystem *as) {
    attention_compute_salience(as);
    attention_compute_relevance(as);
    attention_compete(as);
    attention_modulate(as);
}
```

### Attention Capture (Ventral / Bottom-Up)

```c
int attention_check_capture(AttentionSystem *as) {
    int captured = -1;
    float max_salience = as->capture_threshold;
    
    for (int i = 0; i < as->num_items; i++) {
        if (!as->items[i].active) continue;
        float urgency = as->items[i].salience + as->items[i].emotional_boost;
        if (urgency > max_salience) {
            max_salience = urgency;
            captured = i;
        }
    }
    
    if (captured >= 0 && captured != as->winner_idx) {
        /* Override! Bottom-up capture takes over */
        as->winner_idx = captured;
        attention_modulate(as);
    }
    
    return captured;
}
```

### Query & Print

```c
float attention_get_gain(const AttentionSystem *as, int idx) {
    if (idx < 0 || idx >= as->num_items) return 1.0f;
    return as->items[idx].gain;
}

int attention_winner(const AttentionSystem *as) { return as->winner_idx; }
float attention_level(const AttentionSystem *as) { return as->total_attention; }
```

---

## What This Achieves

| Brain Feature | Our Implementation | Verified By |
|--------------|-------------------|------------|
| Dorsal attention (top-down) | `attention_set_goal()` + relevance | Goal-matching bias |
| Ventral attention (bottom-up) | `attention_check_capture()` | High-salience override |
| Biased competition | `attention_compete()` | Lateral inhibition + bias |
| Gain modulation | `attention_modulate()` | Winner amplified, losers suppressed |
| Limited capacity | `budget` depletion | Performance degrades under load |
| Feature attention | Feature-based goals + cosine matching | Attend-to-color works |
| Priority map | salience + relevance + emotion | Combined bottom-up + top-down |
