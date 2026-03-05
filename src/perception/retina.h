/*
 * retina.h — Edge Detection + Lateral Inhibition
 * Rethink AI — Phase 3
 */

#ifndef RETINA_H
#define RETINA_H

#define RETINA_MAX_SIZE 64

typedef enum {
    EDGE_HORIZONTAL = 0,
    EDGE_VERTICAL,
    EDGE_DIAG_LEFT,
    EDGE_DIAG_RIGHT,
    EDGE_TYPE_COUNT
} EdgeType;

typedef struct {
    int size;
    float pixels[RETINA_MAX_SIZE][RETINA_MAX_SIZE];
    float edges[EDGE_TYPE_COUNT][RETINA_MAX_SIZE][RETINA_MAX_SIZE];
    float inhibited[RETINA_MAX_SIZE][RETINA_MAX_SIZE];
} Retina;

void retina_init(Retina *r, int size);
void retina_load(Retina *r, const float *image);
void retina_detect_edges(Retina *r);
void retina_lateral_inhibition(Retina *r, float strength);
void retina_extract_features(const Retina *r, float *features, int *num_features);

#endif /* RETINA_H */
