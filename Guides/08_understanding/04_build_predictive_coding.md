# 🔨 Build: Predictive Coding Engine — The Brain That Expects

## What We're Building

A hierarchical prediction system where:
1. Each level generates predictions about the level below
2. Only prediction errors propagate upward
3. Models update based on prediction errors
4. Precision (confidence) controls attention

## Code: `src/understanding/predictor.h`

```c
/*
 * predictor.h — Hierarchical Predictive Coding
 *
 * Rethink AI — Phase 8
 *
 * Each layer:
 *   - Receives input from below
 *   - Generates prediction of what it expects
 *   - Computes error = actual - predicted
 *   - Updates its model to reduce future errors
 *   - Passes error up (not raw input!)
 *
 * Understanding = prediction errors → 0
 */

#ifndef PREDICTOR_H
#define PREDICTOR_H

#define PRED_MAX_LAYERS 6
#define PRED_MAX_DIM 64

typedef struct {
    int dim;
    
    /* State */
    float input[PRED_MAX_DIM];           /* Actual input from below */
    float prediction[PRED_MAX_DIM];      /* What this layer predicts */
    float error[PRED_MAX_DIM];           /* Surprise = input - prediction */
    float precision[PRED_MAX_DIM];       /* Confidence per feature */
    
    /* Internal model: simple linear mapping + bias */
    /* prediction[i] = sum_j(weights[i][j] * representation[j]) + bias[i] */
    float representation[PRED_MAX_DIM];  /* This layer's understanding */
    float weights[PRED_MAX_DIM][PRED_MAX_DIM]; /* Generative model */
    float bias[PRED_MAX_DIM];
    
    /* Learning */
    float learning_rate;
    float precision_learning_rate;
    
    /* Statistics */
    float total_error;                   /* Running average of total error */
    float error_history[100];            /* Track error over time */
    int history_idx;
} PredictiveLayer;

typedef struct {
    PredictiveLayer layers[PRED_MAX_LAYERS];
    int num_layers;
    int input_dim;
    
    /* Overall state */
    float surprise;           /* Current total surprise */
    float understanding;      /* 1 - normalized surprise (how well we predict) */
    int timestep;
} Predictor;

/* Create a hierarchical predictor */
Predictor *predictor_create(int num_layers, int input_dim);
void predictor_destroy(Predictor *pred);

/* Feed input and process through hierarchy */
void predictor_process(Predictor *pred, const float *input);

/* Get the top-level representation (the "understanding") */
void predictor_get_representation(const Predictor *pred, float *repr, int *dim);

/* Get current prediction of what input SHOULD be */
void predictor_get_prediction(const Predictor *pred, float *prediction);

/* How surprised is the system? (0 = expected, 1 = totally unexpected) */
float predictor_get_surprise(const Predictor *pred);

/* How well does the system understand? (0 = clueless, 1 = perfect) */
float predictor_get_understanding(const Predictor *pred);

/* Force the system to predict the NEXT input (before seeing it) */
void predictor_forecast(Predictor *pred, float *next_prediction);

/* Print status */
void predictor_print_status(const Predictor *pred);
void predictor_print_errors(const Predictor *pred);

#endif /* PREDICTOR_H */
```

## Code: `src/understanding/predictor.c`

```c
#include "predictor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

Predictor *predictor_create(int num_layers, int input_dim) {
    Predictor *pred = (Predictor *)calloc(1, sizeof(Predictor));
    pred->num_layers = (num_layers < PRED_MAX_LAYERS) ? num_layers : PRED_MAX_LAYERS;
    pred->input_dim = (input_dim < PRED_MAX_DIM) ? input_dim : PRED_MAX_DIM;
    pred->timestep = 0;
    
    for (int l = 0; l < pred->num_layers; l++) {
        PredictiveLayer *layer = &pred->layers[l];
        layer->dim = pred->input_dim;  /* Same dim for simplicity */
        layer->learning_rate = 0.05f;
        layer->precision_learning_rate = 0.01f;
        layer->total_error = 1.0f;
        layer->history_idx = 0;
        
        /* Initialize precision to moderate confidence */
        for (int i = 0; i < layer->dim; i++) {
            layer->precision[i] = 1.0f;
        }
        
        /* Initialize weights with small random values */
        for (int i = 0; i < layer->dim; i++) {
            for (int j = 0; j < layer->dim; j++) {
                layer->weights[i][j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
            }
            /* Diagonal dominance — identity-like initialization */
            layer->weights[i][i] += 0.8f;
            layer->bias[i] = 0.0f;
        }
    }
    
    return pred;
}

void predictor_destroy(Predictor *pred) {
    free(pred);
}

static void layer_generate_prediction(PredictiveLayer *layer) {
    /* prediction = W × representation + bias */
    for (int i = 0; i < layer->dim; i++) {
        float sum = layer->bias[i];
        for (int j = 0; j < layer->dim; j++) {
            sum += layer->weights[i][j] * layer->representation[j];
        }
        /* Sigmoid-like activation to keep predictions bounded */
        layer->prediction[i] = tanhf(sum);
    }
}

static void layer_compute_error(PredictiveLayer *layer) {
    float total = 0.0f;
    for (int i = 0; i < layer->dim; i++) {
        layer->error[i] = layer->input[i] - layer->prediction[i];
        total += layer->error[i] * layer->error[i];
    }
    layer->total_error = layer->total_error * 0.95f + (total / layer->dim) * 0.05f;
    
    /* Record in history */
    layer->error_history[layer->history_idx % 100] = total / layer->dim;
    layer->history_idx++;
}

static void layer_update_model(PredictiveLayer *layer) {
    float lr = layer->learning_rate;
    
    for (int i = 0; i < layer->dim; i++) {
        /* Precision-weighted error */
        float weighted_error = layer->error[i] * layer->precision[i];
        
        /* Update representation (recognition model) */
        layer->representation[i] += lr * weighted_error;
        
        /* Update generative model (weights) */
        for (int j = 0; j < layer->dim; j++) {
            layer->weights[i][j] += lr * 0.1f * layer->error[i] * layer->representation[j];
        }
        layer->bias[i] += lr * 0.1f * layer->error[i];
        
        /* Update precision based on error variability */
        float error_sq = layer->error[i] * layer->error[i];
        /* precision = 1 / variance — high variance = low precision */
        float variance_estimate = layer->precision[i] > 0.01f ? 
                                  1.0f / layer->precision[i] : 100.0f;
        variance_estimate = variance_estimate * 0.95f + error_sq * 0.05f;
        if (variance_estimate > 0.001f) {
            layer->precision[i] = 1.0f / variance_estimate;
        }
        /* Clamp precision */
        if (layer->precision[i] > 100.0f) layer->precision[i] = 100.0f;
        if (layer->precision[i] < 0.01f) layer->precision[i] = 0.01f;
    }
}

void predictor_process(Predictor *pred, const float *input) {
    int dim = pred->input_dim;
    
    /* Layer 0 gets raw input */
    memcpy(pred->layers[0].input, input, dim * sizeof(float));
    
    /* Bottom-up pass: compute errors at each layer */
    for (int l = 0; l < pred->num_layers; l++) {
        PredictiveLayer *layer = &pred->layers[l];
        
        /* 1. Generate prediction from current model */
        layer_generate_prediction(layer);
        
        /* 2. Compute error */
        layer_compute_error(layer);
        
        /* 3. Update model to reduce future errors */
        layer_update_model(layer);
        
        /* 4. Pass error (not input!) to next layer */
        if (l + 1 < pred->num_layers) {
            /* Next layer's input = this layer's precision-weighted error */
            for (int i = 0; i < dim; i++) {
                pred->layers[l + 1].input[i] = layer->error[i] * 
                    sqrtf(layer->precision[i]) * 0.1f;
            }
        }
    }
    
    /* Compute overall surprise and understanding */
    float total_surprise = 0.0f;
    for (int l = 0; l < pred->num_layers; l++) {
        total_surprise += pred->layers[l].total_error;
    }
    pred->surprise = total_surprise / pred->num_layers;
    pred->understanding = 1.0f / (1.0f + pred->surprise);
    
    pred->timestep++;
}

void predictor_get_representation(const Predictor *pred, float *repr, int *dim) {
    /* Top layer representation = highest-level understanding */
    int top = pred->num_layers - 1;
    memcpy(repr, pred->layers[top].representation, 
           pred->input_dim * sizeof(float));
    if (dim) *dim = pred->input_dim;
}

void predictor_get_prediction(const Predictor *pred, float *prediction) {
    /* Layer 0's prediction = what we expect the input to be */
    memcpy(prediction, pred->layers[0].prediction, 
           pred->input_dim * sizeof(float));
}

float predictor_get_surprise(const Predictor *pred) {
    return pred->surprise;
}

float predictor_get_understanding(const Predictor *pred) {
    return pred->understanding;
}

void predictor_forecast(Predictor *pred, float *next_prediction) {
    /* Use current state to predict next input */
    layer_generate_prediction(&pred->layers[0]);
    memcpy(next_prediction, pred->layers[0].prediction, 
           pred->input_dim * sizeof(float));
}

void predictor_print_status(const Predictor *pred) {
    printf("\n=== Predictive Coding Status (t=%d) ===\n", pred->timestep);
    printf("Overall surprise:     %.4f\n", pred->surprise);
    printf("Understanding level:  %.1f%%\n", pred->understanding * 100);
    
    int bar = (int)(pred->understanding * 30);
    printf("Understanding: [");
    for (int i = 0; i < bar; i++) printf("█");
    for (int i = bar; i < 30; i++) printf("░");
    printf("] %.1f%%\n", pred->understanding * 100);
    
    for (int l = 0; l < pred->num_layers; l++) {
        printf("  Layer %d: error=%.4f\n", l, pred->layers[l].total_error);
    }
    printf("========================================\n");
}

void predictor_print_errors(const Predictor *pred) {
    printf("\nError Over Time (Layer 0):\n");
    const PredictiveLayer *layer = &pred->layers[0];
    int count = (layer->history_idx < 100) ? layer->history_idx : 100;
    int start = (layer->history_idx < 100) ? 0 : layer->history_idx - 100;
    
    float max_err = 0.01f;
    for (int i = 0; i < count; i++) {
        int idx = (start + i) % 100;
        if (layer->error_history[idx] > max_err) max_err = layer->error_history[idx];
    }
    
    for (int row = 10; row >= 0; row--) {
        float threshold = max_err * row / 10.0f;
        printf("  %5.3f |", threshold);
        for (int i = 0; i < count && i < 50; i++) {
            int idx = (start + i * count / 50) % 100;
            printf("%s", layer->error_history[idx] >= threshold ? "█" : " ");
        }
        printf("\n");
    }
    printf("        +");
    for (int i = 0; i < 50; i++) printf("─");
    printf("\n         time →\n");
}
```

## Test: `test_predictor.c`

```c
/*
 * test_predictor.c — Watch the brain learn to predict
 *
 * Shows: understanding increases as prediction errors decrease
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_predictor test_predictor.c predictor.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "predictor.h"

#define DIM 8

int main(void) {
    srand(42);
    printf("=== Predictive Coding: Learning to Predict ===\n\n");
    
    Predictor *pred = predictor_create(3, DIM);
    float input[DIM];
    
    /* === Test 1: Repeating pattern === */
    printf("TEST 1: Repeating pattern (should learn quickly)\n");
    float pattern[DIM] = {0.5, -0.3, 0.8, -0.1, 0.4, -0.6, 0.2, 0.7};
    
    for (int t = 0; t < 100; t++) {
        /* Same pattern every time */
        predictor_process(pred, pattern);
        
        if (t % 20 == 0) {
            printf("  t=%3d: surprise=%.4f  understanding=%.1f%%\n",
                   t, predictor_get_surprise(pred), 
                   predictor_get_understanding(pred) * 100);
        }
    }
    predictor_print_status(pred);
    
    /* === Test 2: Pattern suddenly changes === */
    printf("\nTEST 2: SURPRISE! Pattern changes at t=100\n");
    float new_pattern[DIM] = {-0.5, 0.3, -0.8, 0.1, -0.4, 0.6, -0.2, -0.7};
    
    for (int t = 100; t < 200; t++) {
        predictor_process(pred, new_pattern);
        
        if (t == 100 || t == 101 || t == 105 || t == 110 || 
            t == 120 || t == 150 || t == 199) {
            printf("  t=%3d: surprise=%.4f  understanding=%.1f%%",
                   t, predictor_get_surprise(pred),
                   predictor_get_understanding(pred) * 100);
            if (t == 100) printf("  ← BIG SURPRISE!");
            if (t == 110) printf("  ← adapting...");
            if (t == 199) printf("  ← learned it!");
            printf("\n");
        }
    }
    
    /* === Test 3: Alternating patterns === */
    printf("\nTEST 3: Alternating between two patterns\n");
    
    Predictor *pred2 = predictor_create(3, DIM);
    
    for (int t = 0; t < 200; t++) {
        float *p = (t % 2 == 0) ? pattern : new_pattern;
        predictor_process(pred2, p);
        
        if (t % 40 == 0) {
            printf("  t=%3d: surprise=%.4f  understanding=%.1f%%\n",
                   t, predictor_get_surprise(pred2),
                   predictor_get_understanding(pred2) * 100);
        }
    }
    printf("  (Harder to predict — understanding plateus lower)\n");
    
    /* === Test 4: Prediction accuracy === */
    printf("\nTEST 4: Prediction accuracy (trained predictor)\n");
    float forecast[DIM];
    predictor_forecast(pred, forecast);
    printf("  Expected: [");
    for (int i = 0; i < DIM; i++) printf("%.2f ", new_pattern[i]);
    printf("]\n");
    printf("  Predicted:[");
    for (int i = 0; i < DIM; i++) printf("%.2f ", forecast[i]);
    printf("]\n");
    
    predictor_print_errors(pred);
    
    printf("\n=== KEY INSIGHT ===\n");
    printf("Understanding = predicting correctly.\n");
    printf("When surprise drops, understanding rises.\n");
    printf("When the world changes, surprise spikes and the brain LEARNS.\n");
    printf("This is the computational definition of understanding.\n");
    
    predictor_destroy(pred);
    predictor_destroy(pred2);
    return 0;
}
```

## What We Just Built

A system where:
- **Understanding is measurable** — it's the inverse of prediction error
- **Learning is automatic** — prediction errors drive model updates
- **Attention is principled** — precision weights control what matters
- **Only surprises propagate** — massive information compression

---

*Next: [Build — Causal Reasoning Network](05_build_causal_network.md)*
