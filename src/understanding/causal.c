/*
 * causal.c — Causal Network Implementation
 * Rethink AI — Phase 8
 */

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
    for (int i = 0; i < cn->num_nodes; i++)
        cn->nodes[i].activation = cn->nodes[i].base_rate;
}

int causal_find_node(const CausalNet *cn, const char *name) {
    for (int i = 0; i < cn->num_nodes; i++)
        if (strcmp(cn->nodes[i].name, name) == 0) return i;
    return -1;
}

void causal_predict(CausalNet *cn, int cause_node) {
    if (cause_node < 0 || cause_node >= cn->num_nodes) return;
    cn->nodes[cause_node].activation = 1.0f;
    
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

void causal_predict_multiple(CausalNet *cn, const int *causes, int num_causes) {
    causal_reset(cn);
    for (int i = 0; i < num_causes; i++)
        cn->nodes[causes[i]].activation = 1.0f;
    
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
    int chain_len = 0, current = effect_node;
    int visited[CAUSAL_MAX_NODES] = {0};
    
    while (chain_len < max_chain) {
        visited[current] = 1;
        float best_strength = 0.0f;
        int best_source = -1;
        
        for (int e = 0; e < cn->num_edges; e++) {
            if (cn->edges[e].target == current && !visited[cn->edges[e].source]) {
                if (cn->edges[e].strength > best_strength) {
                    best_strength = cn->edges[e].strength;
                    best_source = cn->edges[e].source;
                }
            }
        }
        if (best_source < 0) break;
        cause_chain[chain_len] = best_source;
        chain_strengths[chain_len] = best_strength;
        chain_len++;
        current = best_source;
    }
    return chain_len;
}

void causal_counterfactual(CausalNet *cn, int removed_cause,
                           float *original_activations, float *new_activations) {
    for (int i = 0; i < cn->num_nodes; i++)
        original_activations[i] = cn->nodes[i].activation;
    
    causal_reset(cn);
    for (int i = 0; i < cn->num_nodes; i++) {
        if (i == removed_cause) continue;
        if (original_activations[i] > 0.5f) {
            int has_active_cause = 0;
            for (int e = 0; e < cn->num_edges; e++) {
                if (cn->edges[e].target == i &&
                    original_activations[cn->edges[e].source] > 0.5f) {
                    has_active_cause = 1; break;
                }
            }
            if (!has_active_cause) cn->nodes[i].activation = 1.0f;
        }
    }
    
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
    
    for (int i = 0; i < cn->num_nodes; i++)
        new_activations[i] = cn->nodes[i].activation;
}

void causal_learn_temporal(CausalNet *cn, int before, int after, float delay) {
    if (delay > cn->temporal_window) return;
    for (int e = 0; e < cn->num_edges; e++) {
        if (cn->edges[e].source == before && cn->edges[e].target == after) {
            cn->edges[e].strength = fminf(1.0f, cn->edges[e].strength + cn->learning_rate);
            cn->edges[e].observations++;
            return;
        }
    }
    causal_add_edge(cn, before, after, cn->learning_rate, delay);
}

void causal_intervene(CausalNet *cn, int node, float value) {
    causal_reset(cn);
    cn->nodes[node].activation = value;
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
    for (int e = 0; e < cn->num_edges; e++)
        if (cn->edges[e].source == node) cn->edges[e].interventions++;
}

void causal_print(const CausalNet *cn) {
    printf("\n=== Causal Network ===\n");
    printf("Nodes: %d  Edges: %d\n\n", cn->num_nodes, cn->num_edges);
    for (int e = 0; e < cn->num_edges; e++) {
        printf("  %s ─(%.2f)─→ %s  [obs=%d, int=%d]\n",
               cn->nodes[cn->edges[e].source].name, cn->edges[e].strength,
               cn->nodes[cn->edges[e].target].name,
               cn->edges[e].observations, cn->edges[e].interventions);
    }
    printf("======================\n");
}

void causal_print_active(const CausalNet *cn) {
    printf("Active nodes: ");
    for (int i = 0; i < cn->num_nodes; i++)
        if (cn->nodes[i].activation > 0.1f)
            printf("%s(%.2f) ", cn->nodes[i].name, cn->nodes[i].activation);
    printf("\n");
}

void causal_print_explanation(const CausalNet *cn, int effect,
                               const int *chain, const float *strengths, int chain_len) {
    printf("Why '%s'?\n  %s ← ", cn->nodes[effect].name, cn->nodes[effect].name);
    for (int i = 0; i < chain_len; i++) {
        printf("(%.2f) %s", strengths[i], cn->nodes[chain[i]].name);
        if (i < chain_len - 1) printf(" ← ");
    }
    printf("\n");
}
