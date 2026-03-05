/*
 * som.h — Self-Organizing Map
 * Rethink AI — Phase 3
 */

#ifndef SOM_H
#define SOM_H

#define SOM_MAX_DIM 32
#define SOM_MAX_INPUT 128

typedef struct {
    int width, height;
    int input_dim;
    float weights[SOM_MAX_DIM][SOM_MAX_DIM][SOM_MAX_INPUT];
    float learning_rate;
    float radius;
} SOM;

void som_init(SOM *s, int width, int height, int input_dim);
void som_find_bmu(const SOM *s, const float *input, int *bx, int *by);
void som_train_step(SOM *s, const float *input);

#endif /* SOM_H */
