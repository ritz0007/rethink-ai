# Phase 12 — Build Guide: `attention.h`

## What We're Building

A principled attention system that replaces the single `attention_level` float with real brain-inspired mechanisms:

1. **Multiple competing items** (biased competition)
2. **Top-down goals** (dorsal attention)
3. **Bottom-up capture** (ventral attention)
4. **Gain modulation** (thalamic amplification/suppression)
5. **Limited budget** (finite attention capacity)
6. **Spotlight, feature, and temporal attention types**

---

## Header: `src/modular/attention.h`

```c
#ifndef ATTENTION_H
#define ATTENTION_H

#define ATT_MAX_ITEMS    16    /* Max items competing for attention */
#define ATT_MAX_GOALS     4    /* Max simultaneous attention goals */
#define ATT_FEATURE_DIM  32    /* Feature vector size for matching */

/* ──── Attention Types ──── */
typedef enum {
    ATT_SPATIAL,      /* Attend to a location */
    ATT_FEATURE,      /* Attend to a feature (color, frequency, etc.) */
    ATT_TEMPORAL,     /* Attend to a time window */
    ATT_OBJECT        /* Attend to a bound object */
} AttentionType;

/* ──── Attention Goal (Top-Down Bias) ──── */
typedef struct {
    AttentionType type;
    float target[ATT_FEATURE_DIM];    /* What to look for */
    int target_dim;
    float strength;                    /* How strongly to bias (0..1) */
    int active;
    int ttl;                           /* Time-to-live in ticks */
} AttentionGoal;

/* ──── Competing Item ──── */
typedef struct {
    float features[ATT_FEATURE_DIM];
    int feature_dim;
    float raw_activation;    /* Bottom-up input strength */
    float salience;          /* Bottom-up attention capture */
    float relevance;         /* Top-down goal match */
    float emotional_boost;   /* Amygdala-driven priority */
    float priority;          /* Combined priority score */
    float gain;              /* Current gain modulation */
    int active;
    char label[32];
} AttentionItem;

/* ──── The Attention System ──── */
typedef struct {
    /* Items competing for attention */
    AttentionItem items[ATT_MAX_ITEMS];
    int num_items;

    /* Top-down goals */
    AttentionGoal goals[ATT_MAX_GOALS];
    int num_goals;

    /* Current winner */
    int   winner_idx;          /* Which item won the competition */
    float winner_gain;         /* Gain applied to winner */
    float suppression_gain;    /* Gain applied to losers */

    /* Budget (limited resource) */
    float budget;              /* Remaining capacity (0..1) */
    float budget_max;          /* Maximum capacity */
    float budget_recovery;     /* Recovery rate per tick */

    /* Parameters */
    float inhibition_strength; /* Lateral inhibition between items */
    float salience_weight;     /* Bottom-up weight in priority */
    float relevance_weight;    /* Top-down weight in priority */
    float emotion_weight;      /* Emotional weight in priority */
    float capture_threshold;   /* Salience needed for automatic capture */

    /* Output: modulated features for the winner */
    float attended_features[ATT_FEATURE_DIM];
    int attended_dim;
    float total_attention;     /* Overall "how focused" (replaces old float) */
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

/* Full cycle: salience → relevance → compete → modulate */
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
```

---

## Key Design Decisions

### Why ATT_MAX_ITEMS = 16?
Working memory holds ~4 items, but the competition might involve more items before the winner is selected. 16 gives room for a rich scene.

### Why a budget?
The brain has finite processing bandwidth. Attending to something costs resources. When the budget is depleted, attention quality degrades.

### Why separate salience/relevance?
They come from different brain systems:
- Salience = bottom-up (ventral attention, automatic)
- Relevance = top-down (dorsal attention, goal-driven)

Combining them into a single priority score mirrors the brain's priority map.

### Why TTL on goals?
Attention goals fade if not refreshed. You can only actively look for something for so long before your attention drifts.

---

## How It Connects

```
Sensory Input → [Retina/Auditory/Tactile]
                         ↓
                  AttentionItems (competing)
                         ↓
                  AttentionGoals (bias from PFC)
                         ↓
                  Competition (lateral inhibition)
                         ↓
                  Gain Modulation (winner amplified)
                         ↓
                  attended_features → Experience Pipeline
```

The attention system sits between perception and the experience pipeline, determining **what gets fully processed**.
