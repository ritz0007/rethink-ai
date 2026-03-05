# 🔨 Build: Tactile Processing Module

## What We're Building

A somatosensory-inspired tactile processor with multiple receptor types, sensitivity weighting (like the sensory homunculus), adaptation dynamics, and feature extraction.

## Brain Region → Code Mapping

| Brain Region | Code Function | What It Does |
|-------------|--------------|-------------|
| Skin receptors | `tactile_sense()` | Multi-channel input (pressure, temp, texture, vibration) |
| Sensory homunculus | `channel_sensitivity[]` | Per-channel importance weighting |
| Spinal cord adaptation | `tactile_adapt()` | Sustained touch fades, onsets amplified |
| S1 | `tactile_extract_features()` | Feature vector output |

## Code: `src/perception/tactile.h`

```c
/*
 * tactile.h — Somatosensory-Inspired Tactile Processing
 * Rethink AI — Phase 11 / V11
 *
 * Models: skin receptors (multi-channel), sensory homunculus (sensitivity),
 *         adaptation (sustained touch fades), S1 (feature extraction)
 */

#ifndef TACTILE_H
#define TACTILE_H

#define TACT_NUM_CHANNELS   8    /* Receptor types */
#define TACT_HISTORY_LEN    8    /* Adaptation history */
#define TACT_FEATURE_DIM    32   /* Output feature dimension */

/* Channel indices — each maps to a receptor type */
typedef enum {
    TACT_PRESSURE = 0,      /* Merkel cells: sustained pressure */
    TACT_LIGHT_TOUCH,       /* Meissner: light/transient touch */
    TACT_VIBRATION,         /* Pacinian: deep vibration */
    TACT_STRETCH,           /* Ruffini: skin stretch */
    TACT_TEMPERATURE,       /* Thermoreceptors: hot/cold (-1=cold, +1=hot) */
    TACT_PAIN,              /* Nociceptors: damage signal (0-1) */
    TACT_TEXTURE_ROUGH,     /* Derived: roughness (0=smooth, 1=rough) */
    TACT_PROPRIOCEPTION     /* Body position sense (angle/position) */
} TactileChannel;

/* ──── Tactile System ──── */
typedef struct {
    /* Current raw readings */
    float channels[TACT_NUM_CHANNELS];
    
    /* Sensitivity per channel (sensory homunculus) */
    float channel_sensitivity[TACT_NUM_CHANNELS];
    
    /* Adapted readings (after habituation) */
    float adapted[TACT_NUM_CHANNELS];
    
    /* History for adaptation */
    float history[TACT_HISTORY_LEN][TACT_NUM_CHANNELS];
    int history_idx;
    int history_count;
    
    /* Onset detection */
    float onsets[TACT_NUM_CHANNELS];
    
    /* Pain fast-path flag (for amygdala) */
    int pain_alert;
    float pain_intensity;
    
    /* Temperature extreme flag */
    int temp_extreme;
    
    /* Output features */
    float features[TACT_FEATURE_DIM];
    int feature_dim;
} TactileSystem;

/* Lifecycle */
void tactile_init(TactileSystem *ts);

/* Set sensitivity for a channel (homunculus weighting) */
void tactile_set_sensitivity(TactileSystem *ts, TactileChannel ch, float sensitivity);

/* Feed raw tactile input */
void tactile_sense(TactileSystem *ts, const float *channel_values);

/* Internal: adaptation (habituation to sustained stimuli) */
void tactile_adapt(TactileSystem *ts);

/* Process: full pipeline (sense → adapt → detect onsets → check alerts) */
void tactile_process(TactileSystem *ts, const float *channel_values);

/* Extract feature vector for downstream modules */
void tactile_extract_features(TactileSystem *ts, float *out_features, int *out_dim);

/* Printing */
void tactile_print_state(const TactileSystem *ts);

#endif /* TACTILE_H */
```

## Code: `src/perception/tactile.c`

```c
#include "tactile.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

void tactile_init(TactileSystem *ts) {
    memset(ts, 0, sizeof(TactileSystem));
    
    /* Default sensitivities — modeled after sensory homunculus
       Fingertip-like sensitivity: pressure and light touch are high
       Pain always maximal sensitivity (survival) */
    ts->channel_sensitivity[TACT_PRESSURE]       = 0.8f;
    ts->channel_sensitivity[TACT_LIGHT_TOUCH]    = 0.9f;
    ts->channel_sensitivity[TACT_VIBRATION]      = 0.6f;
    ts->channel_sensitivity[TACT_STRETCH]        = 0.4f;
    ts->channel_sensitivity[TACT_TEMPERATURE]    = 0.7f;
    ts->channel_sensitivity[TACT_PAIN]           = 1.0f;  /* Pain always max */
    ts->channel_sensitivity[TACT_TEXTURE_ROUGH]  = 0.7f;
    ts->channel_sensitivity[TACT_PROPRIOCEPTION] = 0.5f;
    
    ts->feature_dim = TACT_FEATURE_DIM;
}

void tactile_set_sensitivity(TactileSystem *ts, TactileChannel ch, float sensitivity) {
    if (ch < TACT_NUM_CHANNELS) {
        ts->channel_sensitivity[ch] = fminf(1.0f, fmaxf(0.0f, sensitivity));
    }
}

void tactile_sense(TactileSystem *ts, const float *channel_values) {
    for (int c = 0; c < TACT_NUM_CHANNELS; c++) {
        ts->channels[c] = channel_values[c] * ts->channel_sensitivity[c];
    }
}

/*
 * tactile_adapt — Spinal cord adaptation
 *
 * Sustained stimuli fade (you stop feeling your clothes after a while).
 * Rapid changes are amplified (something new touching you).
 * Pain does NOT adapt — it stays strong as long as the stimulus persists.
 */
void tactile_adapt(TactileSystem *ts) {
    if (ts->history_count == 0) {
        /* First touch — no adaptation yet */
        memcpy(ts->adapted, ts->channels, TACT_NUM_CHANNELS * sizeof(float));
        memset(ts->onsets, 0, TACT_NUM_CHANNELS * sizeof(float));
        return;
    }
    
    /* Compute running average from history (what we're habituated to) */
    float avg[TACT_NUM_CHANNELS];
    memset(avg, 0, sizeof(avg));
    for (int h = 0; h < ts->history_count; h++) {
        for (int c = 0; c < TACT_NUM_CHANNELS; c++) {
            avg[c] += ts->history[h][c];
        }
    }
    for (int c = 0; c < TACT_NUM_CHANNELS; c++) {
        avg[c] /= ts->history_count;
    }
    
    for (int c = 0; c < TACT_NUM_CHANNELS; c++) {
        if (c == TACT_PAIN) {
            /* Pain does NOT habituate — survival mechanism */
            ts->adapted[c] = ts->channels[c];
        } else {
            /* Adapted = current minus habituated baseline */
            float diff = ts->channels[c] - avg[c];
            /* Signed adaptation: positive = stronger than usual, negative = weaker */
            ts->adapted[c] = diff;
        }
        
        /* Onset detection */
        int prev_idx = (ts->history_idx - 1 + TACT_HISTORY_LEN) % TACT_HISTORY_LEN;
        float change = ts->channels[c] - ts->history[prev_idx][c];
        ts->onsets[c] = (change > 0.05f) ? change : 0.0f;
    }
}

void tactile_process(TactileSystem *ts, const float *channel_values) {
    /* Step 1: Sense with sensitivity weighting */
    tactile_sense(ts, channel_values);
    
    /* Step 2: Adaptation */
    tactile_adapt(ts);
    
    /* Step 3: Check fast-path alerts */
    ts->pain_alert = (ts->channels[TACT_PAIN] > 0.5f) ? 1 : 0;
    ts->pain_intensity = ts->channels[TACT_PAIN];
    ts->temp_extreme = (fabsf(ts->channels[TACT_TEMPERATURE]) > 0.8f) ? 1 : 0;
    
    /* Step 4: Store in history */
    memcpy(ts->history[ts->history_idx], ts->channels, 
           TACT_NUM_CHANNELS * sizeof(float));
    ts->history_idx = (ts->history_idx + 1) % TACT_HISTORY_LEN;
    if (ts->history_count < TACT_HISTORY_LEN) ts->history_count++;
}

/*
 * tactile_extract_features — S1 Output
 *
 * Feature vector layout:
 * [0..7]   = raw channel values (sensitivity-weighted)
 * [8..15]  = adapted values (after habituation)
 * [16..23] = onset patterns (what just changed)
 * [24..27] = summary: total pressure, pain level, temp, texture
 * [28..31] = temporal: adaptation depth, stability, onset strength, alert flags
 */
void tactile_extract_features(TactileSystem *ts, float *out_features, int *out_dim) {
    int idx = 0;
    
    /* Raw channels [0..7] */
    for (int c = 0; c < TACT_NUM_CHANNELS && idx < TACT_FEATURE_DIM; c++) {
        out_features[idx++] = ts->channels[c];
    }
    
    /* Adapted channels [8..15] */
    for (int c = 0; c < TACT_NUM_CHANNELS && idx < TACT_FEATURE_DIM; c++) {
        out_features[idx++] = ts->adapted[c];
    }
    
    /* Onset patterns [16..23] */
    for (int c = 0; c < TACT_NUM_CHANNELS && idx < TACT_FEATURE_DIM; c++) {
        out_features[idx++] = ts->onsets[c];
    }
    
    /* Summary stats [24..27] */
    float total_pressure = ts->channels[TACT_PRESSURE] + 
                           ts->channels[TACT_LIGHT_TOUCH] + 
                           ts->channels[TACT_STRETCH];
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = total_pressure / 3.0f;
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = ts->pain_intensity;
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = ts->channels[TACT_TEMPERATURE];
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = ts->channels[TACT_TEXTURE_ROUGH];
    
    /* Temporal context [28..31] */
    float onset_total = 0.0f;
    for (int c = 0; c < TACT_NUM_CHANNELS; c++) onset_total += ts->onsets[c];
    
    float adaptation_depth = 0.0f;
    for (int c = 0; c < TACT_NUM_CHANNELS; c++) {
        adaptation_depth += fabsf(ts->channels[c] - ts->adapted[c]);
    }
    adaptation_depth /= TACT_NUM_CHANNELS;
    
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = adaptation_depth;
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = (float)ts->history_count / TACT_HISTORY_LEN;
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = onset_total;
    if (idx < TACT_FEATURE_DIM) out_features[idx++] = (float)(ts->pain_alert || ts->temp_extreme);
    
    memcpy(ts->features, out_features, idx * sizeof(float));
    *out_dim = idx;
    ts->feature_dim = idx;
}

void tactile_print_state(const TactileSystem *ts) {
    const char *names[] = {"Pressure", "LightTch", "Vibrate ", "Stretch ",
                           "Temp    ", "Pain    ", "Texture ", "Proprio "};
    printf("\n=== Tactile State ===\n");
    for (int c = 0; c < TACT_NUM_CHANNELS; c++) {
        printf("  %s: raw=%+.2f adapted=%+.2f onset=%.2f sens=%.1f",
               names[c], ts->channels[c], ts->adapted[c], 
               ts->onsets[c], ts->channel_sensitivity[c]);
        if (c == TACT_PAIN && ts->pain_alert) printf(" ** PAIN ALERT **");
        if (c == TACT_TEMPERATURE && ts->temp_extreme) printf(" ** EXTREME TEMP **");
        printf("\n");
    }
}
```

## Key Design Decisions

- **8 channels** map to real receptor types — not arbitrary dimensions
- **Sensitivity weighting** is the sensory homunculus: pain always at 1.0, touch high, stretch low
- **Pain never habituates** — this is biologically accurate and connects to amygdala fast-path
- **Adaptation** subtracts the running average: sustained touch fades, novel touch amplified
- **32-dim output** matches other modules for seamless integration

---

*Next: [Build — Multi-Modal Binding](06_build_multimodal.md)*
