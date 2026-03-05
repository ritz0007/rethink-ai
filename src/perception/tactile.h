/*
 * tactile.h — Somatosensory Processing Module
 * Rethink AI — Phase 11 / V11
 *
 * Models four receptor types of the skin + pain/temp/proprioception.
 * Applies homunculus-weighted sensitivity and sensory adaptation.
 */

#ifndef TACTILE_H
#define TACTILE_H

#define TACTILE_NUM_CHANNELS 8
#define TACTILE_HISTORY_LEN  8
#define TACTILE_FEATURE_DIM  32

/* Channel indices — each maps to a biological receptor type */
typedef enum {
    TOUCH_PRESSURE     = 0,   /* Merkel cells (sustained pressure)      */
    TOUCH_LIGHT_TOUCH  = 1,   /* Meissner corpuscles (light brush)      */
    TOUCH_VIBRATION    = 2,   /* Pacinian corpuscles (vibration)        */
    TOUCH_STRETCH      = 3,   /* Ruffini endings (skin stretch)         */
    TOUCH_TEMPERATURE  = 4,   /* Thermoreceptors                        */
    TOUCH_PAIN         = 5,   /* Nociceptors                            */
    TOUCH_TEXTURE_ROUGH = 6,  /* Derived: roughness (combo of channels) */
    TOUCH_PROPRIOCEPTION = 7  /* Joint position / muscle tension        */
} TouchChannel;

typedef struct {
    /* Raw and processed values */
    float raw[TACTILE_NUM_CHANNELS];
    float adapted[TACTILE_NUM_CHANNELS];
    float sensitivity[TACTILE_NUM_CHANNELS];

    /* Adaptation accumulators */
    float adaptation_level[TACTILE_NUM_CHANNELS];

    /* Onset detection */
    float onsets[TACTILE_NUM_CHANNELS];

    /* History ring buffer */
    float history[TACTILE_HISTORY_LEN][TACTILE_NUM_CHANNELS];
    int   history_idx;
    int   history_count;

    /* Alerts */
    int   pain_alert;
    int   temp_extreme;

    /* Output features */
    float features[TACTILE_FEATURE_DIM];
    int   feature_dim;
} TactileSystem;

/* Lifecycle */
void tactile_init(TactileSystem *ts);

/* Configure sensitivity per channel (homunculus weighting) */
void tactile_set_sensitivity(TactileSystem *ts, TouchChannel ch, float sens);

/* Raw input */
void tactile_sense(TactileSystem *ts, TouchChannel ch, float value);

/* Full processing pipeline */
void tactile_process(TactileSystem *ts);

/* Extract 32-d feature vector */
void tactile_extract_features(TactileSystem *ts, float *out_features, int *out_dim);

/* Debug */
void tactile_print(const TactileSystem *ts);
void tactile_print_features(const TactileSystem *ts);

#endif /* TACTILE_H */
