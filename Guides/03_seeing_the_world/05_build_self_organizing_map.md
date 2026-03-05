# 🔨 Build: Self-Organizing Map — Neurons That Organize Themselves

## 🧠 Brain Science

In your brain, neurons in the cortex are arranged in **maps**. Neighboring neurons respond to similar things:
- In the visual cortex: nearby neurons respond to similar edge orientations
- In the auditory cortex: nearby neurons respond to similar frequencies
- In the somatosensory cortex: nearby neurons map to nearby body parts

Nobody programs this. The map **self-organizes** through experience. A baby's brain starts messy and gradually organizes as it receives input.

## The Self-Organizing Map (SOM)

Invented by Teuvo Kohonen in the 1980s, the SOM is a grid of neurons where:
1. Each neuron has a **weight vector** (what pattern it responds to)
2. When input arrives, the **closest matching neuron** wins (the "Best Matching Unit")
3. The winner AND its neighbors update their weights to become more like the input
4. Over time, the map organizes: similar patterns activate nearby neurons

**No backpropagation. No loss function. Just competition and neighborhood learning.**

### Code: `src/perception/som.h`

```c
/*
 * som.h — Self-Organizing Map (Kohonen Map)
 * 
 * Rethink AI — Phase 3
 * 
 * A 2D grid of neurons that self-organizes to represent input patterns.
 * Competitive Hebbian learning — no backprop.
 */

#ifndef SOM_H
#define SOM_H

#include <stdint.h>

#define SOM_MAX_DIM 32       /* Max grid dimension */
#define SOM_MAX_FEATURES 1024 /* Max input feature length */

typedef struct {
    /* Grid dimensions */
    int width;
    int height;
    int input_dim;    /* Length of input feature vector */
    
    /* Weights: [y][x][feature] — what each neuron "represents" */
    float weights[SOM_MAX_DIM][SOM_MAX_DIM][SOM_MAX_FEATURES];
    
    /* Learning parameters */
    float learning_rate;       /* Initial learning rate */
    float learning_rate_decay; /* How fast learning rate decreases */
    float neighborhood_radius; /* How far the winner influences neighbors */
    float radius_decay;        /* How fast the neighborhood shrinks */
    
    /* Training state */
    int current_epoch;
    int total_epochs;
    
    /* Last BMU (Best Matching Unit) */
    int bmu_x;
    int bmu_y;
} SOM;

/* Create a SOM with given dimensions */
SOM *som_create(int width, int height, int input_dim, int total_epochs);

/* Free SOM memory */
void som_destroy(SOM *som);

/* 
 * Train one step: present one input sample.
 * Finds the BMU, updates weights of BMU and neighbors.
 * Returns the BMU coordinates.
 */
void som_train_step(SOM *som, const float *input);

/* 
 * Find the Best Matching Unit (most similar neuron) for an input.
 * Returns via bmu_x, bmu_y pointers.
 */
void som_find_bmu(const SOM *som, const float *input, int *bmu_x, int *bmu_y);

/*
 * Get the "label" (most common class) at each map position.
 * Call after training to see what the map learned.
 */
void som_map_labels(const SOM *som, const float **inputs, const int *labels,
                    int num_samples, int label_map[][SOM_MAX_DIM]);

/* Print the map as ASCII showing which cells are active */
void som_print_activation(const SOM *som, const float *input);

/* Print the label map */
void som_print_label_map(const int label_map[][SOM_MAX_DIM], int width, int height);

#endif /* SOM_H */
```

### Code: `src/perception/som.c`

```c
/*
 * som.c — Self-Organizing Map Implementation
 * Rethink AI — Phase 3
 */

#include "som.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

SOM *som_create(int width, int height, int input_dim, int total_epochs) {
    SOM *som = (SOM *)calloc(1, sizeof(SOM));
    som->width = width;
    som->height = height;
    som->input_dim = input_dim;
    som->total_epochs = total_epochs;
    som->current_epoch = 0;
    
    /* Initial values */
    som->learning_rate = 0.5f;
    som->learning_rate_decay = 0.99f;
    som->neighborhood_radius = (float)(width > height ? width : height) / 2.0f;
    som->radius_decay = 0.995f;
    
    /* Initialize weights randomly (0 to 1) */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int f = 0; f < input_dim; f++) {
                som->weights[y][x][f] = (float)rand() / (float)RAND_MAX;
            }
        }
    }
    
    return som;
}

void som_destroy(SOM *som) {
    free(som);
}

static float distance_squared(const float *a, const float *b, int dim) {
    float sum = 0.0f;
    for (int i = 0; i < dim; i++) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sum;
}

void som_find_bmu(const SOM *som, const float *input, int *bmu_x, int *bmu_y) {
    float min_dist = FLT_MAX;
    *bmu_x = 0;
    *bmu_y = 0;
    
    for (int y = 0; y < som->height; y++) {
        for (int x = 0; x < som->width; x++) {
            float d = distance_squared(input, som->weights[y][x], som->input_dim);
            if (d < min_dist) {
                min_dist = d;
                *bmu_x = x;
                *bmu_y = y;
            }
        }
    }
}

void som_train_step(SOM *som, const float *input) {
    /* Find Best Matching Unit */
    int bx, by;
    som_find_bmu(som, input, &bx, &by);
    som->bmu_x = bx;
    som->bmu_y = by;
    
    float lr = som->learning_rate;
    float radius = som->neighborhood_radius;
    float radius_sq = radius * radius;
    
    /* Update BMU and neighbors */
    int r = (int)ceilf(radius);
    
    for (int y = by - r; y <= by + r; y++) {
        for (int x = bx - r; x <= bx + r; x++) {
            if (y < 0 || y >= som->height || x < 0 || x >= som->width) continue;
            
            /* Distance from BMU on the grid */
            float grid_dist_sq = (float)((x - bx) * (x - bx) + (y - by) * (y - by));
            
            if (grid_dist_sq > radius_sq) continue;
            
            /* Neighborhood function: Gaussian — closer = stronger influence */
            float influence = expf(-grid_dist_sq / (2.0f * radius_sq));
            
            /* Update weights: move toward the input */
            float update_rate = lr * influence;
            for (int f = 0; f < som->input_dim; f++) {
                som->weights[y][x][f] += update_rate * (input[f] - som->weights[y][x][f]);
            }
        }
    }
    
    /* Decay learning rate and radius */
    som->learning_rate *= som->learning_rate_decay;
    som->neighborhood_radius *= som->radius_decay;
    
    /* Floor values */
    if (som->learning_rate < 0.01f) som->learning_rate = 0.01f;
    if (som->neighborhood_radius < 1.0f) som->neighborhood_radius = 1.0f;
}

void som_print_activation(const SOM *som, const float *input) {
    int bx, by;
    som_find_bmu(som, input, &bx, &by);
    
    printf("SOM activation (BMU at [%d,%d]):\n", bx, by);
    for (int y = 0; y < som->height; y++) {
        for (int x = 0; x < som->width; x++) {
            float d = sqrtf(distance_squared(input, som->weights[y][x], som->input_dim));
            if (x == bx && y == by) printf("@");
            else if (d < 0.5f) printf("#");
            else if (d < 1.0f) printf("+");
            else if (d < 2.0f) printf(".");
            else printf(" ");
        }
        printf("\n");
    }
}

void som_print_label_map(const int label_map[][SOM_MAX_DIM], int width, int height) {
    printf("SOM Label Map:\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (label_map[y][x] >= 0) {
                printf("%d ", label_map[y][x]);
            } else {
                printf(". ");
            }
        }
        printf("\n");
    }
}
```

---

## Milestone: Pattern Recognition Without Backprop

After building the retina + SOM:
1. Load digit images → process through retina (edge detection)
2. Feed edge features to SOM → SOM self-organizes
3. Label each SOM cell with the digit it most responds to
4. To classify a new image: process through retina → find BMU on SOM → read its label

**All unsupervised. All brain-inspired. No backpropagation.**

---

*Phase 3 Complete → Tag as V3*

*Next: [Phase 4 — Memory](../04_memory/01_brain_types_of_memory.md)*
