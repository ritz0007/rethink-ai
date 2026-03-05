/*
 * multimodal.h — Multi-Modal Sensory Binding Module
 * Rethink AI — Phase 11 / V11
 *
 * Inspired by the Superior Temporal Sulcus (STS) and
 * Superior Colliculus (SC) — where vision, hearing, and touch
 * are bound into a single unified percept.
 *
 * Implements 3 brain binding rules:
 *   1. Spatial Rule  — inputs from same "location" bind
 *   2. Temporal Rule — inputs close in time bind
 *   3. Inverse Effectiveness — weaker inputs benefit more from binding
 *
 * Uses Bayesian precision-weighted combination for fusion.
 */

#ifndef MULTIMODAL_H
#define MULTIMODAL_H

#define MM_MAX_MODALITIES 3
#define MM_MAX_FEATURES   48   /* room for combined output */

typedef enum {
    MODALITY_VISUAL   = 0,
    MODALITY_AUDITORY = 1,
    MODALITY_TACTILE  = 2
} ModalityType;

typedef struct {
    float features[32];        /* Feature vector from one modality     */
    int   feature_dim;         /* Actual dimension used                */
    float confidence;          /* How reliable (0..1)                  */
    float timestamp;           /* When this input arrived              */
    int   active;              /* Is this modality providing input?    */
} ModalityInput;

typedef struct {
    float features[MM_MAX_FEATURES];  /* Bound feature vector         */
    int   feature_dim;                /* Actual dimension used         */
    float binding_strength;           /* How well modalities agree     */
    float total_confidence;           /* Combined multi-modal confidence */
    float cross_modal_correlation;    /* Correlation between modalities */
    int   conflict_detected;          /* Do the senses disagree?       */
    int   num_active;                 /* How many modalities active    */
} BoundPercept;

typedef struct {
    ModalityInput inputs[MM_MAX_MODALITIES];
    float         precision[MM_MAX_MODALITIES];   /* Learned reliability */
    BoundPercept  percept;
    float         current_time;
} MultiModalSystem;

/* Lifecycle */
void multimodal_init(MultiModalSystem *mm);

/* Provide input from one modality */
void multimodal_set_input(MultiModalSystem *mm, ModalityType mod,
                          const float *features, int dim,
                          float confidence, float timestamp);

/* Clear a modality (no current input from it) */
void multimodal_clear_input(MultiModalSystem *mm, ModalityType mod);

/* Bind all active modalities into one percept */
void multimodal_bind(MultiModalSystem *mm);

/* Update precision (learning which modalities to trust) */
void multimodal_update_precision(MultiModalSystem *mm, ModalityType mod,
                                 float success);

/* Debug */
void multimodal_print(const MultiModalSystem *mm);
void multimodal_print_percept(const BoundPercept *bp);

#endif /* MULTIMODAL_H */
