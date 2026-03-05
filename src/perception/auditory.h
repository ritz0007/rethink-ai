/*
 * auditory.h — Cochlea-Inspired Auditory Processing
 * Rethink AI — Phase 11 / V11
 *
 * Models: cochlea (freq decomposition), cochlear nucleus (onset detect),
 *         inferior colliculus (spectral change), A1 (feature extraction)
 */

#ifndef AUDITORY_H
#define AUDITORY_H

#define AUD_MAX_SAMPLES    256
#define AUD_NUM_BANDS      16   /* Frequency bands (like cochlear positions) */
#define AUD_HISTORY_LEN    8    /* How many past frames we remember */
#define AUD_FEATURE_DIM    32   /* Output feature dimension */

/* ──── Auditory System ──── */
typedef struct {
    /* Cochlea: current frequency spectrum */
    float spectrum[AUD_NUM_BANDS];
    
    /* Cochlear Nucleus: history for onset/offset detection */
    float history[AUD_HISTORY_LEN][AUD_NUM_BANDS];
    int history_idx;
    int history_count;
    
    /* Onset/offset detection */
    float onsets[AUD_NUM_BANDS];
    float offsets[AUD_NUM_BANDS];
    
    /* Spectral change (inferior colliculus) */
    float spectral_flux;        /* Overall change magnitude */
    float spectral_centroid;    /* "Center of mass" of spectrum */
    
    /* A1: tonotopic sensitivity (like homunculus for hearing) */
    float band_sensitivity[AUD_NUM_BANDS];
    
    /* Output features */
    float features[AUD_FEATURE_DIM];
    int feature_dim;
} AuditorySystem;

/* Lifecycle */
void auditory_init(AuditorySystem *as);

/* Process a raw audio buffer — like sound hitting the cochlea */
void auditory_process(AuditorySystem *as, const float *samples, int num_samples);

/* Internal: DFT-based frequency decomposition (cochlea) */
void auditory_analyze_spectrum(AuditorySystem *as, const float *samples, int num_samples);

/* Internal: onset/offset detection (cochlear nucleus) */
void auditory_detect_onsets(AuditorySystem *as);

/* Internal: spectral change tracking (inferior colliculus) */
void auditory_track_change(AuditorySystem *as);

/* Extract full feature vector for downstream modules (A1 output) */
void auditory_extract_features(AuditorySystem *as, float *out_features, int *out_dim);

/* Printing */
void auditory_print_spectrum(const AuditorySystem *as);
void auditory_print_features(const AuditorySystem *as);

#endif /* AUDITORY_H */
