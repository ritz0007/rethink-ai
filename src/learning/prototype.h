/*
 * prototype.h — Few-Shot Prototype Learner
 * Rethink AI — Phase 5
 */

#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#define PROTO_MAX_FEATURES 128
#define PROTO_MAX_CATEGORIES 32
#define PROTO_MAX_EXAMPLES 1000

typedef struct {
    char name[32];
    float centroid[PROTO_MAX_FEATURES];
    float variance[PROTO_MAX_FEATURES];
    int count;
    int feature_dim;
} Prototype;

typedef struct {
    Prototype categories[PROTO_MAX_CATEGORIES];
    int num_categories;
    int feature_dim;
    float attention[PROTO_MAX_FEATURES];
    float novelty_threshold;
} PrototypeNet;

PrototypeNet *prototype_create(int feature_dim);
void prototype_destroy(PrototypeNet *pn);

void prototype_learn(PrototypeNet *pn, const float *features, int dim, const char *category);
int prototype_classify(const PrototypeNet *pn, const float *features, int dim,
                       char *out_name, float *out_confidence);
int prototype_is_novel(const PrototypeNet *pn, const float *features, int dim);

#endif /* PROTOTYPE_H */
