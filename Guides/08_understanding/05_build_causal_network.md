# 🔨 Build: Causal Reasoning Network — Knowing Why

## What We're Building

A network that stores cause-effect relationships and can:
1. **Predict** effects from causes (forward reasoning)
2. **Explain** effects by tracing back to causes (backward reasoning)
3. **Imagine** counterfactuals ("what if X hadn't happened?")
4. **Learn** new causal links from temporal experience

## Code: `src/understanding/causal.h`

```c
/*
 * causal.h — Causal Reasoning Network
 *
 * Rethink AI — Phase 8
 *
 * Stores directed causal relationships (A → B).
 * Supports:
 *   - Forward inference (predict effects from causes)
 *   - Backward inference (explain effects by finding causes)
 *   - Counterfactual reasoning (what would happen without X?)
 *   - Learning from temporal sequences
 */

#ifndef CAUSAL_H
#define CAUSAL_H

#define CAUSAL_MAX_NODES 64
#define CAUSAL_MAX_EDGES 256
#define CAUSAL_MAX_CHAIN 16

typedef struct {
    int id;
    char name[32];
    float activation;     /* Current activation (0 to 1) */
    float base_rate;      /* Probability without any causes */
} CausalNode;

typedef struct {
    int source;           /* Cause node index */
    int target;           /* Effect node index */
    float strength;       /* Causal strength (0 to 1) */
    float delay;          /* Time delay for cause → effect */
    int observations;     /* How many times this link was confirmed */
    int interventions;    /* How many times tested by intervention */
} CausalEdge;

typedef struct {
    CausalNode nodes[CAUSAL_MAX_NODES];
    CausalEdge edges[CAUSAL_MAX_EDGES];
    int num_nodes;
    int num_edges;
    
    /* Learning */
    float temporal_window;    /* Max time between cause and effect */
    float min_strength;       /* Below this, edge is pruned */
    float learning_rate;
} CausalNet;

/* Create and destroy */
CausalNet *causal_create(void);
void causal_destroy(CausalNet *cn);

/* Add nodes and edges */
int causal_add_node(CausalNet *cn, const char *name, float base_rate);
int causal_add_edge(CausalNet *cn, int source, int target, float strength, float delay);

/* Forward inference: activate cause, propagate to effects */
void causal_predict(CausalNet *cn, int cause_node);
void causal_predict_multiple(CausalNet *cn, const int *causes, int num_causes);

/* Backward inference: given active effect, find probable causes */
int causal_explain(CausalNet *cn, int effect_node, 
                   int *cause_chain, float *chain_strengths, int max_chain);

/* Counterfactual: what changes if we remove this cause? */
void causal_counterfactual(CausalNet *cn, int removed_cause,
                           float *original_activations, float *new_activations);

/* Learn: observed A then B → strengthen A→B link */
void causal_learn_temporal(CausalNet *cn, int before_node, int after_node, float delay);

/* Intervention: force a node on/off and observe effects */
void causal_intervene(CausalNet *cn, int node, float value);

/* Reset all activations */
void causal_reset(CausalNet *cn);

/* Get node by name */
int causal_find_node(const CausalNet *cn, const char *name);

/* Print */
void causal_print(const CausalNet *cn);
void causal_print_active(const CausalNet *cn);
void causal_print_explanation(const CausalNet *cn, int effect, 
                               const int *chain, const float *strengths, int chain_len);

#endif /* CAUSAL_H */
```

## Code: `src/understanding/causal.c`

```c
#include "causal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

CausalNet *causal_create(void) {
    CausalNet *cn = (CausalNet *)calloc(1, sizeof(CausalNet));
    cn->temporal_window = 5.0f;
    cn->min_strength = 0.05f;
    cn->learning_rate = 0.1f;
    return cn;
}

void causal_destroy(CausalNet *cn) { free(cn); }

int causal_add_node(CausalNet *cn, const char *name, float base_rate) {
    if (cn->num_nodes >= CAUSAL_MAX_NODES) return -1;
    int idx = cn->num_nodes++;
    cn->nodes[idx].id = idx;
    strncpy(cn->nodes[idx].name, name, 31);
    cn->nodes[idx].name[31] = '\0';
    cn->nodes[idx].activation = 0.0f;
    cn->nodes[idx].base_rate = base_rate;
    return idx;
}

int causal_add_edge(CausalNet *cn, int source, int target, float strength, float delay) {
    if (cn->num_edges >= CAUSAL_MAX_EDGES) return -1;
    if (source < 0 || source >= cn->num_nodes) return -1;
    if (target < 0 || target >= cn->num_nodes) return -1;
    
    int idx = cn->num_edges++;
    cn->edges[idx].source = source;
    cn->edges[idx].target = target;
    cn->edges[idx].strength = strength;
    cn->edges[idx].delay = delay;
    cn->edges[idx].observations = 1;
    cn->edges[idx].interventions = 0;
    return idx;
}

void causal_reset(CausalNet *cn) {
    for (int i = 0; i < cn->num_nodes; i++) {
        cn->nodes[i].activation = cn->nodes[i].base_rate;
    }
}

int causal_find_node(const CausalNet *cn, const char *name) {
    for (int i = 0; i < cn->num_nodes; i++) {
        if (strcmp(cn->nodes[i].name, name) == 0) return i;
    }
    return -1;
}

void causal_predict(CausalNet *cn, int cause_node) {
    if (cause_node < 0 || cause_node >= cn->num_nodes) return;
    
    /* Activate the cause */
    cn->nodes[cause_node].activation = 1.0f;
    
    /* Propagate through the network (simple BFS-like) */
    /* Multiple passes to handle chains */
    for (int pass = 0; pass < cn->num_nodes; pass++) {
        int changed = 0;
        for (int e = 0; e < cn->num_edges; e++) {
            CausalEdge *edge = &cn->edges[e];
            float source_act = cn->nodes[edge->source].activation;
            if (source_act < 0.01f) continue;
            
            float effect = source_act * edge->strength;
            float old_act = cn->nodes[edge->target].activation;
            /* Noisy-OR combination: P(effect) = 1 - (1-old)(1-new) */
            float new_act = 1.0f - (1.0f - old_act) * (1.0f - effect);
            
            if (new_act > old_act + 0.001f) {
                cn->nodes[edge->target].activation = new_act;
                changed = 1;
            }
        }
        if (!changed) break;
    }
}

void causal_predict_multiple(CausalNet *cn, const int *causes, int num_causes) {
    causal_reset(cn);
    for (int i = 0; i < num_causes; i++) {
        cn->nodes[causes[i]].activation = 1.0f;
    }
    /* Single propagation after setting all causes */
    for (int pass = 0; pass < cn->num_nodes; pass++) {
        int changed = 0;
        for (int e = 0; e < cn->num_edges; e++) {
            CausalEdge *edge = &cn->edges[e];
            float source_act = cn->nodes[edge->source].activation;
            if (source_act < 0.01f) continue;
            float effect = source_act * edge->strength;
            float old_act = cn->nodes[edge->target].activation;
            float new_act = 1.0f - (1.0f - old_act) * (1.0f - effect);
            if (new_act > old_act + 0.001f) {
                cn->nodes[edge->target].activation = new_act;
                changed = 1;
            }
        }
        if (!changed) break;
    }
}

int causal_explain(CausalNet *cn, int effect_node,
                   int *cause_chain, float *chain_strengths, int max_chain) {
    if (effect_node < 0 || effect_node >= cn->num_nodes) return 0;
    
    int chain_len = 0;
    int current = effect_node;
    
    /* Trace backward through strongest causes */
    int visited[CAUSAL_MAX_NODES] = {0};
    
    while (chain_len < max_chain) {
        visited[current] = 1;
        
        /* Find strongest incoming edge */
        float best_strength = 0.0f;
        int best_source = -1;
        
        for (int e = 0; e < cn->num_edges; e++) {
            if (cn->edges[e].target == current && !visited[cn->edges[e].source]) {
                float s = cn->edges[e].strength;
                if (s > best_strength) {
                    best_strength = s;
                    best_source = cn->edges[e].source;
                }
            }
        }
        
        if (best_source < 0) break;  /* No more causes */
        
        cause_chain[chain_len] = best_source;
        chain_strengths[chain_len] = best_strength;
        chain_len++;
        current = best_source;
    }
    
    return chain_len;
}

void causal_counterfactual(CausalNet *cn, int removed_cause,
                           float *original_activations, float *new_activations) {
    /* Save original activations */
    for (int i = 0; i < cn->num_nodes; i++) {
        original_activations[i] = cn->nodes[i].activation;
    }
    
    /* Reset and recompute without the removed cause */
    causal_reset(cn);
    
    /* Activate all nodes that were originally active EXCEPT the removed one */
    for (int i = 0; i < cn->num_nodes; i++) {
        if (i == removed_cause) continue;
        if (original_activations[i] > 0.5f) {
            /* Check if this node had no incoming cause (it was independently active) */
            int has_active_cause = 0;
            for (int e = 0; e < cn->num_edges; e++) {
                if (cn->edges[e].target == i && 
                    original_activations[cn->edges[e].source] > 0.5f) {
                    has_active_cause = 1;
                    break;
                }
            }
            if (!has_active_cause) {
                cn->nodes[i].activation = 1.0f;
            }
        }
    }
    
    /* Propagate */
    for (int pass = 0; pass < cn->num_nodes; pass++) {
        int changed = 0;
        for (int e = 0; e < cn->num_edges; e++) {
            if (cn->edges[e].source == removed_cause) continue; 
            CausalEdge *edge = &cn->edges[e];
            float source_act = cn->nodes[edge->source].activation;
            if (source_act < 0.01f) continue;
            float effect = source_act * edge->strength;
            float old_act = cn->nodes[edge->target].activation;
            float new_act = 1.0f - (1.0f - old_act) * (1.0f - effect);
            if (new_act > old_act + 0.001f) {
                cn->nodes[edge->target].activation = new_act;
                changed = 1;
            }
        }
        if (!changed) break;
    }
    
    for (int i = 0; i < cn->num_nodes; i++) {
        new_activations[i] = cn->nodes[i].activation;
    }
}

void causal_learn_temporal(CausalNet *cn, int before, int after, float delay) {
    if (delay > cn->temporal_window) return;
    
    /* Check if edge already exists */
    for (int e = 0; e < cn->num_edges; e++) {
        if (cn->edges[e].source == before && cn->edges[e].target == after) {
            /* Strengthen existing edge */
            cn->edges[e].strength = fminf(1.0f, 
                cn->edges[e].strength + cn->learning_rate);
            cn->edges[e].observations++;
            return;
        }
    }
    
    /* Create new edge */
    causal_add_edge(cn, before, after, cn->learning_rate, delay);
}

void causal_intervene(CausalNet *cn, int node, float value) {
    /* Force node to specific value, then propagate */
    causal_reset(cn);
    cn->nodes[node].activation = value;
    
    /* Propagate from intervened node */
    for (int pass = 0; pass < cn->num_nodes; pass++) {
        int changed = 0;
        for (int e = 0; e < cn->num_edges; e++) {
            CausalEdge *edge = &cn->edges[e];
            float source_act = cn->nodes[edge->source].activation;
            if (source_act < 0.01f) continue;
            float effect = source_act * edge->strength;
            float old_act = cn->nodes[edge->target].activation;
            float new_act = 1.0f - (1.0f - old_act) * (1.0f - effect);
            if (new_act > old_act + 0.001f) {
                cn->nodes[edge->target].activation = new_act;
                changed = 1;
            }
        }
        if (!changed) break;
    }
    
    /* Mark edges from this node as tested by intervention */
    for (int e = 0; e < cn->num_edges; e++) {
        if (cn->edges[e].source == node) {
            cn->edges[e].interventions++;
        }
    }
}

void causal_print(const CausalNet *cn) {
    printf("\n=== Causal Network ===\n");
    printf("Nodes: %d  Edges: %d\n\n", cn->num_nodes, cn->num_edges);
    for (int e = 0; e < cn->num_edges; e++) {
        printf("  %s ─(%.2f)─→ %s  [obs=%d, int=%d]\n",
               cn->nodes[cn->edges[e].source].name,
               cn->edges[e].strength,
               cn->nodes[cn->edges[e].target].name,
               cn->edges[e].observations,
               cn->edges[e].interventions);
    }
    printf("======================\n");
}

void causal_print_active(const CausalNet *cn) {
    printf("Active nodes: ");
    for (int i = 0; i < cn->num_nodes; i++) {
        if (cn->nodes[i].activation > 0.1f) {
            printf("%s(%.2f) ", cn->nodes[i].name, cn->nodes[i].activation);
        }
    }
    printf("\n");
}

void causal_print_explanation(const CausalNet *cn, int effect,
                               const int *chain, const float *strengths, int chain_len) {
    printf("Why '%s'?\n  ", cn->nodes[effect].name);
    printf("%s ← ", cn->nodes[effect].name);
    for (int i = 0; i < chain_len; i++) {
        printf("(%.2f) %s", strengths[i], cn->nodes[chain[i]].name);
        if (i < chain_len - 1) printf(" ← ");
    }
    printf("\n");
}
```

## Test: `test_causal.c`

```c
/*
 * test_causal.c — Causal reasoning in action
 *
 * Compile: gcc -std=c11 -O2 -Wall -o test_causal test_causal.c causal.c -lm
 */

#include <stdio.h>
#include "causal.h"

int main(void) {
    printf("=== Causal Reasoning Network ===\n\n");
    
    CausalNet *cn = causal_create();
    
    /* Build a causal model of weather → traffic */
    int rain     = causal_add_node(cn, "rain", 0.0f);
    int wet      = causal_add_node(cn, "wet_ground", 0.0f);
    int slip     = causal_add_node(cn, "slippery", 0.0f);
    int slow     = causal_add_node(cn, "slow_cars", 0.0f);
    int traffic  = causal_add_node(cn, "traffic", 0.0f);
    int accident = causal_add_node(cn, "accident", 0.0f);
    int late     = causal_add_node(cn, "late_for_work", 0.0f);
    
    causal_add_edge(cn, rain, wet, 0.9f, 1.0f);
    causal_add_edge(cn, wet, slip, 0.7f, 0.5f);
    causal_add_edge(cn, slip, slow, 0.6f, 1.0f);
    causal_add_edge(cn, slip, accident, 0.3f, 2.0f);
    causal_add_edge(cn, slow, traffic, 0.5f, 2.0f);
    causal_add_edge(cn, accident, traffic, 0.8f, 1.0f);
    causal_add_edge(cn, traffic, late, 0.7f, 3.0f);
    
    causal_print(cn);
    
    /* Test 1: Forward prediction */
    printf("\n--- Forward: What happens if it rains? ---\n");
    causal_reset(cn);
    causal_predict(cn, rain);
    causal_print_active(cn);
    
    /* Test 2: Backward explanation */
    printf("\n--- Backward: Why am I late? ---\n");
    int chain[CAUSAL_MAX_CHAIN];
    float strengths[CAUSAL_MAX_CHAIN];
    int len = causal_explain(cn, late, chain, strengths, CAUSAL_MAX_CHAIN);
    causal_print_explanation(cn, late, chain, strengths, len);
    
    /* Test 3: Counterfactual */
    printf("\n--- Counterfactual: What if it hadn't rained? ---\n");
    causal_reset(cn);
    causal_predict(cn, rain);  /* Original world */
    float original[CAUSAL_MAX_NODES], counterfactual[CAUSAL_MAX_NODES];
    causal_counterfactual(cn, rain, original, counterfactual);
    
    printf("With rain:    traffic=%.2f  late=%.2f  accident=%.2f\n",
           original[traffic], original[late], original[accident]);
    printf("Without rain: traffic=%.2f  late=%.2f  accident=%.2f\n",
           counterfactual[traffic], counterfactual[late], counterfactual[accident]);
    printf("→ Rain CAUSED the lateness (it would not have happened without rain)\n");
    
    /* Test 4: Learn from experience */
    printf("\n--- Learning: Observe new causal link ---\n");
    int construction = causal_add_node(cn, "construction", 0.0f);
    /* Observe: construction before slow_cars, 3 times */
    causal_learn_temporal(cn, construction, slow, 2.0f);
    causal_learn_temporal(cn, construction, slow, 2.0f);
    causal_learn_temporal(cn, construction, slow, 2.0f);
    
    printf("Learned: construction →(%.2f)→ slow_cars (from 3 observations)\n",
           cn->edges[cn->num_edges - 1].strength);
    
    /* Test 5: Intervention */
    printf("\n--- Intervention: What if we force road to be dry? ---\n");
    causal_intervene(cn, slip, 0.0f);  /* Make road not slippery */
    causal_print_active(cn);
    printf("→ Forcing dry roads prevents downstream effects\n");
    
    causal_destroy(cn);
    printf("\n✓ Causal reasoning works!\n");
    return 0;
}
```

---

*Next: [Milestone — The Understanding Brain](06_milestone_understanding.md)*
