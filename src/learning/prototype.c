/*
 * prototype.c — Prototype Learner Implementation
 * Rethink AI — Phase 5
 */

#include "prototype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

PrototypeNet *prototype_create(int feature_dim) {
    PrototypeNet *pn = (PrototypeNet *)calloc(1, sizeof(PrototypeNet));
    pn->feature_dim = feature_dim < PROTO_MAX_FEATURES ? feature_dim : PROTO_MAX_FEATURES;
    pn->novelty_threshold = 0.3f;
    for (int i = 0; i < pn->feature_dim; i++) {
        pn->attention[i] = 1.0f;
    }
    return pn;
}

void prototype_destroy(PrototypeNet *pn) { free(pn); }

static float cosine_sim(const float *a, const float *b, const float *attn, int n) {
    float dot = 0, ma = 0, mb = 0;
    for (int i = 0; i < n; i++) {
        float aa = a[i] * attn[i], bb = b[i] * attn[i];
        dot += aa * bb;
        ma += aa * aa;
        mb += bb * bb;
    }
    if (ma < 1e-8f || mb < 1e-8f) return 0.0f;
    return dot / (sqrtf(ma) * sqrtf(mb));
}

void prototype_learn(PrototypeNet *pn, const float *features, int dim, const char *category) {
    int d = dim < pn->feature_dim ? dim : pn->feature_dim;
    
    /* Find existing category */
    int idx = -1;
    for (int i = 0; i < pn->num_categories; i++) {
        if (strcmp(pn->categories[i].name, category) == 0) { idx = i; break; }
    }
    
    if (idx < 0) {
        if (pn->num_categories >= PROTO_MAX_CATEGORIES) return;
        idx = pn->num_categories++;
        memset(&pn->categories[idx], 0, sizeof(Prototype));
        strncpy(pn->categories[idx].name, category, 31);
        pn->categories[idx].feature_dim = d;
    }
    
    Prototype *p = &pn->categories[idx];
    p->count++;
    
    /* Online mean update (Welford's) */
    for (int i = 0; i < d; i++) {
        float delta = features[i] - p->centroid[i];
        p->centroid[i] += delta / p->count;
        float delta2 = features[i] - p->centroid[i];
        p->variance[i] += delta * delta2;
    }
}

int prototype_classify(const PrototypeNet *pn, const float *features, int dim,
                       char *out_name, float *out_confidence) {
    int d = dim < pn->feature_dim ? dim : pn->feature_dim;
    float best_sim = -1.0f;
    int best_idx = -1;
    
    for (int i = 0; i < pn->num_categories; i++) {
        float sim = cosine_sim(pn->categories[i].centroid, features, pn->attention, d);
        if (sim > best_sim) {
            best_sim = sim;
            best_idx = i;
        }
    }
    
    if (best_idx >= 0) {
        if (out_name) strncpy(out_name, pn->categories[best_idx].name, 31);
        if (out_confidence) *out_confidence = best_sim;
    } else {
        if (out_name) strcpy(out_name, "unknown");
        if (out_confidence) *out_confidence = 0.0f;
    }
    return best_idx;
}

int prototype_is_novel(const PrototypeNet *pn, const float *features, int dim) {
    int d = dim < pn->feature_dim ? dim : pn->feature_dim;
    for (int i = 0; i < pn->num_categories; i++) {
        float sim = cosine_sim(pn->categories[i].centroid, features, pn->attention, d);
        if (sim > pn->novelty_threshold) return 0;
    }
    return 1;
}
