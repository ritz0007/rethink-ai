/*
 * tactile.c — Somatosensory Processing Implementation
 * Rethink AI — Phase 11 / V11
 */

#include "tactile.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

static const char *channel_names[TACTILE_NUM_CHANNELS] = {
    "Pressure", "LightTouch", "Vibration", "Stretch",
    "Temp", "Pain", "Roughness", "Proprio"
};

void tactile_init(TactileSystem *ts) {
    memset(ts, 0, sizeof(TactileSystem));

    /* Default sensitivity — homunculus-inspired
       Fingertips (light touch) >> trunk (pressure) */
    ts->sensitivity[TOUCH_PRESSURE]       = 0.6f;
    ts->sensitivity[TOUCH_LIGHT_TOUCH]    = 1.0f;   /* most sensitive */
    ts->sensitivity[TOUCH_VIBRATION]      = 0.8f;
    ts->sensitivity[TOUCH_STRETCH]        = 0.4f;
    ts->sensitivity[TOUCH_TEMPERATURE]    = 0.7f;
    ts->sensitivity[TOUCH_PAIN]           = 1.0f;   /* pain always high */
    ts->sensitivity[TOUCH_TEXTURE_ROUGH]  = 0.7f;
    ts->sensitivity[TOUCH_PROPRIOCEPTION] = 0.5f;

    ts->feature_dim = TACTILE_FEATURE_DIM;
}

void tactile_set_sensitivity(TactileSystem *ts, TouchChannel ch, float sens) {
    if (ch >= 0 && ch < TACTILE_NUM_CHANNELS) {
        ts->sensitivity[ch] = sens;
    }
}

void tactile_sense(TactileSystem *ts, TouchChannel ch, float value) {
    if (ch >= 0 && ch < TACTILE_NUM_CHANNELS) {
        ts->raw[ch] = value;
    }
}

/*
 * tactile_adapt — Sensory adaptation (habituation)
 *
 * Brain truth: repeated same stimulus = reduced response.
 * Exception: PAIN never habituates (nociceptors stay alert).
 * Fast-adapting: light touch, vibration (Meissner, Pacinian)
 * Slow-adapting: pressure, stretch (Merkel, Ruffini)
 */
static void tactile_adapt(TactileSystem *ts) {
    /* Adaptation rates per channel type */
    static const float adapt_rate[TACTILE_NUM_CHANNELS] = {
        0.02f,  /* pressure      — slow adapting */
        0.10f,  /* light touch   — fast adapting */
        0.12f,  /* vibration     — fast adapting */
        0.02f,  /* stretch       — slow adapting */
        0.05f,  /* temperature   — moderate       */
        0.00f,  /* pain          — NEVER adapts   */
        0.08f,  /* roughness     — moderate        */
        0.03f   /* proprioception — slow           */
    };

    for (int ch = 0; ch < TACTILE_NUM_CHANNELS; ch++) {
        float val = ts->raw[ch] * ts->sensitivity[ch];

        /* Build adaptation level toward current input */
        ts->adaptation_level[ch] += adapt_rate[ch] *
            (val - ts->adaptation_level[ch]);

        /* Adapted value = raw - habituated baseline */
        ts->adapted[ch] = val - ts->adaptation_level[ch];
        if (ts->adapted[ch] < 0.0f) ts->adapted[ch] = 0.0f;

        /* Pain override: always pass through raw */
        if (ch == TOUCH_PAIN) {
            ts->adapted[ch] = val;
        }
    }
}

/*
 * tactile_detect_onsets — Detect new touch events
 */
static void tactile_detect_onsets(TactileSystem *ts) {
    if (ts->history_count == 0) {
        for (int ch = 0; ch < TACTILE_NUM_CHANNELS; ch++) {
            ts->onsets[ch] = ts->adapted[ch];
        }
        return;
    }
    int prev = (ts->history_idx - 1 + TACTILE_HISTORY_LEN) % TACTILE_HISTORY_LEN;
    for (int ch = 0; ch < TACTILE_NUM_CHANNELS; ch++) {
        float diff = ts->adapted[ch] - ts->history[prev][ch];
        ts->onsets[ch] = (diff > 0.01f) ? diff : 0.0f;
    }
}

/*
 * tactile_check_alerts — Pain fast-path & temperature extremes
 */
static void tactile_check_alerts(TactileSystem *ts) {
    ts->pain_alert  = (ts->raw[TOUCH_PAIN] > 0.5f) ? 1 : 0;
    ts->temp_extreme = (ts->raw[TOUCH_TEMPERATURE] > 0.8f ||
                        ts->raw[TOUCH_TEMPERATURE] < 0.1f) ? 1 : 0;
}

void tactile_process(TactileSystem *ts) {
    tactile_adapt(ts);
    tactile_detect_onsets(ts);
    tactile_check_alerts(ts);

    /* Store adapted values in history ring buffer */
    memcpy(ts->history[ts->history_idx], ts->adapted,
           TACTILE_NUM_CHANNELS * sizeof(float));
    ts->history_idx = (ts->history_idx + 1) % TACTILE_HISTORY_LEN;
    if (ts->history_count < TACTILE_HISTORY_LEN) ts->history_count++;
}

/*
 * tactile_extract_features — Somatosensory cortex output
 *
 * [0..7]   = raw channel values
 * [8..15]  = adapted values (post habituation)
 * [16..23] = onset pattern
 * [24..27] = summary (total intensity, pain level, adaptation ratio, contact count)
 * [28..31] = temporal context (avg change, stability, history depth, alert flags)
 */
void tactile_extract_features(TactileSystem *ts, float *out_features, int *out_dim) {
    int idx = 0;

    /* Raw [0..7] */
    for (int ch = 0; ch < TACTILE_NUM_CHANNELS && idx < TACTILE_FEATURE_DIM; ch++) {
        out_features[idx++] = ts->raw[ch] * ts->sensitivity[ch];
    }

    /* Adapted [8..15] */
    for (int ch = 0; ch < TACTILE_NUM_CHANNELS && idx < TACTILE_FEATURE_DIM; ch++) {
        out_features[idx++] = ts->adapted[ch];
    }

    /* Onsets [16..23] */
    for (int ch = 0; ch < TACTILE_NUM_CHANNELS && idx < TACTILE_FEATURE_DIM; ch++) {
        out_features[idx++] = ts->onsets[ch];
    }

    /* Summary [24..27] */
    float total_intensity = 0.0f, adaptation_ratio = 0.0f;
    int contact_count = 0;
    float total_raw = 0.0f, total_adapted = 0.0f;

    for (int ch = 0; ch < TACTILE_NUM_CHANNELS; ch++) {
        float r = ts->raw[ch] * ts->sensitivity[ch];
        total_raw += r;
        total_adapted += ts->adapted[ch];
        total_intensity += ts->adapted[ch];
        if (r > 0.05f) contact_count++;
    }
    adaptation_ratio = (total_raw > 1e-6f) ? total_adapted / total_raw : 1.0f;

    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = total_intensity;
    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = ts->adapted[TOUCH_PAIN];
    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = adaptation_ratio;
    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = (float)contact_count / TACTILE_NUM_CHANNELS;

    /* Temporal context [28..31] */
    float avg_change = 0.0f, stability = 0.0f;
    if (ts->history_count >= 2) {
        for (int h = 1; h < ts->history_count; h++) {
            int cur = (ts->history_idx - h + TACTILE_HISTORY_LEN) % TACTILE_HISTORY_LEN;
            int prev = (cur - 1 + TACTILE_HISTORY_LEN) % TACTILE_HISTORY_LEN;
            for (int ch = 0; ch < TACTILE_NUM_CHANNELS; ch++) {
                float d = ts->history[cur][ch] - ts->history[prev][ch];
                avg_change += d * d;
            }
        }
        avg_change = sqrtf(avg_change / (ts->history_count - 1));
        stability = 1.0f / (1.0f + avg_change * 10.0f);
    }

    float alert_flags = 0.0f;
    if (ts->pain_alert) alert_flags += 0.5f;
    if (ts->temp_extreme) alert_flags += 0.5f;

    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = avg_change;
    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = stability;
    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = (float)ts->history_count / TACTILE_HISTORY_LEN;
    if (idx < TACTILE_FEATURE_DIM) out_features[idx++] = alert_flags;

    memcpy(ts->features, out_features, idx * sizeof(float));
    *out_dim = idx;
    ts->feature_dim = idx;
}

void tactile_print(const TactileSystem *ts) {
    printf("\n=== Tactile Channels ===\n");
    for (int ch = 0; ch < TACTILE_NUM_CHANNELS; ch++) {
        float raw = ts->raw[ch] * ts->sensitivity[ch];
        printf("  %-12s  raw=%.3f  adapted=%.3f  onset=%.3f",
               channel_names[ch], raw, ts->adapted[ch], ts->onsets[ch]);
        if (ch == TOUCH_PAIN && ts->pain_alert)
            printf("  *** PAIN ALERT ***");
        if (ch == TOUCH_TEMPERATURE && ts->temp_extreme)
            printf("  *** EXTREME TEMP ***");
        printf("\n");
    }
}

void tactile_print_features(const TactileSystem *ts) {
    printf("Tactile features [%d]: [", ts->feature_dim);
    int limit = ts->feature_dim < 8 ? ts->feature_dim : 8;
    for (int i = 0; i < limit; i++) {
        printf("%.3f", ts->features[i]);
        if (i < limit - 1) printf(", ");
    }
    if (ts->feature_dim > 8) printf(", ...");
    printf("]\n");
}
