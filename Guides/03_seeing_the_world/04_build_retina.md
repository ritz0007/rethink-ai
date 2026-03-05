# 🔨 Build: The Retina — Turning Pixels Into Neural Signals

## What We're Building

A retina module that takes a grid of pixel values and converts them to neural signals using:
1. **Edge detection** — horizontal, vertical, and diagonal edge-detecting neurons
2. **Lateral inhibition** — sharpen features, suppress redundancy
3. **Spiking output** — convert processed signals into spike trains for our network

### Code: `src/perception/retina.h`

```c
/*
 * retina.h — Visual Input Processing
 * 
 * Rethink AI — Phase 3
 * 
 * Converts a 2D pixel grid into neural signals.
 * Mimics the retina + V1 (primary visual cortex):
 *   1. Edge detection using receptive field filters
 *   2. Lateral inhibition for feature sharpening
 *   3. Output as spike-compatible signals
 */

#ifndef RETINA_H
#define RETINA_H

#include <stdint.h>

/* Maximum image dimensions */
#define RETINA_MAX_SIZE 64

/* Edge types that our "V1 neurons" detect */
typedef enum {
    EDGE_HORIZONTAL = 0,
    EDGE_VERTICAL   = 1,
    EDGE_DIAG_LEFT  = 2,  /* \ diagonal */
    EDGE_DIAG_RIGHT = 3,  /* / diagonal */
    NUM_EDGE_TYPES  = 4
} EdgeType;

/* Retina output: edge-detected features */
typedef struct {
    int width;
    int height;
    
    /* Raw pixel input [y][x] */
    float pixels[RETINA_MAX_SIZE][RETINA_MAX_SIZE];
    
    /* Edge responses [edge_type][y][x] — how strongly each edge type is detected */
    float edges[NUM_EDGE_TYPES][RETINA_MAX_SIZE][RETINA_MAX_SIZE];
    
    /* Post-inhibition response [edge_type][y][x] — after lateral inhibition */
    float inhibited[NUM_EDGE_TYPES][RETINA_MAX_SIZE][RETINA_MAX_SIZE];
    
    /* Lateral inhibition strength */
    float inhibition_radius;
    float inhibition_strength;
} Retina;

/* Create a retina for images of given size */
Retina retina_create(int width, int height);

/* Load a pixel array into the retina (values 0.0 to 1.0) */
void retina_load_pixels(Retina *r, const float *pixels, int width, int height);

/* Process: detect edges using receptive field filters */
void retina_detect_edges(Retina *r);

/* Apply lateral inhibition to edge responses */
void retina_apply_inhibition(Retina *r);

/* Full processing pipeline: edges → inhibition */
void retina_process(Retina *r);

/* Convert retina output to a flat feature vector for downstream modules */
/* Returns total number of features written */
int retina_to_features(const Retina *r, float *out_features, int max_features);

/* Print ASCII visualization of detected edges */
void retina_print_edges(const Retina *r, EdgeType type);

/* Print the raw pixel input as ASCII art */
void retina_print_pixels(const Retina *r);

#endif /* RETINA_H */
```

### Code: `src/perception/retina.c`

```c
/*
 * retina.c — Retina Implementation
 * Rethink AI — Phase 3
 */

#include "retina.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ─── Edge detection kernels (3×3) ─── */
/* These mimic V1 simple cell receptive fields */

static const float KERNEL_H[3][3] = {
    { 1,  1,  1},
    { 0,  0,  0},
    {-1, -1, -1}
};

static const float KERNEL_V[3][3] = {
    { 1,  0, -1},
    { 1,  0, -1},
    { 1,  0, -1}
};

static const float KERNEL_DL[3][3] = {
    { 1,  0,  0},
    { 0,  0,  0},
    { 0,  0, -1}
};

static const float KERNEL_DR[3][3] = {
    { 0,  0,  1},
    { 0,  0,  0},
    {-1,  0,  0}
};

static const float (*KERNELS[4])[3] = {
    KERNEL_H, KERNEL_V, KERNEL_DL, KERNEL_DR
};

Retina retina_create(int width, int height) {
    Retina r;
    memset(&r, 0, sizeof(r));
    r.width = (width > RETINA_MAX_SIZE) ? RETINA_MAX_SIZE : width;
    r.height = (height > RETINA_MAX_SIZE) ? RETINA_MAX_SIZE : height;
    r.inhibition_radius = 2.0f;
    r.inhibition_strength = 0.3f;
    return r;
}

void retina_load_pixels(Retina *r, const float *pixels, int width, int height) {
    r->width = (width > RETINA_MAX_SIZE) ? RETINA_MAX_SIZE : width;
    r->height = (height > RETINA_MAX_SIZE) ? RETINA_MAX_SIZE : height;
    
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            r->pixels[y][x] = pixels[y * width + x];
        }
    }
}

void retina_detect_edges(Retina *r) {
    /* Apply each edge kernel to the image */
    for (int k = 0; k < NUM_EDGE_TYPES; k++) {
        for (int y = 1; y < r->height - 1; y++) {
            for (int x = 1; x < r->width - 1; x++) {
                float sum = 0.0f;
                
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        sum += r->pixels[y + ky][x + kx] * KERNELS[k][ky + 1][kx + 1];
                    }
                }
                
                /* ReLU-like: only positive responses (edge present) */
                r->edges[k][y][x] = (sum > 0) ? sum : 0.0f;
            }
        }
    }
}

void retina_apply_inhibition(Retina *r) {
    int rad = (int)r->inhibition_radius;
    
    for (int k = 0; k < NUM_EDGE_TYPES; k++) {
        for (int y = 0; y < r->height; y++) {
            for (int x = 0; x < r->width; x++) {
                float self = r->edges[k][y][x];
                float neighbor_sum = 0.0f;
                int count = 0;
                
                /* Sum activity of neighbors within radius */
                for (int dy = -rad; dy <= rad; dy++) {
                    for (int dx = -rad; dx <= rad; dx++) {
                        if (dy == 0 && dx == 0) continue;
                        int ny = y + dy, nx = x + dx;
                        if (ny >= 0 && ny < r->height && nx >= 0 && nx < r->width) {
                            neighbor_sum += r->edges[k][ny][nx];
                            count++;
                        }
                    }
                }
                
                /* Subtract neighbor activity (lateral inhibition) */
                float avg_neighbor = (count > 0) ? neighbor_sum / count : 0.0f;
                float inhibited = self - r->inhibition_strength * avg_neighbor;
                r->inhibited[k][y][x] = (inhibited > 0) ? inhibited : 0.0f;
            }
        }
    }
}

void retina_process(Retina *r) {
    retina_detect_edges(r);
    retina_apply_inhibition(r);
}

int retina_to_features(const Retina *r, float *out_features, int max_features) {
    int idx = 0;
    for (int k = 0; k < NUM_EDGE_TYPES && idx < max_features; k++) {
        for (int y = 0; y < r->height && idx < max_features; y++) {
            for (int x = 0; x < r->width && idx < max_features; x++) {
                out_features[idx++] = r->inhibited[k][y][x];
            }
        }
    }
    return idx;
}

void retina_print_pixels(const Retina *r) {
    const char *shades = " .:-=+*#%@";
    int num_shades = 10;
    
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            int idx = (int)(r->pixels[y][x] * (num_shades - 1));
            if (idx < 0) idx = 0;
            if (idx >= num_shades) idx = num_shades - 1;
            printf("%c", shades[idx]);
        }
        printf("\n");
    }
}

void retina_print_edges(const Retina *r, EdgeType type) {
    const char *names[] = {"Horizontal", "Vertical", "Diag \\", "Diag /"};
    printf("Edge type: %s\n", names[type]);
    
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            float val = r->inhibited[type][y][x];
            if (val > 0.5f) printf("#");
            else if (val > 0.2f) printf("+");
            else if (val > 0.05f) printf(".");
            else printf(" ");
        }
        printf("\n");
    }
}
```

---

## Test It

Create a simple test that draws a shape and processes it through the retina:

```c
/* In test_retina.c */
#include "perception/retina.h"

/* Draw a simple 'L' shape on an 8x8 grid */
float image[64] = {
    0,0,0,0,0,0,0,0,
    0,1,1,0,0,0,0,0,
    0,0,1,0,0,0,0,0,
    0,0,1,0,0,0,0,0,
    0,0,1,0,0,0,0,0,
    0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

Retina r = retina_create(8, 8);
retina_load_pixels(&r, image, 8, 8);
retina_process(&r);  /* Detect edges + apply inhibition */

retina_print_pixels(&r);
retina_print_edges(&r, EDGE_VERTICAL);
retina_print_edges(&r, EDGE_HORIZONTAL);
```

You'll see vertical edges on the sides of the L, and horizontal edges on the top and bottom arms. Just like V1 neurons would.

---

*Next: [05_build_self_organizing_map.md](05_build_self_organizing_map.md) — SOM: neurons that organize themselves*
