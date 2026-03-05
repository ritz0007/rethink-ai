# 🔨 Build: STDP Learning — Teaching the Network Sequences

## What We're Building

A network that learns **sequences** using STDP (Spike-Timing Dependent Plasticity).

We'll train it so that neuron A → B → C fire in order. After training, triggering A should automatically trigger B, which triggers C. Like how hearing "Happy Birth..." makes your brain automatically continue "...day to you."

---

## The Demo: Learning a Chain

Create `src/test_stdp.c`:

```c
/*
 * test_stdp.c — STDP Sequence Learning Demo
 * 
 * Setup:
 *   - 5 neurons in a chain: 0 → 1 → 2 → 3 → 4
 *   - All-to-all connections with weak initial weights  
 *   - Training: force neurons to fire in sequence with specific timing
 *   - STDP strengthens connections in the firing direction
 *   - Test: trigger neuron 0 → does the sequence cascade?
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "core/neuron.h"
#include "core/synapse.h"
#include "core/network.h"
#include "learning/stdp.h"

int main(void) {
    srand((unsigned)time(NULL));
    
    printf("=== RETHINK AI — Phase 2: STDP Sequence Learning ===\n\n");
    
    /* 5 neurons, 0.1ms time step */
    Network *net = network_create(5, 0.1f);
    
    /* All-to-all connections with weak initial weights */
    for (uint32_t i = 0; i < 5; i++) {
        for (uint32_t j = 0; j < 5; j++) {
            if (i != j) {
                network_connect(net, i, j, 0.3f, 1.0f);  /* Weak, 1ms delay */
            }
        }
    }
    
    STDPParams sparams = stdp_default_params();
    sparams.a_plus = 0.015f;
    sparams.a_minus = 0.018f;
    
    /* ─── Print initial weights ─── */
    printf("Initial weights (forward chain 0→1→2→3→4):\n");
    for (uint32_t s = 0; s < net->num_synapses; s++) {
        uint32_t src = net->synapses[s].source_id;
        uint32_t tgt = net->synapses[s].target_id;
        if (tgt == src + 1) {  /* Forward connections */
            printf("  %u → %u : %.4f\n", src, tgt, net->synapses[s].weight);
        }
    }
    
    printf("Initial weights (backward chain 4→3→2→1→0):\n");
    for (uint32_t s = 0; s < net->num_synapses; s++) {
        uint32_t src = net->synapses[s].source_id;
        uint32_t tgt = net->synapses[s].target_id;
        if (src == tgt + 1) {  /* Backward connections */
            printf("  %u → %u : %.4f\n", src, tgt, net->synapses[s].weight);
        }
    }
    
    /* ─── TRAINING: Force sequential firing ─── */
    printf("\n--- TRAINING: Forcing sequence 0→1→2→3→4 (200 repetitions) ---\n");
    printf("STDP will strengthen forward connections，weaken backward ones.\n\n");
    
    for (int rep = 0; rep < 200; rep++) {
        /* Reset neurons (keep learned weights) */
        for (uint32_t i = 0; i < 5; i++) neuron_reset(&net->neurons[i]);
        net->current_time = 0; net->num_pending = 0;
        network_clear_input(net);
        
        /* Force sequence: neuron i fires at time i*5 ms */
        /* We do this by giving strong input to each neuron at the right time */
        for (int step = 0; step < 500; step++) {  /* 50ms total */
            float t = step * 0.1f;
            
            /* Each neuron gets a burst of input at its scheduled time */
            for (uint32_t i = 0; i < 5; i++) {
                float fire_time = i * 5.0f;  /* Neuron i fires at i*5 ms */
                if (t >= fire_time && t < fire_time + 2.0f) {
                    network_set_input(net, i, 5.0f);  /* Strong input */
                } else {
                    network_set_input(net, i, 0.0f);
                }
            }
            
            network_step(net);
            stdp_learn(net, &sparams);
        }
        
        if ((rep + 1) % 50 == 0) {
            printf("  Rep %3d: Forward weights: ", rep + 1);
            for (uint32_t s = 0; s < net->num_synapses; s++) {
                if (net->synapses[s].target_id == net->synapses[s].source_id + 1) {
                    printf("%.3f ", net->synapses[s].weight);
                }
            }
            printf("\n");
        }
    }
    
    /* ─── Print learned weights ─── */
    printf("\nLearned weights (forward chain — should be STRONG):\n");
    for (uint32_t s = 0; s < net->num_synapses; s++) {
        uint32_t src = net->synapses[s].source_id;
        uint32_t tgt = net->synapses[s].target_id;
        if (tgt == src + 1) {
            printf("  %u → %u : %.4f\n", src, tgt, net->synapses[s].weight);
        }
    }
    
    printf("Learned weights (backward chain — should be WEAK):\n");
    for (uint32_t s = 0; s < net->num_synapses; s++) {
        uint32_t src = net->synapses[s].source_id;
        uint32_t tgt = net->synapses[s].target_id;
        if (src == tgt + 1) {
            printf("  %u → %u : %.4f\n", src, tgt, net->synapses[s].weight);
        }
    }
    
    /* ─── TEST: Trigger only neuron 0 ─── */
    printf("\n--- TEST: Triggering ONLY neuron 0 ---\n");
    printf("If STDP worked, the sequence should cascade: 0→1→2→3→4\n\n");
    
    /* Reset neurons (keep weights) */
    for (uint32_t i = 0; i < 5; i++) neuron_reset(&net->neurons[i]);
    net->current_time = 0; net->num_pending = 0;
    network_clear_input(net);
    
    /* Only trigger neuron 0 */
    network_set_input(net, 0, 4.0f);
    
    /* Run and watch */
    int neuron_fired[5] = {0, 0, 0, 0, 0};
    float first_fire_time[5] = {-1, -1, -1, -1, -1};
    
    for (int step = 0; step < 1000; step++) {  /* 100ms */
        network_step(net);
        
        /* After 5ms, remove input to neuron 0 (let the chain run on its own) */
        if (step == 50) {
            network_set_input(net, 0, 0.0f);
        }
        
        for (uint32_t i = 0; i < 5; i++) {
            if (net->neurons[i].spiked && !neuron_fired[i]) {
                neuron_fired[i] = 1;
                first_fire_time[i] = net->current_time;
                printf("  t=%6.1fms: Neuron %u FIRES! %s\n", 
                       net->current_time, i,
                       i == 0 ? "(triggered)" : "(cascaded!)");
            }
        }
    }
    
    /* Results */
    printf("\n═══════════════════════════════════════\n");
    printf("Sequence cascade results:\n");
    int chain_length = 0;
    for (uint32_t i = 0; i < 5; i++) {
        printf("  Neuron %u: %s", i, neuron_fired[i] ? "FIRED" : "silent");
        if (neuron_fired[i]) {
            printf(" (t=%.1fms)", first_fire_time[i]);
            chain_length++;
        }
        printf("\n");
    }
    
    printf("\nChain propagation: %d / 5 neurons activated\n", chain_length);
    
    if (chain_length >= 4) {
        printf("✓ SUCCESS! STDP learned the sequence!\n");
        printf("  Triggering neuron 0 cascades through the chain.\n");
        printf("  Like hearing 'Happy Birth...' → '...day to you'\n");
    } else if (chain_length >= 2) {
        printf("△ Partial chain. The sequence partially propagated.\n");
        printf("  Try more training repetitions or higher learning rate.\n");
    } else {
        printf("✗ Chain didn't propagate. Forward weights may not be strong enough.\n");
        printf("  Try: more reps, higher a_plus, stronger initial weights.\n");
    }
    printf("═══════════════════════════════════════\n");
    
    network_destroy(net);
    return 0;
}
```

### Compile and run:
```bash
gcc -o test_stdp test_stdp.c core/neuron.c core/synapse.c core/network.c \
    learning/stdp.c -lm -Wall -Wextra -std=c11 -O2
./test_stdp
```

---

## What You Should See

After training:
- **Forward weights (0→1, 1→2, 2→3, 3→4)** are STRONG (grew from 0.3 to ~2-5)
- **Backward weights (4→3, 3→2, 2→1, 1→0)** are WEAK (shrank from 0.3 to ~0)
- **Test**: Trigger neuron 0 → neuron 1 fires → neuron 2 fires → etc.

STDP has turned a random network into a **directed chain** — from timing alone!

---

## Key Insight

The network learned **direction** from **timing**:
- "A fires before B" → strengthen A→B (causation)
- "A fires after B" → weaken A→B (not causal)

No one told it what direction the chain should go. It discovered direction from timing patterns. This is how your brain learns sequences — melodies, sentences, dance steps, habits.

---

## Comparison: Hebbian vs STDP

| | Hebbian (test_hebbian.c) | STDP (test_stdp.c) |
|---|---|---|
| **What it learns** | Associations (A↔B) | Sequences (A→B→C) |
| **Direction** | Bidirectional (symmetric) | Unidirectional (asymmetric) |
| **Key signal** | Co-occurrence (both fire) | Timing (who fires first) |
| **Brain analogy** | "Coffee smell = kitchen" | "Happy Birth → day to you" |

Both are biologically real. Both happen in your brain. Neither uses backpropagation.

---

**Phase 2 Complete.** Tag as **V2**.

*Next phase: [Phase 3 — Seeing the World](../03_seeing_the_world/01_brain_visual_cortex.md)*
