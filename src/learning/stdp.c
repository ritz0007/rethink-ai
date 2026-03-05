/*
 * stdp.c — STDP Implementation
 * Rethink AI — Phase 2
 */

#include "stdp.h"
#include <math.h>

STDPParams stdp_default_params(void) {
    STDPParams p;
    p.a_plus = 0.01f;
    p.a_minus = 0.012f;
    p.tau_plus = 20.0f;
    p.tau_minus = 20.0f;
    p.w_max = 1.0f;
    p.w_min = 0.0f;
    return p;
}

float stdp_weight_change(const STDPParams *p, float dt_ms) {
    if (dt_ms > 0) {
        /* Pre before post → LTP (strengthen) */
        return p->a_plus * expf(-dt_ms / p->tau_plus);
    } else if (dt_ms < 0) {
        /* Post before pre → LTD (weaken) */
        return -p->a_minus * expf(dt_ms / p->tau_minus);
    }
    return 0.0f;
}

void stdp_apply(const STDPParams *p, float *weight, float pre_time, float post_time) {
    float dt = post_time - pre_time;
    float dw = stdp_weight_change(p, dt);
    *weight += dw;
    if (*weight > p->w_max) *weight = p->w_max;
    if (*weight < p->w_min) *weight = p->w_min;
}
