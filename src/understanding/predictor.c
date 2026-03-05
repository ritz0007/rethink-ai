/*
 * predictor.c — Predictive Coding Implementation
 * Rethink AI — Phase 8
 */

#include "predictor.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

PredictiveNet *predictive_create(const int *layer_dims, int num_layers, float lr) {
    PredictiveNet *pn = (PredictiveNet *)calloc(1, sizeof(PredictiveNet));
    pn->num_layers = num_layers < PRED_MAX_LAYERS ? num_layers : PRED_MAX_LAYERS;
    pn->learning_rate = lr;
    
    for (int l = 0; l < pn->num_layers; l++) {
        PredictiveLayer *layer = &pn->layers[l];
        int dim = layer_dims[l] < PRED_MAX_DIM ? layer_dims[l] : PRED_MAX_DIM;
        layer->input_dim = dim;
        layer->output_dim = (l + 1 < pn->num_layers) ?
            (layer_dims[l + 1] < PRED_MAX_DIM ? layer_dims[l + 1] : PRED_MAX_DIM) : dim;
        
        for (int i = 0; i < PRED_MAX_DIM; i++) {
            layer->precision[i] = 1.0f;
            for (int j = 0; j < PRED_MAX_DIM; j++)
                layer->weights[i][j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
        }
    }
    return pn;
}

void predictive_destroy(PredictiveNet *pn) { free(pn); }

static float tanh_f(float x) { return tanhf(x); }

void predictive_process(PredictiveNet *pn, const float *input, int dim) {
    PredictiveLayer *L0 = &pn->layers[0];
    int d = dim < L0->input_dim ? dim : L0->input_dim;
    memcpy(L0->input, input, d * sizeof(float));
    
    /* Bottom-up: compute input for each layer */
    for (int l = 0; l < pn->num_layers; l++) {
        PredictiveLayer *layer = &pn->layers[l];
        
        /* Compute prediction from current representation */
        for (int i = 0; i < layer->input_dim; i++) {
            float sum = 0;
            for (int j = 0; j < layer->output_dim; j++)
                sum += layer->weights[i][j] * layer->representation[j];
            layer->prediction[i] = tanh_f(sum);
        }
        
        /* Compute prediction error */
        for (int i = 0; i < layer->input_dim; i++) {
            layer->error[i] = (layer->input[i] - layer->prediction[i]) * layer->precision[i];
        }
        
        /* Update representation based on error */
        for (int j = 0; j < layer->output_dim; j++) {
            float grad = 0;
            for (int i = 0; i < layer->input_dim; i++)
                grad += layer->weights[i][j] * layer->error[i];
            layer->representation[j] += pn->learning_rate * grad;
        }
        
        /* Update weights (learn to predict) */
        for (int i = 0; i < layer->input_dim; i++) {
            for (int j = 0; j < layer->output_dim; j++) {
                layer->weights[i][j] += pn->learning_rate * 
                    layer->error[i] * layer->representation[j];
            }
        }
        
        /* Update precision (Welford-like) */
        for (int i = 0; i < layer->input_dim; i++) {
            float err_sq = layer->error[i] * layer->error[i];
            layer->precision[i] = 1.0f / (err_sq + 0.01f);
            if (layer->precision[i] > 10.0f) layer->precision[i] = 10.0f;
        }
        
        /* Pass errors up to next layer */
        if (l + 1 < pn->num_layers) {
            PredictiveLayer *next = &pn->layers[l + 1];
            for (int i = 0; i < next->input_dim && i < layer->output_dim; i++)
                next->input[i] = layer->error[i];
        }
    }
    
    /* Accumulate total surprise */
    pn->total_surprise = 0;
    for (int i = 0; i < L0->input_dim; i++)
        pn->total_surprise += L0->error[i] * L0->error[i];
    pn->total_surprise = sqrtf(pn->total_surprise / L0->input_dim);
    pn->step++;
}

float predictive_surprise(const PredictiveNet *pn) {
    return pn->total_surprise;
}

float predictive_understanding(const PredictiveNet *pn) {
    return 1.0f / (1.0f + pn->total_surprise);
}
