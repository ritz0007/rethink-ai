/*
 * predictor.h — Hierarchical Predictive Coding
 * Rethink AI — Phase 8
 */

#ifndef PREDICTOR_H
#define PREDICTOR_H

#define PRED_MAX_LAYERS 6
#define PRED_MAX_DIM 64

typedef struct {
    float input[PRED_MAX_DIM];
    float prediction[PRED_MAX_DIM];
    float error[PRED_MAX_DIM];
    float precision[PRED_MAX_DIM];
    float representation[PRED_MAX_DIM];
    float weights[PRED_MAX_DIM][PRED_MAX_DIM];
    int input_dim;
    int output_dim;
} PredictiveLayer;

typedef struct {
    PredictiveLayer layers[PRED_MAX_LAYERS];
    int num_layers;
    float learning_rate;
    float total_surprise;
    int step;
} PredictiveNet;

PredictiveNet *predictive_create(const int *layer_dims, int num_layers, float lr);
void predictive_destroy(PredictiveNet *pn);

void predictive_process(PredictiveNet *pn, const float *input, int dim);
float predictive_surprise(const PredictiveNet *pn);
float predictive_understanding(const PredictiveNet *pn);

#endif /* PREDICTOR_H */
