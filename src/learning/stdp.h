/*
 * stdp.h — Spike-Timing Dependent Plasticity
 * Rethink AI — Phase 2
 */

#ifndef STDP_H
#define STDP_H

typedef struct {
    float a_plus;    /* LTP amplitude (positive), typically 0.01 */
    float a_minus;   /* LTD amplitude (positive), typically 0.012 */
    float tau_plus;  /* LTP time constant (ms), typically 20 */
    float tau_minus;  /* LTD time constant (ms), typically 20 */
    float w_max;     /* Maximum weight */
    float w_min;     /* Minimum weight */
} STDPParams;

STDPParams stdp_default_params(void);
float stdp_weight_change(const STDPParams *p, float dt_ms);
void stdp_apply(const STDPParams *p, float *weight, float pre_spike_time, float post_spike_time);

#endif /* STDP_H */
