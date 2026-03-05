# 🔨 Build: Your First Spiking Neuron in C

## What We're Building

A **Leaky Integrate-and-Fire (LIF) neuron** — the simplest model that captures real neuron behavior:

- Accumulates charge from inputs
- Leaks charge over time  
- Fires a spike when threshold is crossed
- Goes refractory after firing

By the end of this guide, you'll have a single neuron that you can feed input to and watch it spike.

---

## Step 1: Define the Neuron (the struct)

Create the file `src/core/neuron.h`:

```c
/*
 * neuron.h — Leaky Integrate-and-Fire Spiking Neuron
 * 
 * Rethink AI — Phase 1: The Neuron
 * 
 * This is NOT an artificial neuron (weights × inputs + bias).
 * This is a model of a real biological neuron that:
 *   - accumulates electrical charge over time
 *   - leaks charge (like a bucket with a hole)
 *   - fires a spike when charge exceeds threshold
 *   - goes silent (refractory) after firing
 */

#ifndef NEURON_H
#define NEURON_H

#include <stdint.h>

/* ─── Neuron Parameters ─── */

typedef struct {
    /* Voltage levels (in millivolts, mV) */
    float v_rest;         /* Resting voltage: -70 mV (baseline, doing nothing) */
    float v_threshold;    /* Threshold: -55 mV (fire when exceeded) */
    float v_reset;        /* Reset voltage: -75 mV (after firing, goes below rest) */

    /* Timing (in milliseconds, ms) */
    float tau_m;          /* Membrane time constant: controls leak speed */
                          /* Big tau = slow leak, small tau = fast leak */
    float t_refractory;   /* Refractory duration: can't fire during this time */
    
    /* Input scaling */
    float resistance;     /* Membrane resistance: how much input affects voltage */
} NeuronParams;

/* ─── Neuron State ─── */

typedef struct {
    /* Parameters (set once at creation) */
    NeuronParams params;

    /* Dynamic state (changes every time step) */
    float voltage;        /* Current membrane potential (mV) */
    float refractory_timer; /* Time remaining in refractory period (ms) */
                            /* 0 = ready to fire, >0 = still refractory */
    
    /* Output */
    int spiked;           /* 1 if neuron fired this time step, 0 if not */

    /* History tracking */
    uint32_t spike_count; /* Total spikes since creation */
    float last_spike_time; /* When was the last spike? (ms) */
    
    /* Identity */
    uint32_t id;          /* Unique neuron identifier */
} Neuron;

/* ─── Functions ─── */

/* Create a neuron with default biological parameters */
Neuron neuron_create(uint32_t id);

/* Create a neuron with custom parameters */
Neuron neuron_create_custom(uint32_t id, NeuronParams params);

/* Get default biological parameters */
NeuronParams neuron_default_params(void);

/* 
 * Advance the neuron by one time step.
 * 
 * input_current: total incoming signal from other neurons (nanoamps)
 * dt: size of time step (milliseconds)
 * current_time: current simulation time (milliseconds)
 * 
 * Returns: 1 if the neuron fired (spiked), 0 if not
 */
int neuron_step(Neuron *n, float input_current, float dt, float current_time);

/* Reset neuron to resting state (like it was just created) */
void neuron_reset(Neuron *n);

/* Print neuron state to console (for debugging) */
void neuron_print(const Neuron *n);

#endif /* NEURON_H */
```

### What each field means (in human terms):

- **`voltage`** — How "full" the water bucket is right now
- **`v_rest`** — The level water settles to when nothing is happening (-70)
- **`v_threshold`** — The rim of the bucket — overflow when reached (-55)
- **`v_reset`** — After overflow, drain to this level (-75, below rest)
- **`tau_m`** — How big is the hole in the bucket? (controls leak speed)
- **`refractory_timer`** — Countdown timer: "I just fired, can't fire yet"
- **`spiked`** — Did I fire THIS time step? (1 = yes, 0 = no)

---

## Step 2: Implement the Neuron (the logic)

Create the file `src/core/neuron.c`:

```c
/*
 * neuron.c — Leaky Integrate-and-Fire Neuron Implementation
 * 
 * Rethink AI — Phase 1
 * 
 * The core equation:
 *   dV/dt = (-(V - V_rest) + R * I) / tau_m
 * 
 * In English:
 *   "Voltage changes because of leak (pulls toward rest) 
 *    and input current (pushes voltage up or down).
 *    Tau controls how fast this all happens."
 */

#include "neuron.h"
#include <stdio.h>

/* ─── Default Parameters ─── */

NeuronParams neuron_default_params(void) {
    NeuronParams p;
    p.v_rest       = -70.0f;   /* Resting voltage (mV) */
    p.v_threshold  = -55.0f;   /* Firing threshold (mV) */
    p.v_reset      = -75.0f;   /* Post-spike reset (mV) */
    p.tau_m        = 20.0f;    /* Membrane time constant (ms) */
    p.t_refractory = 2.0f;     /* Refractory period (ms) */
    p.resistance   = 10.0f;    /* Membrane resistance (MΩ) */
    return p;
}

/* ─── Creation ─── */

Neuron neuron_create(uint32_t id) {
    return neuron_create_custom(id, neuron_default_params());
}

Neuron neuron_create_custom(uint32_t id, NeuronParams params) {
    Neuron n;
    n.params = params;
    n.voltage = params.v_rest;    /* Start at resting voltage */
    n.refractory_timer = 0.0f;    /* Not refractory */
    n.spiked = 0;                 /* Not firing */
    n.spike_count = 0;
    n.last_spike_time = -1000.0f; /* Never fired yet */
    n.id = id;
    return n;
}

/* ─── The Main Simulation Step ─── */

int neuron_step(Neuron *n, float input_current, float dt, float current_time) {
    n->spiked = 0;  /* Reset spike flag */
    
    /* ─── REFRACTORY CHECK ─── */
    /* If we just fired, we can't do anything yet */
    if (n->refractory_timer > 0.0f) {
        n->refractory_timer -= dt;
        /* During refractory, voltage stays at reset */
        n->voltage = n->params.v_reset;
        return 0;  /* Did not fire */
    }
    
    /* ─── LEAK + INPUT ─── */
    /* 
     * The LIF equation (Euler method):
     *   dV = (-(V - V_rest) + R * I) / tau_m * dt
     * 
     * First part:  -(V - V_rest)  → pulls voltage TOWARD rest (the leak)
     *   If V > V_rest (too high), this is negative → pulls down
     *   If V < V_rest (too low), this is positive → pulls up
     * 
     * Second part: R * I  → input current pushes voltage
     *   Positive current → pushes up
     *   Negative current → pushes down (inhibition!)
     */
    float dv = (-(n->voltage - n->params.v_rest) 
                + n->params.resistance * input_current) 
               / n->params.tau_m * dt;
    
    n->voltage += dv;
    
    /* ─── THRESHOLD CHECK ─── */
    /* Did we accumulate enough charge to fire? */
    if (n->voltage >= n->params.v_threshold) {
        /* FIRE! */
        n->spiked = 1;
        n->spike_count++;
        n->last_spike_time = current_time;
        
        /* Reset voltage (drops below resting — the undershoot) */
        n->voltage = n->params.v_reset;
        
        /* Start refractory period */
        n->refractory_timer = n->params.t_refractory;
        
        return 1;  /* We fired! */
    }
    
    return 0;  /* Did not fire */
}

/* ─── Reset ─── */

void neuron_reset(Neuron *n) {
    n->voltage = n->params.v_rest;
    n->refractory_timer = 0.0f;
    n->spiked = 0;
    n->spike_count = 0;
    n->last_spike_time = -1000.0f;
}

/* ─── Debug Print ─── */

void neuron_print(const Neuron *n) {
    printf("Neuron #%u | V: %7.2f mV | Spikes: %u | %s | %s\n",
           n->id,
           n->voltage,
           n->spike_count,
           n->spiked ? "** FIRING **" : "            ",
           n->refractory_timer > 0 ? "REFRACTORY" : "READY     ");
}
```

---

## Step 3: Test Your Neuron

Create `src/test_neuron.c`:

```c
/*
 * test_neuron.c — Test the spiking neuron
 * 
 * Feeds a constant current into a single neuron and watches it fire.
 * You should see the voltage rise, spike, reset, refractory, rise again...
 */

#include <stdio.h>
#include "core/neuron.h"

int main(void) {
    printf("=== RETHINK AI — Phase 1: Spiking Neuron Test ===\n\n");
    
    /* Create a neuron with default biological parameters */
    Neuron n = neuron_create(1);
    
    /* Simulation settings */
    float dt = 0.1f;           /* Time step: 0.1 ms */
    float total_time = 100.0f; /* Run for 100 ms */
    float input_current = 2.0f; /* Constant input: 2 nanoamps */
    
    printf("Parameters:\n");
    printf("  V_rest = %.1f mV\n", n.params.v_rest);
    printf("  V_threshold = %.1f mV\n", n.params.v_threshold);
    printf("  V_reset = %.1f mV\n", n.params.v_reset);
    printf("  tau_m = %.1f ms\n", n.params.tau_m);
    printf("  Refractory = %.1f ms\n", n.params.t_refractory);
    printf("  Input current = %.1f nA\n\n", input_current);
    
    printf("Time(ms)  Voltage(mV)  Event\n");
    printf("────────  ───────────  ─────\n");
    
    int total_spikes = 0;
    
    for (float t = 0.0f; t < total_time; t += dt) {
        int fired = neuron_step(&n, input_current, dt, t);
        
        /* Print every 1ms (every 10 steps) or when firing */
        int print_step = ((int)(t * 10)) % 10 == 0;
        
        if (fired) {
            printf("%7.1f   %8.2f     *** SPIKE! ***\n", t, n.voltage);
            total_spikes++;
        } else if (print_step) {
            printf("%7.1f   %8.2f     %s\n", t, n.voltage,
                   n.refractory_timer > 0 ? "(refractory)" : "");
        }
    }
    
    printf("\n────────────────────────────\n");
    printf("Total spikes in %.0f ms: %d\n", total_time, total_spikes);
    printf("Firing rate: %.1f Hz (spikes per second)\n", 
           total_spikes / (total_time / 1000.0f));
    printf("════════════════════════════\n");
    
    /* ─── Test 2: No input (should never fire) ─── */
    printf("\n--- Test 2: No input (should stay at rest) ---\n");
    neuron_reset(&n);
    
    for (float t = 0.0f; t < 50.0f; t += dt) {
        neuron_step(&n, 0.0f, dt, t);
    }
    printf("After 50ms with no input: V = %.2f mV (should be ~%.1f)\n",
           n.voltage, n.params.v_rest);
    printf("Spikes: %u (should be 0)\n", n.spike_count);
    
    /* ─── Test 3: Weak input (should never fire) ─── */
    printf("\n--- Test 3: Weak input (below threshold) ---\n");
    neuron_reset(&n);
    
    for (float t = 0.0f; t < 50.0f; t += dt) {
        neuron_step(&n, 0.5f, dt, t);  /* Weak: 0.5 nA */
    }
    printf("After 50ms with weak input (0.5 nA): V = %.2f mV\n", n.voltage);
    printf("Spikes: %u\n", n.spike_count);
    printf("(Voltage settled because leak balances the weak input)\n");
    
    /* ─── Test 4: Strong input (should fire fast) ─── */
    printf("\n--- Test 4: Strong input (should fire rapidly) ---\n");
    neuron_reset(&n);
    total_spikes = 0;
    
    for (float t = 0.0f; t < 50.0f; t += dt) {
        int fired = neuron_step(&n, 5.0f, dt, t);  /* Strong: 5 nA */
        if (fired) total_spikes++;
    }
    printf("Strong input (5 nA) for 50ms: %d spikes\n", total_spikes);
    printf("Firing rate: %.1f Hz\n", total_spikes / (50.0f / 1000.0f));
    
    printf("\n=== All tests complete ===\n");
    return 0;
}
```

---

## Step 4: Compile and Run

```bash
cd src
gcc -o test_neuron test_neuron.c core/neuron.c -lm -Wall -Wextra -std=c11 -O2
./test_neuron
```

### What you should see:

The neuron starts at -70mV, voltage rises as current flows in, hits -55mV → **SPIKE!** → resets to -75mV → refractory for 2ms → starts rising again → **SPIKE!** → and so on.

With weak input, it never fires. With strong input, it fires rapidly. Just like a real neuron.

---

## Step 5: Visualize (ASCII Spike Train)

Add this at the end of your test to see a visual spike train:

```c
/* ─── Visual: ASCII spike raster ─── */
printf("\n--- Visual: Spike train (each | is a spike) ---\n");
printf("Time: ");
neuron_reset(&n);

for (float t = 0.0f; t < 200.0f; t += 1.0f) {
    /* Run 10 sub-steps per ms */
    int fired_this_ms = 0;
    for (int sub = 0; sub < 10; sub++) {
        float sub_t = t + sub * 0.1f;
        if (neuron_step(&n, 2.0f, 0.1f, sub_t)) {
            fired_this_ms = 1;
        }
    }
    printf("%c", fired_this_ms ? '|' : '.');
}
printf("\n");
```

This will print something like:
```
Time: ..........|..........|..........|..........|..........
```

Each `|` is a spike. The gap between them is the integrate + refractory time. Notice the regular rhythm — like a heartbeat. Real neurons can also fire irregularly, which we'll explore later.

---

## What You Just Built

✅ A spiking neuron that **accumulates charge** (not just weighted sum)  
✅ A neuron that **leaks** (charge decays toward rest)  
✅ A neuron that **fires all-or-nothing spikes** (not smooth outputs)  
✅ A neuron with a **refractory period** (can't fire right after firing)  
✅ A neuron that lives in **time** (simulation runs in ms time steps)  

This is fundamentally different from every "neural network" tutorial. You're not computing `sigmoid(wx + b)`. You're simulating a physical process.

---

## Common Issues

| Problem | Cause | Fix |
|---------|-------|-----|
| Neuron never fires | Input too weak or dt too large | Increase input current or decrease dt |
| Neuron fires every step | Input too strong or threshold too low | Decrease input or increase threshold |
| Voltage goes to infinity | dt too large for tau_m | Use smaller dt (try 0.1) |
| Voltage becomes NaN | Division by zero or overflow | Check tau_m isn't 0 |

---

*Next: [05_build_neuron_network.md](05_build_neuron_network.md) — Connect neurons together and watch patterns emerge!*
