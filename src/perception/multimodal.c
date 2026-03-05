/*
 * multimodal.c — Multi-Modal Sensory Binding Implementation
 * Rethink AI — Phase 11 / V11
 *
 * The brain doesn't "see" and "hear" separately — it fuses them.
 * This module combines visual, auditory, and tactile features into
 * a single bound percept using Bayesian precision weighting.
 */

#include "multimodal.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

static const char *modality_names[MM_MAX_MODALITIES] = {
    "Visual", "Auditory", "Tactile"
};

void multimodal_init(MultiModalSystem *mm) {
    memset(mm, 0, sizeof(MultiModalSystem));
    /* Start with equal trust in all senses */
    for (int m = 0; m < MM_MAX_MODALITIES; m++) {
        mm->precision[m] = 1.0f;
    }
    mm->current_time = 0.0f;
}

void multimodal_set_input(MultiModalSystem *mm, ModalityType mod,
                          const float *features, int dim,
                          float confidence, float timestamp) {
    if (mod < 0 || mod >= MM_MAX_MODALITIES) return;
    ModalityInput *inp = &mm->inputs[mod];
    int copy_dim = dim < 32 ? dim : 32;
    memcpy(inp->features, features, copy_dim * sizeof(float));
    inp->feature_dim = copy_dim;
    inp->confidence = confidence;
    inp->timestamp = timestamp;
    inp->active = 1;
    mm->current_time = timestamp;
}

void multimodal_clear_input(MultiModalSystem *mm, ModalityType mod) {
    if (mod < 0 || mod >= MM_MAX_MODALITIES) return;
    mm->inputs[mod].active = 0;
    mm->inputs[mod].confidence = 0.0f;
}

/*
 * compute_temporal_coherence
 *
 * Temporal Rule: inputs arriving close in time bind more strongly.
 * Returns 0..1, where 1 = perfectly synchronized.
 */
static float compute_temporal_coherence(MultiModalSystem *mm) {
    float max_gap = 0.0f;
    int count = 0;
    float timestamps[MM_MAX_MODALITIES];

    for (int m = 0; m < MM_MAX_MODALITIES; m++) {
        if (mm->inputs[m].active) {
            timestamps[count++] = mm->inputs[m].timestamp;
        }
    }
    if (count < 2) return 1.0f;

    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            float gap = fabsf(timestamps[i] - timestamps[j]);
            if (gap > max_gap) max_gap = gap;
        }
    }

    /* Gaussian falloff — gap of 1.0 ≈ 60% coherence */
    return expf(-max_gap * max_gap / 2.0f);
}

/*
 * compute_cross_modal_correlation
 *
 * How similar are the feature patterns across modalities?
 * Uses normalized dot product on overlapping dimensions.
 */
static float compute_cross_modal_correlation(MultiModalSystem *mm) {
    float total_corr = 0.0f;
    int pairs = 0;

    for (int i = 0; i < MM_MAX_MODALITIES; i++) {
        if (!mm->inputs[i].active) continue;
        for (int j = i + 1; j < MM_MAX_MODALITIES; j++) {
            if (!mm->inputs[j].active) continue;

            int min_dim = mm->inputs[i].feature_dim;
            if (mm->inputs[j].feature_dim < min_dim)
                min_dim = mm->inputs[j].feature_dim;
            if (min_dim <= 0) continue;

            float dot = 0.0f, mag_a = 0.0f, mag_b = 0.0f;
            for (int d = 0; d < min_dim; d++) {
                dot   += mm->inputs[i].features[d] * mm->inputs[j].features[d];
                mag_a += mm->inputs[i].features[d] * mm->inputs[i].features[d];
                mag_b += mm->inputs[j].features[d] * mm->inputs[j].features[d];
            }
            mag_a = sqrtf(mag_a);
            mag_b = sqrtf(mag_b);

            if (mag_a > 1e-6f && mag_b > 1e-6f) {
                total_corr += dot / (mag_a * mag_b);
            }
            pairs++;
        }
    }

    return (pairs > 0) ? total_corr / pairs : 0.0f;
}

/*
 * multimodal_bind — The Main Binding Operation
 *
 * Steps:
 *   1. Count active modalities
 *   2. Compute temporal coherence (Rule 2)
 *   3. Compute cross-modal correlation (Rule 1 proxy)
 *   4. Bayesian precision-weighted feature fusion
 *   5. Apply inverse effectiveness (Rule 3)
 *   6. Detect conflicts
 */
void multimodal_bind(MultiModalSystem *mm) {
    BoundPercept *bp = &mm->percept;
    memset(bp, 0, sizeof(BoundPercept));

    /* Count active modalities */
    int active = 0;
    for (int m = 0; m < MM_MAX_MODALITIES; m++) {
        if (mm->inputs[m].active) active++;
    }
    bp->num_active = active;

    if (active == 0) return;

    /* Single modality → pass through directly */
    if (active == 1) {
        for (int m = 0; m < MM_MAX_MODALITIES; m++) {
            if (mm->inputs[m].active) {
                memcpy(bp->features, mm->inputs[m].features,
                       mm->inputs[m].feature_dim * sizeof(float));
                bp->feature_dim = mm->inputs[m].feature_dim;
                bp->binding_strength = 0.0f;
                bp->total_confidence = mm->inputs[m].confidence;
                bp->cross_modal_correlation = 0.0f;
                bp->conflict_detected = 0;
                break;
            }
        }
        return;
    }

    /* Multi-modal binding */
    float temporal_coh = compute_temporal_coherence(mm);
    float cross_corr   = compute_cross_modal_correlation(mm);

    /* Bayesian precision-weighted combination */
    float bound[MM_MAX_FEATURES];
    float total_precision[MM_MAX_FEATURES];
    memset(bound, 0, sizeof(bound));
    memset(total_precision, 0, sizeof(total_precision));

    int max_dim = 0;
    float total_conf = 0.0f;

    for (int m = 0; m < MM_MAX_MODALITIES; m++) {
        if (!mm->inputs[m].active) continue;

        /* Weight = confidence × learned precision × temporal coherence */
        float weight = mm->inputs[m].confidence * mm->precision[m] * temporal_coh;

        for (int d = 0; d < mm->inputs[m].feature_dim; d++) {
            bound[d] += mm->inputs[m].features[d] * weight;
            total_precision[d] += weight;
        }

        if (mm->inputs[m].feature_dim > max_dim)
            max_dim = mm->inputs[m].feature_dim;
        total_conf += mm->inputs[m].confidence;
    }

    /* Normalize by total precision per dimension */
    for (int d = 0; d < max_dim; d++) {
        if (total_precision[d] > 1e-6f) {
            bound[d] /= total_precision[d];
        }
    }

    /* Inverse effectiveness: weaker unisensory signals gain more
       from multi-modal combination (super-additive) */
    float avg_conf = total_conf / active;
    float inverse_boost = 1.0f + (1.0f - avg_conf) * 0.5f * (active - 1);

    /* Binding strength = temporal × correlation */
    float binding = temporal_coh * ((cross_corr + 1.0f) / 2.0f);

    /* Apply boost proportional to binding strength */
    float boost = 1.0f + (inverse_boost - 1.0f) * binding;
    for (int d = 0; d < max_dim; d++) {
        bound[d] *= boost;
    }

    memcpy(bp->features, bound, max_dim * sizeof(float));
    bp->feature_dim = max_dim;
    bp->binding_strength = binding;
    bp->total_confidence = avg_conf * inverse_boost;
    if (bp->total_confidence > 1.0f) bp->total_confidence = 1.0f;
    bp->cross_modal_correlation = cross_corr;

    /* Detect conflict: low cross-correlation with high confidence suggests
       the senses disagree (like a ventriloquist illusion) */
    bp->conflict_detected = (cross_corr < 0.2f && avg_conf > 0.6f) ? 1 : 0;
}

void multimodal_update_precision(MultiModalSystem *mm, ModalityType mod,
                                 float success) {
    if (mod < 0 || mod >= MM_MAX_MODALITIES) return;
    /* Exponential moving average */
    float lr = 0.1f;
    mm->precision[mod] += lr * (success - mm->precision[mod]);
    if (mm->precision[mod] < 0.1f) mm->precision[mod] = 0.1f;
    if (mm->precision[mod] > 2.0f) mm->precision[mod] = 2.0f;
}

void multimodal_print(const MultiModalSystem *mm) {
    printf("\n=== Multi-Modal System ===\n");
    for (int m = 0; m < MM_MAX_MODALITIES; m++) {
        printf("  %s: %s  confidence=%.3f  precision=%.3f  dim=%d\n",
               modality_names[m],
               mm->inputs[m].active ? "ACTIVE" : "off",
               mm->inputs[m].confidence,
               mm->precision[m],
               mm->inputs[m].feature_dim);
    }
}

void multimodal_print_percept(const BoundPercept *bp) {
    printf("=== Bound Percept ===\n");
    printf("  Active modalities: %d\n", bp->num_active);
    printf("  Binding strength:  %.3f\n", bp->binding_strength);
    printf("  Total confidence:  %.3f\n", bp->total_confidence);
    printf("  Cross-modal corr:  %.3f\n", bp->cross_modal_correlation);
    printf("  Conflict:          %s\n", bp->conflict_detected ? "YES" : "no");
    printf("  Features [%d]: [", bp->feature_dim);
    int limit = bp->feature_dim < 8 ? bp->feature_dim : 8;
    for (int i = 0; i < limit; i++) {
        printf("%.3f", bp->features[i]);
        if (i < limit - 1) printf(", ");
    }
    if (bp->feature_dim > 8) printf(", ...");
    printf("]\n");
}
