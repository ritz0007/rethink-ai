# 🔨 Build: Connect Neurons Into a Network

## What We're Building

A single neuron is like a single person shouting into the void. The magic happens when neurons **talk to each other**. In this guide, we'll:

1. Build **synapses** (connections between neurons)
2. Build a **network** (groups of connected neurons)
3. Feed input and watch **firing patterns emerge**

---

## The Synapse — Connections Between Neurons

### 🧠 Brain Science

When neuron A fires, the spike travels down its axon to the synapse — the junction where A meets neuron B. The signal crosses a tiny gap using chemicals (neurotransmitters). This signal can be:

- **Excitatory** — pushes neuron B toward firing (adds charge)
- **Inhibitory** — pushes neuron B away from firing (subtracts charge)

Key properties of a real synapse:
- **Weight (strength)** — how strongly A affects B. This changes with learning!
- **Delay** — the signal takes time to travel. Not instant.
- **Type** — excitatory (+) or inhibitory (-)

### Code: `src/core/synapse.h`

```c
/*
 * synapse.h — Connections between neurons
 * 
 * Rethink AI — Phase 1
 * 
 * A synapse carries a signal from one neuron (source) to another (target).
 * The signal has:
 *   - weight: how strong the connection is (positive = excitatory, negative = inhibitory)
 *   - delay: how long it takes for the signal to arrive (in ms)
 */

#ifndef SYNAPSE_H
#define SYNAPSE_H

#include <stdint.h>

typedef struct {
    uint32_t source_id;   /* ID of the neuron that sends the signal */
    uint32_t target_id;   /* ID of the neuron that receives the signal */
    float weight;         /* Strength: positive = excitatory, negative = inhibitory */
    float delay;          /* Transmission delay in ms (signal travel time) */
} Synapse;

/* Create a synapse between two neurons */
Synapse synapse_create(uint32_t source_id, uint32_t target_id, 
                       float weight, float delay);

/* Print synapse info */
void synapse_print(const Synapse *s);

#endif /* SYNAPSE_H */
```

### Code: `src/core/synapse.c`

```c
#include "synapse.h"
#include <stdio.h>

Synapse synapse_create(uint32_t source_id, uint32_t target_id, 
                       float weight, float delay) {
    Synapse s;
    s.source_id = source_id;
    s.target_id = target_id;
    s.weight = weight;
    s.delay = (delay > 0.0f) ? delay : 1.0f;  /* Minimum 1ms delay */
    return s;
}

void synapse_print(const Synapse *s) {
    printf("Synapse: Neuron #%u → Neuron #%u | weight: %+.3f | delay: %.1f ms | %s\n",
           s->source_id, s->target_id, s->weight, s->delay,
           s->weight >= 0 ? "EXCITATORY" : "INHIBITORY");
}
```

---

## The Network — Neurons Working Together

### 🧠 Brain Science

The brain isn't organized in neat layers like a standard neural network. Neurons connect in complex, sometimes random patterns. Some regions are densely connected (like the cortex), others have long-distance connections (like the corpus callosum connecting the two brain hemispheres).

For now, we'll build a simple network where:
- Any neuron can connect to any other neuron
- Connections can be excitatory or inhibitory
- We track pending spikes (because signals have delay)

### Code: `src/core/network.h`

```c
/*
 * network.h — A network of spiking neurons connected by synapses
 * 
 * Rethink AI — Phase 1
 * 
 * This is NOT a layer-by-layer feedforward network.
 * Any neuron can connect to any other neuron (including itself).
 * Signals have delays. The network runs in real time.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "neuron.h"
#include "synapse.h"
#include <stdint.h>

/* A pending spike: a signal that's been sent but hasn't arrived yet */
typedef struct {
    uint32_t target_id;     /* Which neuron will receive this */
    float weight;           /* How strong the signal is */
    float arrival_time;     /* When the signal will arrive (ms) */
} PendingSpike;

/* The network */
typedef struct {
    /* Neurons */
    Neuron *neurons;        /* Array of all neurons */
    uint32_t num_neurons;   /* How many neurons */
    
    /* Synapses (connections) */
    Synapse *synapses;      /* Array of all connections */
    uint32_t num_synapses;  /* How many connections */
    uint32_t max_synapses;  /* Max capacity (for dynamic growth) */
    
    /* Pending spikes (signals in transit) */
    PendingSpike *pending;  /* Spikes that haven't arrived yet */
    uint32_t num_pending;   /* How many pending spikes */
    uint32_t max_pending;   /* Max capacity */
    
    /* Simulation state */
    float current_time;     /* Current simulation time (ms) */
    float dt;               /* Time step (ms) */
    
    /* External input (for feeding signals into the network) */
    float *external_input;  /* External current for each neuron */
} Network;

/* ─── Creation & Destruction ─── */

/* Create a network with N neurons */
Network *network_create(uint32_t num_neurons, float dt);

/* Free all memory */
void network_destroy(Network *net);

/* ─── Building the Network ─── */

/* Connect two neurons with a synapse */
void network_connect(Network *net, uint32_t source, uint32_t target, 
                     float weight, float delay);

/* Connect randomly: each pair has 'probability' chance of connecting */
void network_connect_random(Network *net, float probability, 
                            float weight_mean, float weight_std,
                            float delay);

/* Set external input current for a specific neuron */
void network_set_input(Network *net, uint32_t neuron_id, float current);

/* Set external input for all neurons */
void network_set_input_all(Network *net, float current);

/* Clear all external inputs */
void network_clear_input(Network *net);

/* ─── Simulation ─── */

/* Advance the entire network by one time step */
void network_step(Network *net);

/* Run the network for a given duration (ms) */
void network_run(Network *net, float duration_ms);

/* ─── Output & Debugging ─── */

/* Print which neurons fired this time step */
void network_print_spikes(const Network *net);

/* Print network summary */
void network_print_summary(const Network *net);

/* Print ASCII raster plot of all spikes over a time window */
void network_print_raster(const Network *net, int *spike_log, 
                          uint32_t num_steps);

#endif /* NETWORK_H */
```

### Code: `src/core/network.c`

```c
/*
 * network.c — Spiking Neural Network Implementation
 * 
 * Rethink AI — Phase 1
 * 
 * How it works:
 * 1. For each time step:
 *    a. Deliver any pending spikes that have arrived
 *    b. Add external input
 *    c. Update each neuron (leak, accumulate, threshold check)
 *    d. For neurons that fired, create new pending spikes for all their targets
 */

#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Simple random float between -1 and 1 */
static float rand_float(void) {
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

/* Gaussian random (Box-Muller) */
static float rand_gaussian(float mean, float std) {
    float u1 = (float)rand() / (float)RAND_MAX;
    float u2 = (float)rand() / (float)RAND_MAX;
    if (u1 < 1e-10f) u1 = 1e-10f;
    float z = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * 3.14159265f * u2);
    return mean + std * z;
}

/* ─── Creation & Destruction ─── */

Network *network_create(uint32_t num_neurons, float dt) {
    Network *net = (Network *)calloc(1, sizeof(Network));
    if (!net) {
        fprintf(stderr, "ERROR: Failed to allocate network\n");
        return NULL;
    }
    
    net->num_neurons = num_neurons;
    net->dt = dt;
    net->current_time = 0.0f;
    
    /* Allocate neurons */
    net->neurons = (Neuron *)calloc(num_neurons, sizeof(Neuron));
    for (uint32_t i = 0; i < num_neurons; i++) {
        net->neurons[i] = neuron_create(i);
    }
    
    /* Allocate synapses (start with capacity for 10× neurons) */
    net->max_synapses = num_neurons * 10;
    net->synapses = (Synapse *)calloc(net->max_synapses, sizeof(Synapse));
    net->num_synapses = 0;
    
    /* Allocate pending spikes buffer */
    net->max_pending = num_neurons * 20;
    net->pending = (PendingSpike *)calloc(net->max_pending, sizeof(PendingSpike));
    net->num_pending = 0;
    
    /* Allocate external input buffer */
    net->external_input = (float *)calloc(num_neurons, sizeof(float));
    
    return net;
}

void network_destroy(Network *net) {
    if (!net) return;
    free(net->neurons);
    free(net->synapses);
    free(net->pending);
    free(net->external_input);
    free(net);
}

/* ─── Building ─── */

void network_connect(Network *net, uint32_t source, uint32_t target, 
                     float weight, float delay) {
    if (net->num_synapses >= net->max_synapses) {
        /* Grow the synapse array */
        net->max_synapses *= 2;
        net->synapses = (Synapse *)realloc(net->synapses, 
                                           net->max_synapses * sizeof(Synapse));
    }
    
    net->synapses[net->num_synapses] = synapse_create(source, target, weight, delay);
    net->num_synapses++;
}

void network_connect_random(Network *net, float probability,
                            float weight_mean, float weight_std,
                            float delay) {
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        for (uint32_t j = 0; j < net->num_neurons; j++) {
            if (i == j) continue;  /* No self-connections (for now) */
            
            float r = (float)rand() / (float)RAND_MAX;
            if (r < probability) {
                float w = rand_gaussian(weight_mean, weight_std);
                network_connect(net, i, j, w, delay);
            }
        }
    }
}

void network_set_input(Network *net, uint32_t neuron_id, float current) {
    if (neuron_id < net->num_neurons) {
        net->external_input[neuron_id] = current;
    }
}

void network_set_input_all(Network *net, float current) {
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        net->external_input[i] = current;
    }
}

void network_clear_input(Network *net) {
    memset(net->external_input, 0, net->num_neurons * sizeof(float));
}

/* ─── Simulation ─── */

static void add_pending_spike(Network *net, uint32_t target_id, 
                               float weight, float arrival_time) {
    if (net->num_pending >= net->max_pending) {
        net->max_pending *= 2;
        net->pending = (PendingSpike *)realloc(net->pending,
                                                net->max_pending * sizeof(PendingSpike));
    }
    
    PendingSpike ps;
    ps.target_id = target_id;
    ps.weight = weight;
    ps.arrival_time = arrival_time;
    net->pending[net->num_pending] = ps;
    net->num_pending++;
}

void network_step(Network *net) {
    /* 
     * Step 1: Collect input for each neuron
     * Start with external input, then add arrived pending spikes
     */
    float *total_input = (float *)calloc(net->num_neurons, sizeof(float));
    
    /* Add external input */
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        total_input[i] = net->external_input[i];
    }
    
    /* 
     * Step 2: Deliver pending spikes that have arrived
     * (Remove delivered spikes from the pending list)
     */
    uint32_t new_pending_count = 0;
    for (uint32_t i = 0; i < net->num_pending; i++) {
        PendingSpike *ps = &net->pending[i];
        
        if (ps->arrival_time <= net->current_time) {
            /* This spike has arrived — deliver it */
            if (ps->target_id < net->num_neurons) {
                total_input[ps->target_id] += ps->weight;
            }
            /* Don't keep it (effectively removes from list) */
        } else {
            /* This spike hasn't arrived yet — keep it */
            net->pending[new_pending_count] = net->pending[i];
            new_pending_count++;
        }
    }
    net->num_pending = new_pending_count;
    
    /* 
     * Step 3: Update each neuron
     */
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        int fired = neuron_step(&net->neurons[i], total_input[i], 
                                net->dt, net->current_time);
        
        if (fired) {
            /* 
             * Step 4: This neuron fired — schedule spikes to all its targets
             */
            for (uint32_t s = 0; s < net->num_synapses; s++) {
                if (net->synapses[s].source_id == i) {
                    float arrival = net->current_time + net->synapses[s].delay;
                    add_pending_spike(net, 
                                      net->synapses[s].target_id,
                                      net->synapses[s].weight,
                                      arrival);
                }
            }
        }
    }
    
    free(total_input);
    
    /* Advance time */
    net->current_time += net->dt;
}

void network_run(Network *net, float duration_ms) {
    float end_time = net->current_time + duration_ms;
    while (net->current_time < end_time) {
        network_step(net);
    }
}

/* ─── Output ─── */

void network_print_spikes(const Network *net) {
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        if (net->neurons[i].spiked) {
            printf("  t=%.1fms: Neuron #%u FIRED (total: %u spikes)\n",
                   net->current_time, i, net->neurons[i].spike_count);
        }
    }
}

void network_print_summary(const Network *net) {
    printf("\n=== Network Summary ===\n");
    printf("Neurons: %u\n", net->num_neurons);
    printf("Synapses: %u\n", net->num_synapses);
    printf("Current time: %.1f ms\n", net->current_time);
    printf("Pending spikes: %u\n", net->num_pending);
    
    uint32_t total_spikes = 0;
    uint32_t active_neurons = 0;
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        total_spikes += net->neurons[i].spike_count;
        if (net->neurons[i].spike_count > 0) active_neurons++;
    }
    
    printf("Total spikes: %u\n", total_spikes);
    printf("Active neurons: %u / %u (%.1f%%)\n", 
           active_neurons, net->num_neurons,
           100.0f * active_neurons / net->num_neurons);
    printf("=======================\n");
}
```

---

## Test the Network

Create `src/test_network.c`:

```c
/*
 * test_network.c — Test a small spiking neural network
 * 
 * Creates 20 neurons, connects them randomly, and feeds input.
 * Watch activity cascade through the network!
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "core/neuron.h"
#include "core/synapse.h"
#include "core/network.h"

int main(void) {
    srand((unsigned)time(NULL));
    
    printf("=== RETHINK AI — Phase 1: Network Test ===\n\n");
    
    /* Create a network of 20 neurons, 0.1 ms time step */
    Network *net = network_create(20, 0.1f);
    
    /* Connect randomly: 20% chance any pair connects */
    /* Excitatory connections (positive weight) */
    network_connect_random(net, 0.2f, 1.5f, 0.5f, 1.0f);
    
    /* Add some inhibitory connections (negative weight) */
    network_connect_random(net, 0.05f, -2.0f, 0.3f, 1.0f);
    
    network_print_summary(net);
    
    /* ─── Experiment 1: Feed input to just ONE neuron ─── */
    printf("\n--- Experiment 1: Input to neuron #0 only ---\n");
    printf("(Does activity spread through the network?)\n\n");
    
    network_set_input(net, 0, 3.0f);  /* Strong input to neuron 0 */
    
    /* Run for 100ms, logging spikes */
    printf("Time   Active Neurons\n");
    printf("─────  ──────────────\n");
    
    for (int step = 0; step < 1000; step++) {  /* 1000 steps × 0.1ms = 100ms */
        network_step(net);
        
        /* Print every 5ms */
        if (step % 50 == 0) {
            int active = 0;
            for (uint32_t i = 0; i < net->num_neurons; i++) {
                if (net->neurons[i].spiked) active++;
            }
            if (active > 0) {
                printf("%5.1fms  %d neurons firing: ", net->current_time, active);
                for (uint32_t i = 0; i < net->num_neurons; i++) {
                    if (net->neurons[i].spiked) printf("#%u ", i);
                }
                printf("\n");
            }
        }
    }
    
    network_print_summary(net);
    
    /* ─── Experiment 2: Spike raster (ASCII art) ─── */
    printf("\n--- Experiment 2: Spike Raster (200ms) ---\n");
    printf("Each row = one neuron, each '|' = a spike, '.' = silence\n\n");
    
    /* Reset everything */
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        neuron_reset(&net->neurons[i]);
    }
    net->current_time = 0.0f;
    net->num_pending = 0;
    
    /* Feed input to neurons 0, 1, 2 */
    network_set_input(net, 0, 3.0f);
    network_set_input(net, 1, 2.5f);
    network_set_input(net, 2, 2.0f);
    
    int duration_steps = 2000;  /* 200ms */
    int bucket_size = 10;       /* Group 10 steps (1ms) into one column */
    int num_buckets = duration_steps / bucket_size;
    
    /* Allocate spike log: [neuron][time_bucket] */
    int *spike_log = (int *)calloc(net->num_neurons * num_buckets, sizeof(int));
    
    for (int step = 0; step < duration_steps; step++) {
        network_step(net);
        int bucket = step / bucket_size;
        
        for (uint32_t i = 0; i < net->num_neurons; i++) {
            if (net->neurons[i].spiked) {
                spike_log[i * num_buckets + bucket] = 1;
            }
        }
    }
    
    /* Print the raster */
    printf("N\\T ");
    for (int t = 0; t < num_buckets; t += 10) {
        printf("%-10d", t);
    }
    printf("\n");
    
    for (uint32_t i = 0; i < net->num_neurons; i++) {
        printf("%2u: ", i);
        for (int t = 0; t < num_buckets; t++) {
            printf("%c", spike_log[i * num_buckets + t] ? '|' : '.');
        }
        printf("  (%u spikes)\n", net->neurons[i].spike_count);
    }
    
    free(spike_log);
    
    network_print_summary(net);
    
    /* Cleanup */
    network_destroy(net);
    
    printf("\n=== Phase 1 Complete: You have a working spiking neural network! ===\n");
    return 0;
}
```

### Compile and run:

```bash
gcc -o test_network test_network.c core/neuron.c core/synapse.c core/network.c \
    -lm -Wall -Wextra -std=c11 -O2
./test_network
```

---

## What You Should See

A **spike raster plot** — the fundamental tool for studying spiking networks:

```
N\T 0         10        20        30        
 0: ..........|..........|..........|..  (15 spikes)
 1: ...........|..........|...........|  (12 spikes)
 2: .............|..........|..........  (10 spikes)
 3: ...............|.......|...........  (8 spikes)
 4: .................|..........|......  (5 spikes)
 ...
19: ..................................  (0 spikes)
```

Neurons 0, 1, 2 fire regularly (they have direct input). Other neurons fire BECAUSE of signals spreading through the network. Some neurons may never fire — they didn't get enough input.

**This is emergent behavior.** No one programmed "neuron 7 should fire at time 23ms." It happened because of how the network is wired.

---

## What You Just Built

✅ **Synapses** with weight, delay, and excitatory/inhibitory types  
✅ **A network** where any neuron can connect to any other  
✅ **Signal propagation** with realistic delays  
✅ **Emergent firing patterns** from simple rules  
✅ **Both excitatory and inhibitory** connections  

You now have a small brain. Not a "neural network" — an actual simulation of neurons firing, signals traveling through synapses, and activity spreading across a network.

**Phase 1 is complete.** Tag this as V1.

---

*Next phase: [Phase 2 — Learning Without Backprop](../02_learning_without_backprop/01_brain_hebbian_learning.md)*  
*The network fires, but it doesn't LEARN yet. That changes now.*
