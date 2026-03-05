/*
 * causal.h — Causal Reasoning Network
 * Rethink AI — Phase 8
 */

#ifndef CAUSAL_H
#define CAUSAL_H

#define CAUSAL_MAX_NODES 64
#define CAUSAL_MAX_EDGES 256
#define CAUSAL_MAX_CHAIN 16

typedef struct {
    int id;
    char name[32];
    float activation;
    float base_rate;
} CausalNode;

typedef struct {
    int source;
    int target;
    float strength;
    float delay;
    int observations;
    int interventions;
} CausalEdge;

typedef struct {
    CausalNode nodes[CAUSAL_MAX_NODES];
    CausalEdge edges[CAUSAL_MAX_EDGES];
    int num_nodes;
    int num_edges;
    float temporal_window;
    float min_strength;
    float learning_rate;
} CausalNet;

CausalNet *causal_create(void);
void causal_destroy(CausalNet *cn);

int causal_add_node(CausalNet *cn, const char *name, float base_rate);
int causal_add_edge(CausalNet *cn, int source, int target, float strength, float delay);

void causal_predict(CausalNet *cn, int cause_node);
void causal_predict_multiple(CausalNet *cn, const int *causes, int num_causes);
void causal_reset(CausalNet *cn);

int causal_explain(CausalNet *cn, int effect_node,
                   int *cause_chain, float *chain_strengths, int max_chain);
void causal_counterfactual(CausalNet *cn, int removed_cause,
                           float *original_activations, float *new_activations);
void causal_learn_temporal(CausalNet *cn, int before, int after, float delay);
void causal_intervene(CausalNet *cn, int node, float value);
int causal_find_node(const CausalNet *cn, const char *name);

void causal_print(const CausalNet *cn);
void causal_print_active(const CausalNet *cn);
void causal_print_explanation(const CausalNet *cn, int effect,
                               const int *chain, const float *strengths, int chain_len);

#endif /* CAUSAL_H */
