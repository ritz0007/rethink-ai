/*
 * som.c — Self-Organizing Map Implementation
 * Rethink AI — Phase 3
 */

#include "som.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

void som_init(SOM *s, int width, int height, int input_dim) {
    s->width = width < SOM_MAX_DIM ? width : SOM_MAX_DIM;
    s->height = height < SOM_MAX_DIM ? height : SOM_MAX_DIM;
    s->input_dim = input_dim < SOM_MAX_INPUT ? input_dim : SOM_MAX_INPUT;
    s->learning_rate = 0.5f;
    s->radius = (float)(width > height ? width : height) / 2.0f;
    
    for (int y = 0; y < s->height; y++)
        for (int x = 0; x < s->width; x++)
            for (int d = 0; d < s->input_dim; d++)
                s->weights[y][x][d] = (float)rand() / RAND_MAX;
}

void som_find_bmu(const SOM *s, const float *input, int *bx, int *by) {
    float best_dist = 1e30f;
    *bx = 0; *by = 0;
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            float dist = 0;
            for (int d = 0; d < s->input_dim; d++) {
                float diff = input[d] - s->weights[y][x][d];
                dist += diff * diff;
            }
            if (dist < best_dist) {
                best_dist = dist;
                *bx = x; *by = y;
            }
        }
    }
}

void som_train_step(SOM *s, const float *input) {
    int bx, by;
    som_find_bmu(s, input, &bx, &by);
    
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x++) {
            float dx = (float)(x - bx), dy = (float)(y - by);
            float dist_sq = dx * dx + dy * dy;
            float influence = expf(-dist_sq / (2.0f * s->radius * s->radius));
            
            for (int d = 0; d < s->input_dim; d++) {
                s->weights[y][x][d] += s->learning_rate * influence * 
                    (input[d] - s->weights[y][x][d]);
            }
        }
    }
    
    /* Decay learning rate and radius */
    s->learning_rate *= 0.999f;
    s->radius *= 0.999f;
    if (s->radius < 1.0f) s->radius = 1.0f;
}
