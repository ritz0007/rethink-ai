/*
 * auditory.c — Cochlea-Inspired Auditory Processing Implementation
 * Rethink AI — Phase 11 / V11
 */

#include "auditory.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void auditory_init(AuditorySystem *as) {
    memset(as, 0, sizeof(AuditorySystem));
    /* Default band sensitivities — middle frequencies most sensitive
       (like human hearing curve, peaks ~1-4 kHz) */
    for (int b = 0; b < AUD_NUM_BANDS; b++) {
        float center = (float)b / (AUD_NUM_BANDS - 1);
        /* Bell curve centered at 0.3 (maps to ~1-4kHz range) */
        float diff = center - 0.3f;
        as->band_sensitivity[b] = 0.5f + 0.5f * expf(-diff * diff / 0.08f);
    }
    as->feature_dim = AUD_FEATURE_DIM;
}

/*
 * auditory_analyze_spectrum — Cochlea simulation
 *
 * The real cochlea does a continuous frequency decomposition.
 * We approximate with a simplified DFT into AUD_NUM_BANDS bins.
 * Each band represents a position along the basilar membrane.
 */
void auditory_analyze_spectrum(AuditorySystem *as, const float *samples, int num_samples) {
    if (num_samples <= 0) return;
    int N = num_samples < AUD_MAX_SAMPLES ? num_samples : AUD_MAX_SAMPLES;
    
    for (int b = 0; b < AUD_NUM_BANDS; b++) {
        /* Frequency for this band */
        float freq = (float)(b + 1) / AUD_NUM_BANDS;
        
        /* DFT at this frequency */
        float real_part = 0.0f, imag_part = 0.0f;
        for (int n = 0; n < N; n++) {
            float angle = 2.0f * (float)M_PI * freq * (float)n / N;
            real_part += samples[n] * cosf(angle);
            imag_part += samples[n] * sinf(angle);
        }
        
        /* Magnitude = energy at this frequency */
        float mag = sqrtf(real_part * real_part + imag_part * imag_part) / N;
        
        /* Apply sensitivity weighting (like cochlear amplifier) */
        as->spectrum[b] = mag * as->band_sensitivity[b];
    }
}

/*
 * auditory_detect_onsets — Cochlear Nucleus
 *
 * Compare current spectrum to previous frame.
 * Increases = onsets (sound starting), Decreases = offsets (sound stopping).
 */
void auditory_detect_onsets(AuditorySystem *as) {
    if (as->history_count == 0) {
        for (int b = 0; b < AUD_NUM_BANDS; b++) {
            as->onsets[b] = as->spectrum[b];
            as->offsets[b] = 0.0f;
        }
        return;
    }
    
    int prev_idx = (as->history_idx - 1 + AUD_HISTORY_LEN) % AUD_HISTORY_LEN;
    
    for (int b = 0; b < AUD_NUM_BANDS; b++) {
        float diff = as->spectrum[b] - as->history[prev_idx][b];
        if (diff > 0.01f) {
            as->onsets[b] = diff;
            as->offsets[b] = 0.0f;
        } else if (diff < -0.01f) {
            as->onsets[b] = 0.0f;
            as->offsets[b] = -diff;
        } else {
            as->onsets[b] = 0.0f;
            as->offsets[b] = 0.0f;
        }
    }
}

/*
 * auditory_track_change — Inferior Colliculus
 *
 * Computes spectral flux (how much the spectrum is changing)
 * and spectral centroid (where the "center of mass" of the sound is).
 */
void auditory_track_change(AuditorySystem *as) {
    float flux = 0.0f;
    for (int b = 0; b < AUD_NUM_BANDS; b++) {
        flux += as->onsets[b] * as->onsets[b] + as->offsets[b] * as->offsets[b];
    }
    as->spectral_flux = sqrtf(flux);
    
    float weighted_sum = 0.0f, total_energy = 0.0f;
    for (int b = 0; b < AUD_NUM_BANDS; b++) {
        weighted_sum += (float)(b + 1) * as->spectrum[b];
        total_energy += as->spectrum[b];
    }
    as->spectral_centroid = (total_energy > 1e-6f)
        ? weighted_sum / (total_energy * AUD_NUM_BANDS)
        : 0.5f;
}

void auditory_process(AuditorySystem *as, const float *samples, int num_samples) {
    auditory_analyze_spectrum(as, samples, num_samples);
    auditory_detect_onsets(as);
    auditory_track_change(as);
    
    /* Store in history ring buffer */
    memcpy(as->history[as->history_idx], as->spectrum,
           AUD_NUM_BANDS * sizeof(float));
    as->history_idx = (as->history_idx + 1) % AUD_HISTORY_LEN;
    if (as->history_count < AUD_HISTORY_LEN) as->history_count++;
}

/*
 * auditory_extract_features — A1 Output
 *
 * [0..15]  = spectrum (current frequency energies)
 * [16..19] = summary (centroid, flux, total energy, bandwidth)
 * [20..27] = onset pattern (compressed to 8 values)
 * [28..31] = temporal context (avg flux, stability, history depth, peak energy)
 */
void auditory_extract_features(AuditorySystem *as, float *out_features, int *out_dim) {
    int idx = 0;
    
    /* Spectrum bands [0..15] */
    for (int b = 0; b < AUD_NUM_BANDS && idx < AUD_FEATURE_DIM; b++) {
        out_features[idx++] = as->spectrum[b];
    }
    
    /* Summary [16..19] */
    float total_energy = 0.0f, max_energy = 0.0f;
    int peak_band = 0;
    for (int b = 0; b < AUD_NUM_BANDS; b++) {
        total_energy += as->spectrum[b];
        if (as->spectrum[b] > max_energy) {
            max_energy = as->spectrum[b];
            peak_band = b;
        }
    }
    
    float bandwidth = 0.0f;
    if (total_energy > 1e-6f) {
        for (int b = 0; b < AUD_NUM_BANDS; b++) {
            float diff = (float)(b - peak_band) / AUD_NUM_BANDS;
            bandwidth += (as->spectrum[b] / total_energy) * diff * diff;
        }
        bandwidth = sqrtf(bandwidth);
    }
    
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = as->spectral_centroid;
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = as->spectral_flux;
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = total_energy;
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = bandwidth;
    
    /* Onset pattern [20..27] */
    for (int i = 0; i < 8 && idx < AUD_FEATURE_DIM; i++) {
        int b1 = i * 2, b2 = i * 2 + 1;
        out_features[idx++] = (as->onsets[b1] + as->onsets[b2]) * 0.5f;
    }
    
    /* Temporal context [28..31] */
    float avg_flux = 0.0f, spectral_stability = 0.0f;
    if (as->history_count >= 2) {
        for (int h = 1; h < as->history_count; h++) {
            int cur = (as->history_idx - h + AUD_HISTORY_LEN) % AUD_HISTORY_LEN;
            int prev = (cur - 1 + AUD_HISTORY_LEN) % AUD_HISTORY_LEN;
            for (int b = 0; b < AUD_NUM_BANDS; b++) {
                float d = as->history[cur][b] - as->history[prev][b];
                avg_flux += d * d;
            }
        }
        avg_flux = sqrtf(avg_flux / (as->history_count - 1));
        spectral_stability = 1.0f / (1.0f + avg_flux * 10.0f);
    }
    
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = avg_flux;
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = spectral_stability;
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = (float)as->history_count / AUD_HISTORY_LEN;
    if (idx < AUD_FEATURE_DIM) out_features[idx++] = max_energy;
    
    memcpy(as->features, out_features, idx * sizeof(float));
    *out_dim = idx;
    as->feature_dim = idx;
}

void auditory_print_spectrum(const AuditorySystem *as) {
    printf("\n=== Auditory Spectrum (%d bands) ===\n", AUD_NUM_BANDS);
    for (int b = 0; b < AUD_NUM_BANDS; b++) {
        printf("  Band %2d: ", b);
        int bars = (int)(as->spectrum[b] * 40);
        for (int i = 0; i < bars && i < 40; i++) printf("#");
        printf(" %.3f", as->spectrum[b]);
        if (as->onsets[b] > 0.01f) printf(" [ONSET]");
        if (as->offsets[b] > 0.01f) printf(" [offset]");
        printf("\n");
    }
    printf("  Centroid: %.3f  Flux: %.3f\n",
           as->spectral_centroid, as->spectral_flux);
}

void auditory_print_features(const AuditorySystem *as) {
    printf("Auditory features [%d]: [", as->feature_dim);
    int limit = as->feature_dim < 8 ? as->feature_dim : 8;
    for (int i = 0; i < limit; i++) {
        printf("%.3f", as->features[i]);
        if (i < limit - 1) printf(", ");
    }
    if (as->feature_dim > 8) printf(", ...");
    printf("]\n");
}
