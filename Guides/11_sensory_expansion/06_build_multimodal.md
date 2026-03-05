# 🔨 Build: Multi-Modal Binding Module

## What We're Building

The superior temporal sulcus / superior colliculus equivalent — a module that takes feature vectors from multiple senses (visual, auditory, tactile) and fuses them into a single unified percept, following the brain's three binding rules: spatial, temporal, and inverse effectiveness.

## Brain Region → Code Mapping

| Brain Region | Code Function | What It Does |
|-------------|--------------|-------------|
| Superior Colliculus | `multimodal_bind()` | Fuse aligned modalities |
| STS | Cross-modal correlation | Detect relationships between senses |
| Thalamus gating | Precision weighting | Unreliable senses contribute less |
| Bayesian integration | `multimodal_precision_weight()` | Optimal combination |

## Code: `src/perception/multimodal.h`

```c
/*
 * multimodal.h — Multi-Sensory Binding
 * Rethink AI — Phase 11 / V11
 *
 * Models: superior temporal sulcus (cross-modal binding),
 *         superior colliculus (spatial alignment),
 *         Bayesian precision weighting (optimal combination)
 *
 * Implements the three binding rules:
 *   1. Spatial rule: same-location signals enhance each other
 *   2. Temporal rule: same-time signals bind together
 *   3. Inverse effectiveness: weak signals benefit most from binding
 */

#ifndef MULTIMODAL_H
#define MULTIMODAL_H

#define MM_MAX_MODALITIES  4
#define MM_FEATURE_DIM     32
#define MM_BOUND_DIM       48   /* Unified output: combined + cross-modal */

typedef enum {
    MODALITY_VISUAL = 0,
    MODALITY_AUDITORY,
    MODALITY_TACTILE,
    MODALITY_COUNT
} ModalityType;

/* Per-modality input */
typedef struct {
    float features[MM_FEATURE_DIM];
    int dim;
    float confidence;     /* 0-1: how reliable this sense is right now */
    float timestamp;      /* When this input was generated */
    int active;           /* Whether this modality has data */
} ModalityInput;

/* Binding result */
typedef struct {
    float features[MM_BOUND_DIM];   /* Unified feature vector */
    int dim;
    
    float binding_strength;         /* How strongly modalities agree */
    float total_confidence;         /* Combined confidence (super-additive) */
    float cross_modal_correlation;  /* Agreement between modalities */
    
    int num_modalities_bound;       /* How many senses contributed */
    int conflict_detected;          /* Modalities disagree significantly */
} BoundPercept;

/* ──── Multi-Modal System ──── */
typedef struct {
    ModalityInput inputs[MODALITY_COUNT];
    
    /* Precision (inverse variance) per modality — learned over time */
    float precision[MODALITY_COUNT];
    
    /* Temporal binding window (ms) */
    float temporal_window;
    
    /* Last bound percept */
    BoundPercept last_percept;
} MultiModalSystem;

/* Lifecycle */
void multimodal_init(MultiModalSystem *mm);

/* Feed a single modality's features */
void multimodal_set_input(MultiModalSystem *mm, ModalityType type,
                          const float *features, int dim, 
                          float confidence, float timestamp);

/* Clear a modality (sense not available) */
void multimodal_clear_input(MultiModalSystem *mm, ModalityType type);

/* Bind all active modalities into a unified percept */
BoundPercept multimodal_bind(MultiModalSystem *mm);

/* Update precision estimates based on prediction accuracy */
void multimodal_update_precision(MultiModalSystem *mm, ModalityType type, 
                                  float prediction_error);

/* Printing */
void multimodal_print_percept(const BoundPercept *bp);
void multimodal_print_state(const MultiModalSystem *mm);

#endif /* MULTIMODAL_H */
```

## Code: `src/perception/multimodal.c`

```c
#include "multimodal.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

void multimodal_init(MultiModalSystem *mm) {
    memset(mm, 0, sizeof(MultiModalSystem));
    
    /* Initial precision estimates (equal trust in all senses) */
    for (int m = 0; m < MODALITY_COUNT; m++) {
        mm->precision[m] = 1.0f;
    }
    
    /* Temporal binding window: 100ms (biological value) */
    mm->temporal_window = 100.0f;
}

void multimodal_set_input(MultiModalSystem *mm, ModalityType type,
                          const float *features, int dim, 
                          float confidence, float timestamp) {
    if (type >= MODALITY_COUNT) return;
    ModalityInput *inp = &mm->inputs[type];
    int copy = dim < MM_FEATURE_DIM ? dim : MM_FEATURE_DIM;
    memcpy(inp->features, features, copy * sizeof(float));
    inp->dim = copy;
    inp->confidence = confidence;
    inp->timestamp = timestamp;
    inp->active = 1;
}

void multimodal_clear_input(MultiModalSystem *mm, ModalityType type) {
    if (type < MODALITY_COUNT) {
        mm->inputs[type].active = 0;
    }
}

/*
 * Compute cross-modal correlation between two modalities.
 * Measures how similar the feature patterns are — high correlation
 * suggests they're about the same object.
 */
static float cross_modal_corr(const ModalityInput *a, const ModalityInput *b) {
    int dim = a->dim < b->dim ? a->dim : b->dim;
    if (dim <= 0) return 0.0f;
    
    /* Cosine similarity of overlapping dimensions */
    float dot = 0, ma = 0, mb = 0;
    for (int i = 0; i < dim; i++) {
        dot += a->features[i] * b->features[i];
        ma += a->features[i] * a->features[i];
        mb += b->features[i] * b->features[i];
    }
    ma = sqrtf(ma); mb = sqrtf(mb);
    if (ma < 1e-6f || mb < 1e-6f) return 0.0f;
    return dot / (ma * mb);
}

/*
 * multimodal_bind — Superior Temporal Sulcus + Superior Colliculus
 *
 * Combines active modalities using precision-weighted averaging,
 * applies the three binding rules, and detects conflicts.
 */
BoundPercept multimodal_bind(MultiModalSystem *mm) {
    BoundPercept bp;
    memset(&bp, 0, sizeof(BoundPercept));
    
    /* Count active modalities and check temporal alignment */
    int active[MODALITY_COUNT];
    int num_active = 0;
    float ref_time = 0;
    
    for (int m = 0; m < MODALITY_COUNT; m++) {
        if (mm->inputs[m].active) {
            if (num_active == 0) ref_time = mm->inputs[m].timestamp;
            active[num_active++] = m;
        }
    }
    
    if (num_active == 0) {
        bp.num_modalities_bound = 0;
        return bp;
    }
    
    if (num_active == 1) {
        /* Single modality — no binding needed, just pass through */
        int m = active[0];
        int dim = mm->inputs[m].dim < MM_BOUND_DIM ? mm->inputs[m].dim : MM_BOUND_DIM;
        memcpy(bp.features, mm->inputs[m].features, dim * sizeof(float));
        bp.dim = dim;
        bp.binding_strength = 0.0f;
        bp.total_confidence = mm->inputs[m].confidence;
        bp.num_modalities_bound = 1;
        mm->last_percept = bp;
        return bp;
    }
    
    /* ── Rule 1: Temporal binding ── */
    /* Check if modalities are within the temporal window */
    float temporal_agreement = 1.0f;
    for (int i = 0; i < num_active; i++) {
        float dt = fabsf(mm->inputs[active[i]].timestamp - ref_time);
        if (dt > mm->temporal_window) {
            temporal_agreement *= 0.5f; /* Reduce binding strength */
        }
    }
    
    /* ── Rule 2: Compute cross-modal correlations ── */
    float avg_corr = 0.0f;
    int corr_count = 0;
    for (int i = 0; i < num_active; i++) {
        for (int j = i + 1; j < num_active; j++) {
            avg_corr += cross_modal_corr(&mm->inputs[active[i]], 
                                          &mm->inputs[active[j]]);
            corr_count++;
        }
    }
    if (corr_count > 0) avg_corr /= corr_count;
    bp.cross_modal_correlation = avg_corr;
    
    /* Conflict detection: very negative correlation = disagreement */
    bp.conflict_detected = (avg_corr < -0.2f) ? 1 : 0;
    
    /* ── Rule 3: Inverse effectiveness + Precision weighting ── */
    /* Bayesian combination: weight each modality by precision * confidence */
    float weights[MODALITY_COUNT];
    float total_weight = 0.0f;
    
    for (int i = 0; i < num_active; i++) {
        int m = active[i];
        float conf = mm->inputs[m].confidence;
        float prec = mm->precision[m];
        
        /* Inverse effectiveness: boost low-confidence signals more 
           when other senses are available */
        float inv_eff = 1.0f;
        if (conf < 0.5f && num_active > 1) {
            inv_eff = 1.0f + (0.5f - conf) * 0.5f; /* Up to 1.25x boost */
        }
        
        weights[i] = prec * conf * inv_eff;
        total_weight += weights[i];
    }
    
    /* Normalize weights */
    if (total_weight > 1e-6f) {
        for (int i = 0; i < num_active; i++) weights[i] /= total_weight;
    }
    
    /* ── Precision-weighted feature combination ── */
    /* Part 1: Weighted average of modality features [0..31] */
    int max_dim = 0;
    for (int i = 0; i < num_active; i++) {
        if (mm->inputs[active[i]].dim > max_dim)
            max_dim = mm->inputs[active[i]].dim;
    }
    if (max_dim > MM_FEATURE_DIM) max_dim = MM_FEATURE_DIM;
    
    for (int f = 0; f < max_dim; f++) {
        float val = 0.0f;
        for (int i = 0; i < num_active; i++) {
            int m = active[i];
            if (f < mm->inputs[m].dim) {
                val += mm->inputs[m].features[f] * weights[i];
            }
        }
        bp.features[f] = val;
    }
    int idx = max_dim;
    
    /* Part 2: Cross-modal features [32..47] */
    /* Per-modality energy (how much each sense contributed) */
    for (int i = 0; i < MODALITY_COUNT && idx < MM_BOUND_DIM; i++) {
        float energy = 0;
        if (mm->inputs[i].active) {
            for (int f = 0; f < mm->inputs[i].dim; f++)
                energy += mm->inputs[i].features[f] * mm->inputs[i].features[f];
            energy = sqrtf(energy);
        }
        bp.features[idx++] = energy;
    }
    
    /* Cross-modal correlations as features */
    for (int i = 0; i < num_active && idx < MM_BOUND_DIM; i++) {
        for (int j = i + 1; j < num_active && idx < MM_BOUND_DIM; j++) {
            bp.features[idx++] = cross_modal_corr(&mm->inputs[active[i]],
                                                   &mm->inputs[active[j]]);
        }
    }
    
    /* Summary features */
    if (idx < MM_BOUND_DIM) bp.features[idx++] = avg_corr;
    if (idx < MM_BOUND_DIM) bp.features[idx++] = temporal_agreement;
    if (idx < MM_BOUND_DIM) bp.features[idx++] = (float)num_active / MODALITY_COUNT;
    if (idx < MM_BOUND_DIM) bp.features[idx++] = (float)bp.conflict_detected;
    
    /* Pad remaining to zero */
    while (idx < MM_BOUND_DIM) bp.features[idx++] = 0.0f;
    bp.dim = idx;
    
    /* ── Compute binding strength ── */
    bp.binding_strength = temporal_agreement * (avg_corr * 0.5f + 0.5f);
    
    /* ── Super-additive confidence ── */
    /* Multiple aligned senses produce higher confidence than any individual */
    float max_conf = 0;
    float sum_conf = 0;
    for (int i = 0; i < num_active; i++) {
        int m = active[i];
        if (mm->inputs[m].confidence > max_conf) max_conf = mm->inputs[m].confidence;
        sum_conf += mm->inputs[m].confidence;
    }
    /* Super-additive: combined > max, if binding is strong */
    float bonus = bp.binding_strength * 0.3f * (sum_conf - max_conf);
    bp.total_confidence = fminf(1.0f, max_conf + bonus);
    
    bp.num_modalities_bound = num_active;
    mm->last_percept = bp;
    return bp;
}

void multimodal_update_precision(MultiModalSystem *mm, ModalityType type,
                                  float prediction_error) {
    if (type >= MODALITY_COUNT) return;
    /* Precision = inverse of variance. Low error → high precision. */
    float alpha = 0.1f;
    float new_prec = 1.0f / (1.0f + prediction_error * prediction_error);
    mm->precision[type] = mm->precision[type] * (1.0f - alpha) + new_prec * alpha;
}

void multimodal_print_percept(const BoundPercept *bp) {
    printf("\n=== Bound Percept ===\n");
    printf("  Modalities bound: %d\n", bp->num_modalities_bound);
    printf("  Binding strength: %.3f\n", bp->binding_strength);
    printf("  Total confidence: %.3f\n", bp->total_confidence);
    printf("  Cross-modal corr: %.3f\n", bp->cross_modal_correlation);
    printf("  Conflict:         %s\n", bp->conflict_detected ? "YES" : "no");
    printf("  Features [%d]: [", bp->dim);
    int limit = bp->dim < 6 ? bp->dim : 6;
    for (int i = 0; i < limit; i++) {
        printf("%.3f", bp->features[i]);
        if (i < limit - 1) printf(", ");
    }
    if (bp->dim > 6) printf(", ...");
    printf("]\n");
}

void multimodal_print_state(const MultiModalSystem *mm) {
    const char *names[] = {"Visual", "Auditory", "Tactile"};
    printf("\n=== Multi-Modal State ===\n");
    for (int m = 0; m < MODALITY_COUNT; m++) {
        printf("  %s: %s  conf=%.2f  prec=%.2f  dim=%d\n",
               names[m],
               mm->inputs[m].active ? "ACTIVE" : "off   ",
               mm->inputs[m].confidence,
               mm->precision[m],
               mm->inputs[m].dim);
    }
}
```

## The Three Binding Rules In Code

1. **Temporal rule**: `temporal_agreement` drops to 0.5 when timestamps are too far apart
2. **Spatial rule**: Captured by `cross_modal_corr()` — similar feature patterns from the same location correlate
3. **Inverse effectiveness**: Low-confidence signals get boosted when combined (`inv_eff` factor)

## Output: Unified Percept

The `BoundPercept` contains a 48-dim feature vector:
- `[0..31]` — Precision-weighted average of all modality features
- `[32..34]` — Per-modality energy (how much each sense contributed)
- `[35..37]` — Cross-modal correlations
- `[38..41]` — Summary (avg correlation, temporal agreement, modality count, conflict)

Plus metadata: `binding_strength`, `total_confidence`, `conflict_detected`.

---

*Next: [Milestone — Multi-Modal Recognition](07_milestone_sensory_expansion.md)*
