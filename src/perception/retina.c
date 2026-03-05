/*
 * retina.c — Retina Implementation
 * Rethink AI — Phase 3
 */

#include "retina.h"
#include <string.h>
#include <math.h>

/* Edge detection kernels (3x3) */
static const float KERNELS[EDGE_TYPE_COUNT][3][3] = {
    /* Horizontal */ {{ 1, 1, 1}, { 0, 0, 0}, {-1,-1,-1}},
    /* Vertical */   {{ 1, 0,-1}, { 1, 0,-1}, { 1, 0,-1}},
    /* DiagLeft */   {{ 0, 1, 1}, {-1, 0, 1}, {-1,-1, 0}},
    /* DiagRight */  {{ 1, 1, 0}, { 1, 0,-1}, { 0,-1,-1}}
};

void retina_init(Retina *r, int size) {
    memset(r, 0, sizeof(Retina));
    r->size = size < RETINA_MAX_SIZE ? size : RETINA_MAX_SIZE;
}

void retina_load(Retina *r, const float *image) {
    for (int y = 0; y < r->size; y++)
        for (int x = 0; x < r->size; x++)
            r->pixels[y][x] = image[y * r->size + x];
}

void retina_detect_edges(Retina *r) {
    for (int t = 0; t < EDGE_TYPE_COUNT; t++) {
        for (int y = 1; y < r->size - 1; y++) {
            for (int x = 1; x < r->size - 1; x++) {
                float sum = 0;
                for (int ky = -1; ky <= 1; ky++)
                    for (int kx = -1; kx <= 1; kx++)
                        sum += r->pixels[y + ky][x + kx] * KERNELS[t][ky + 1][kx + 1];
                r->edges[t][y][x] = fabsf(sum);
            }
        }
    }
}

void retina_lateral_inhibition(Retina *r, float strength) {
    for (int y = 0; y < r->size; y++) {
        for (int x = 0; x < r->size; x++) {
            float center = 0;
            for (int t = 0; t < EDGE_TYPE_COUNT; t++)
                center += r->edges[t][y][x];
            
            float surround = 0;
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dy == 0 && dx == 0) continue;
                    int ny = y + dy, nx = x + dx;
                    if (ny < 0 || ny >= r->size || nx < 0 || nx >= r->size) continue;
                    for (int t = 0; t < EDGE_TYPE_COUNT; t++)
                        surround += r->edges[t][ny][nx];
                    count++;
                }
            }
            if (count > 0) surround /= count;
            
            float val = center - strength * surround;
            r->inhibited[y][x] = val > 0 ? val : 0;
        }
    }
}

void retina_extract_features(const Retina *r, float *features, int *num_features) {
    /* Extract: total energy per edge type + spatial distribution */
    int idx = 0;
    for (int t = 0; t < EDGE_TYPE_COUNT; t++) {
        float total = 0;
        for (int y = 0; y < r->size; y++)
            for (int x = 0; x < r->size; x++)
                total += r->edges[t][y][x];
        features[idx++] = total / (r->size * r->size);
    }
    /* Total inhibited energy */
    float total_inh = 0;
    for (int y = 0; y < r->size; y++)
        for (int x = 0; x < r->size; x++)
            total_inh += r->inhibited[y][x];
    features[idx++] = total_inh / (r->size * r->size);
    
    *num_features = idx;
}
