# 🔨 Build: Hebbian Learning Network

## What We're Building

A network that **learns associations** using Hebbian learning. No backpropagation, no loss function, no gradient descent.

The network will:
1. Receive two patterns together
2. Strengthen connections between neurons that fire together
3. Later, given just ONE pattern, activate the associated pattern from memory

Like how smelling cookies instantly makes you think of your grandmother's kitchen.

---

## Step 1: The Hebbian Learning Rule (Code)

Create `src/learning/hebbian.h`:

```c
/*
 * hebbian.h — Hebbian Learning: "Fire together, wire together"
 * 
 * Rethink AI — Phase 2
 * 
 * Learning rules:
 *   1. Basic Hebbian: Δw = η × pre × post
 *   2. Oja's Rule:    Δw = η × post × (pre - w × post)  (self-normalizing)
 *   3. Weight decay:  w *= (1 - decay_rate)  (use it or lose it)
 */

#ifndef HEBBIAN_H
#define HEBBIAN_H

#include "../core/network.h"

/* ─── Learning Parameters ─── */

typedef struct {
    float learning_rate;    /* η: how fast to learn (e.g., 0.01) */
    float decay_rate;       /* How fast unused connections weaken (e.g., 0.001) */
    float w_max;            /* Maximum connection weight */
    float w_min;            /* Minimum connection weight */
    int use_oja;            /* 1 = use Oja's rule (self-normalizing), 0 = basic Hebbian */
} HebbianParams;

/* Get default Hebbian parameters */
HebbianParams hebbian_default_params(void);

/* 
 * Apply Hebbian learning to all synapses in the network.
 * Call this AFTER network_step() each time step.
 * 
 * Looks at which neurons fired, and strengthens connections
 * between co-firing neurons.
 */
void hebbian_learn(Network *net, const HebbianParams *params);

/*
 * Apply weight decay to all synapses.
 * Call this periodically (e.g., every 100 steps) to slowly
 * weaken connections that aren't being reinforced.
 * "Use it or lose it."
 */
void hebbian_decay(Network *net, float decay_rate);

/*
 * Normalize weights: prevent any single neuron's incoming
 * weights from growing too large. Like a "budget" — each neuron
 * has a fixed amount of connection strength to distribute.
 */
void hebbian_normalize(Network *net, float target_norm);

#endif /* HEBBIAN_H */
```

Create `src/learning/hebbian.c`:

```c
/*
 * hebbian.c — Hebbian Learning Implementation
 * 
 * Rethink AI — Phase 2
 */

#include "hebbian.h"
#include <math.h>
#include <stdio.h>

HebbianParams hebbian_default_params(void) {
    HebbianParams p;
    p.learning_rate = 0.01f;
    p.decay_rate = 0.001f;
    p.w_max = 5.0f;
    p.w_min = -5.0f;
    p.use_oja = 0;
    return p;
}

void hebbian_learn(Network *net, const HebbianParams *params) {
    /*
     * For each synapse: check if source and target both fired.
     * If yes → strengthen the connection.
     * 
     * Basic Hebbian:  Δw = η × pre_activity × post_activity
     * Oja's Rule:     Δw = η × post × (pre - w × post)
     */
    
    for (uint32_t s = 0; s < net->num_synapses; s++) {
        Synapse *syn = &net->synapses[s];
        
        /* Get the source and target neurons */
        Neuron *pre = &net->neurons[syn->source_id];
        Neuron *post = &net->neurons[syn->target_id];
        
        /* Activity: 1.0 if the neuron fired this step, 0.0 if not */
        float pre_activity = pre->spiked ? 1.0f : 0.0f;
        float post_activity = post->spiked ? 1.0f : 0.0f;
        
        /* Calculate weight change */
        float dw;
        
        if (params->use_oja) {
            /* Oja's Rule: self-normalizing Hebbian */
            /* Δw = η × post × (pre - w × post) */
            dw = params->learning_rate * post_activity 
                 * (pre_activity - syn->weight * post_activity);
        } else {
            /* Basic Hebbian: fire together, wire together */
            /* Δw = η × pre × post */
            dw = params->learning_rate * pre_activity * post_activity;
        }
        
        /* Apply weight change */
        syn->weight += dw;
        
        /* Clamp to bounds */
        if (syn->weight > params->w_max) syn->weight = params->w_max;
        if (syn->weight < params->w_min) syn->weight = params->w_min;
    }
}

void hebbian_decay(Network *net, float decay_rate) {
    /*
     * "Use it or lose it"
     * All weights shrink slightly toward 0.
     * Only connections that keep getting reinforced survive.
     */
    for (uint32_t s = 0; s < net->num_synapses; s++) {
        net->synapses[s].weight *= (1.0f - decay_rate);
    }
}

void hebbian_normalize(Network *net, float target_norm) {
    /*
     * For each neuron: normalize all incoming weights so that
     * the total incoming weight = target_norm.
     * 
     * This prevents any single neuron from having all its inputs
     * be super strong. Like a budget — distribute your connection
     * strength fairly.
     */
    for (uint32_t n = 0; n < net->num_neurons; n++) {
        /* Sum of absolute incoming weights for this neuron */
        float sum = 0.0f;
        int count = 0;
        
        for (uint32_t s = 0; s < net->num_synapses; s++) {
            if (net->synapses[s].target_id == n) {
                sum += fabsf(net->synapses[s].weight);
                count++;
            }
        }
        
        if (count == 0 || sum < 1e-6f) continue;
        
        /* Scale factor to normalize */
        float scale = target_norm / sum;
        
        for (uint32_t s = 0; s < net->num_synapses; s++) {
            if (net->synapses[s].target_id == n) {
                net->synapses[s].weight *= scale;
            }
        }
    }
}
```

---

## Step 2: The STDP Learning Rule

Create `src/learning/stdp.h`:

```c
/*
 * stdp.h — Spike-Timing Dependent Plasticity
 * 
 * Rethink AI — Phase 2
 * 
 * Learning based on the TIMING between spikes:
 *   - Source fires BEFORE target → strengthen (the source "caused" the target to fire)
 *   - Source fires AFTER target → weaken (the source is irrelevant)
 *   - Closer in time → bigger change
 */

#ifndef STDP_H
#define STDP_H

#include "../core/network.h"

typedef struct {
    float a_plus;       /* Max weight increase (LTP amplitude) */
    float a_minus;      /* Max weight decrease (LTD amplitude) */
    float tau_plus;     /* Time constant for LTP (ms) */
    float tau_minus;    /* Time constant for LTD (ms) */
    float w_max;        /* Maximum weight */
    float w_min;        /* Minimum weight */
} STDPParams;

/* Get default STDP parameters */
STDPParams stdp_default_params(void);

/*
 * Apply STDP learning to all synapses.
 * Uses last_spike_time from each neuron to calculate timing.
 * Call this AFTER network_step().
 */
void stdp_learn(Network *net, const STDPParams *params);

#endif /* STDP_H */
```

Create `src/learning/stdp.c`:

```c
/*
 * stdp.c — Spike-Timing Dependent Plasticity Implementation
 * 
 * Rethink AI — Phase 2
 */

#include "stdp.h"
#include <math.h>

STDPParams stdp_default_params(void) {
    STDPParams p;
    p.a_plus = 0.01f;      /* Strengthening amplitude */
    p.a_minus = 0.012f;    /* Weakening amplitude (slightly stronger → prevents runaway) */
    p.tau_plus = 20.0f;    /* LTP time window (ms) */
    p.tau_minus = 20.0f;   /* LTD time window (ms) */
    p.w_max = 5.0f;
    p.w_min = 0.0f;
    return p;
}

void stdp_learn(Network *net, const STDPParams *params) {
    /*
     * For each synapse where either the source OR target fired this step:
     *   1. Calculate Δt = t_post - t_pre (timing difference)
     *   2. If Δt > 0 (pre fired first): strengthen (LTP)
     *   3. If Δt < 0 (post fired first): weaken (LTD)
     *   4. Magnitude depends on |Δt| (closer = bigger change)
     */
    
    for (uint32_t s = 0; s < net->num_synapses; s++) {
        Synapse *syn = &net->synapses[s];
        Neuron *pre = &net->neurons[syn->source_id];
        Neuron *post = &net->neurons[syn->target_id];
        
        float dw = 0.0f;
        
        /* Only update if at least one neuron fired this step */
        if (post->spiked && pre->last_spike_time > -999.0f) {
            /* 
             * Post-synaptic neuron just fired.
             * Look at when the pre-synaptic neuron last fired.
             * Δt = t_post - t_pre
             */
            float dt = post->last_spike_time - pre->last_spike_time;
            
            if (dt > 0.0f && dt < 5.0f * params->tau_plus) {
                /* Pre fired BEFORE post → LTP (strengthen) */
                /* Δw = A+ × exp(-Δt / τ+) */
                dw = params->a_plus * expf(-dt / params->tau_plus);
            }
        }
        
        if (pre->spiked && post->last_spike_time > -999.0f) {
            /*
             * Pre-synaptic neuron just fired.
             * Look at when the post-synaptic neuron last fired.
             * This gives us the "pre fired AFTER post" case.
             */
            float dt = post->last_spike_time - pre->last_spike_time;
            
            if (dt < 0.0f && dt > -5.0f * params->tau_minus) {
                /* Pre fired AFTER post → LTD (weaken) */
                /* Δw = -A- × exp(Δt / τ-) */
                dw += -params->a_minus * expf(dt / params->tau_minus);
            }
        }
        
        /* Apply weight change */
        if (dw != 0.0f) {
            syn->weight += dw;
            
            /* Clamp to bounds */
            if (syn->weight > params->w_max) syn->weight = params->w_max;
            if (syn->weight < params->w_min) syn->weight = params->w_min;
        }
    }
}
```

---

## Step 3: Demo — Learning an Association

Create `src/test_hebbian.c`:

```c
/*
 * test_hebbian.c — Demonstrate Hebbian associative learning
 * 
 * Setup:
 *   - 10 neurons: 5 for "Pattern A", 5 for "Pattern B"
 *   - Initially: weak random connections between A and B groups
 *   - Training: activate A and B together many times
 *   - Test: activate ONLY A → does B activate? (association!)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "core/neuron.h"
#include "core/synapse.h"
#include "core/network.h"
#include "learning/hebbian.h"

int main(void) {
    srand((unsigned)time(NULL));
    
    printf("=== RETHINK AI — Phase 2: Hebbian Learning Demo ===\n\n");
    
    /* Create network: 10 neurons, 0.1ms time step */
    /* Neurons 0-4 = Pattern A group */
    /* Neurons 5-9 = Pattern B group */
    Network *net = network_create(10, 0.1f);
    
    /* Connect A→B with weak initial weights */
    printf("Setting up connections (A→B, weak initial weights)...\n");
    for (uint32_t a = 0; a < 5; a++) {
        for (uint32_t b = 5; b < 10; b++) {
            float weak_weight = 0.1f + 0.05f * ((float)rand() / RAND_MAX);
            network_connect(net, a, b, weak_weight, 1.0f);
        }
    }
    
    HebbianParams hparams = hebbian_default_params();
    hparams.learning_rate = 0.02f;
    
    /* ─── BEFORE TRAINING: Test if A activates B ─── */
    printf("\n--- BEFORE TRAINING ---\n");
    printf("Activating Pattern A (neurons 0-4) only...\n");
    
    for (uint32_t a = 0; a < 5; a++) {
        network_set_input(net, a, 3.0f);  /* Strong input to A */
    }
    
    /* Run for 50ms */
    int b_spikes_before = 0;
    for (int step = 0; step < 500; step++) {
        network_step(net);
        for (uint32_t b = 5; b < 10; b++) {
            if (net->neurons[b].spiked) b_spikes_before++;
        }
    }
    printf("Pattern B spikes (should be LOW): %d\n", b_spikes_before);
    
    /* Reset */
    for (uint32_t i = 0; i < 10; i++) neuron_reset(&net->neurons[i]);
    net->current_time = 0; net->num_pending = 0;
    network_clear_input(net);
    
    /* ─── TRAINING: Activate A and B together ─── */
    printf("\n--- TRAINING (100 repetitions) ---\n");
    printf("Activating A AND B together, learning with Hebbian rule...\n");
    
    for (int rep = 0; rep < 100; rep++) {
        /* Reset neurons but keep learned weights */
        for (uint32_t i = 0; i < 10; i++) neuron_reset(&net->neurons[i]);
        net->current_time = 0; net->num_pending = 0;
        
        /* Activate BOTH A and B */
        for (uint32_t a = 0; a < 5; a++) network_set_input(net, a, 3.0f);
        for (uint32_t b = 5; b < 10; b++) network_set_input(net, b, 3.0f);
        
        /* Run for 20ms with learning */
        for (int step = 0; step < 200; step++) {
            network_step(net);
            hebbian_learn(net, &hparams);
        }
        
        network_clear_input(net);
        
        if ((rep + 1) % 20 == 0) {
            /* Print average weight from A to B */
            float avg_weight = 0;
            int count = 0;
            for (uint32_t s = 0; s < net->num_synapses; s++) {
                if (net->synapses[s].source_id < 5 && net->synapses[s].target_id >= 5) {
                    avg_weight += net->synapses[s].weight;
                    count++;
                }
            }
            printf("  Rep %3d: Avg A→B weight = %.4f (started ~0.12)\n", 
                   rep + 1, count > 0 ? avg_weight / count : 0);
        }
    }
    
    /* ─── AFTER TRAINING: Test if A activates B ─── */
    printf("\n--- AFTER TRAINING ---\n");
    printf("Activating Pattern A (neurons 0-4) only...\n");
    printf("(Can it activate B through learned associations?)\n");
    
    /* Reset neurons */
    for (uint32_t i = 0; i < 10; i++) neuron_reset(&net->neurons[i]);
    net->current_time = 0; net->num_pending = 0;
    
    /* Only activate A this time! */
    for (uint32_t a = 0; a < 5; a++) network_set_input(net, a, 3.0f);
    
    /* Run for 50ms */
    int b_spikes_after = 0;
    for (int step = 0; step < 500; step++) {
        network_step(net);
        for (uint32_t b = 5; b < 10; b++) {
            if (net->neurons[b].spiked) b_spikes_after++;
        }
    }
    printf("Pattern B spikes (should be HIGHER than before): %d\n", b_spikes_after);
    
    /* Result */
    printf("\n═══════════════════════════════════\n");
    printf("B spikes BEFORE training: %d\n", b_spikes_before);
    printf("B spikes AFTER training:  %d\n", b_spikes_after);
    if (b_spikes_after > b_spikes_before * 2) {
        printf("✓ SUCCESS! The network learned the association!\n");
        printf("  Activating A now automatically activates B.\n");
        printf("  Like how smelling cookies → thinking of grandma's kitchen.\n");
    } else {
        printf("△ Marginal improvement. Try adjusting learning_rate or repetitions.\n");
    }
    printf("═══════════════════════════════════\n");
    
    network_destroy(net);
    return 0;
}
```

### Compile and run:
```bash
gcc -o test_hebbian test_hebbian.c core/neuron.c core/synapse.c core/network.c \
    learning/hebbian.c -lm -Wall -Wextra -std=c11 -O2
./test_hebbian
```

---

## What You Just Built

✅ **Hebbian learning** — connections strengthen when neurons co-fire  
✅ **Weight normalization** — prevents runaway weight growth  
✅ **Weight decay** — unused connections weaken ("use it or lose it")  
✅ **Associative learning** — two patterns become linked through co-occurrence  
✅ **STDP** — timing-based learning for sequences and causation  

**No backpropagation. No gradient descent. No loss function. Just co-firing.**

Phase 2 is complete. Tag as **V2**.

---

*Next: [05_build_stdp_learning.md](05_build_stdp_learning.md) — Build an STDP demo that learns sequences*
