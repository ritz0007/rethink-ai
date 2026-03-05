# 🧠 Brain Science: Causal Reasoning — Knowing Why, Not Just What

## The Difference Between Correlation and Causation

Standard AI learns: "When X appears, Y usually follows."
Understanding requires: "X **causes** Y **because** of mechanism Z."

```
Correlation: Rooster crows → sun rises. (AI might learn: roosters cause sunrise!)
Causation:   Earth rotates → sun rises. (Understanding: the mechanism)
```

## How the Brain Builds Causal Models

### 1. Temporal Precedence

The brain notices: A always happens **before** B.

```
Touch hot stove (t=0) → Pain (t=0.1s) → Pull hand away (t=0.2s)

The brain encodes: touch_hot → causes → pain
                   pain → causes → withdrawal

NOT: withdrawal → causes → pain (wrong temporal order)
```

This is exactly what **STDP** does (Phase 2)! Pre-synaptic before post-synaptic = strengthen connection. STDP discovers causal timing.

### 2. Intervention

The brain doesn't just observe — it **experiments**:

```
Observation: Every time I push this button, the light turns on.
Question: Does the button CAUSE the light?

Test: Push without looking → light on. (Button causes light.)
Test: Someone else turns on light → button not pushed. 
  → Light doesn't cause button.

Causation confirmed: Button → Light (not Light → Button)
```

Babies do this constantly. They push, pull, drop, and throw things to build causal models. Every action is an experiment.

### 3. Counterfactual Reasoning

"What WOULD have happened if..."

```
Fact: I missed the bus. I was late for work.
Counterfactual: If I had left 5 minutes earlier, I would have caught the bus.

→ Causal model: departure_time → catch_bus → arrive_on_time
→ The critical cause was: departure_time
```

The brain can **simulate alternative histories** using its internal model. This is imagination applied to causation.

## Representing Causal Knowledge

### Causal Graphs

```
        Rain           
         │              
    ┌────▼────┐         
    │Wet ground │        
    └────┬────┘         
         │              
    ┌────▼────┐         
    │Slippery  │        
    └────┬────┘         
         │              
    ┌────▼────┐         
    │Slow cars │        
    └────┬────┘         
         │              
    ┌────▼────┐         
    │Traffic   │        
    └─────────┘         
```

Each arrow represents a **causal link** with a strength (probability):

```
P(wet_ground | rain) = 0.9
P(slippery | wet_ground) = 0.7
P(slow_cars | slippery) = 0.6
P(traffic | slow_cars) = 0.5

P(traffic | rain) = 0.9 × 0.7 × 0.6 × 0.5 ≈ 0.19
```

### Forward Inference (Prediction)

Given cause, predict effect:
```
"It's raining" → What will happen?
→ Ground gets wet → Roads slippery → Cars slow down → Traffic
```

### Backward Inference (Explanation)

Given effect, find cause:
```
"There's traffic" → Why?
→ Cars slow? Yes → Slippery? Maybe → Wet ground? Check → Rain? Yes!
```

### Counterfactual Inference

What if we change something:
```
"What if it hadn't rained?"
→ Remove rain → No wet ground → No slippery → No slow cars → No traffic
→ Conclusion: Rain is the root cause
```

## Implementation: Causal Network

```c
#define MAX_NODES 64
#define MAX_EDGES 128

typedef struct {
    int id;
    char name[32];
    float activation;    /* Current activation (0 to 1) */
    float baseline;      /* Default activation without input */
} CausalNode;

typedef struct {
    int source;
    int target;
    float strength;      /* Causal strength (0 to 1) */
    float delay;         /* Temporal delay */
    int learned;         /* Was this learned from experience? */
} CausalEdge;

typedef struct {
    CausalNode nodes[MAX_NODES];
    CausalEdge edges[MAX_EDGES];
    int num_nodes;
    int num_edges;
} CausalNet;

/* Forward: given causes activated, what happens? */
void causal_predict(CausalNet *cn, int cause_node);

/* Backward: given effect, what caused it? */
int causal_explain(CausalNet *cn, int effect_node, int *cause_chain, int max_chain);

/* Counterfactual: what if this cause were removed? */
void causal_counterfactual(CausalNet *cn, int removed_cause, float *effects);

/* Learn causal link from temporal co-occurrence */
void causal_learn(CausalNet *cn, int before_node, int after_node, float delay);
```

## How Causal Learning Works

### From Temporal Patterns (STDP-based)

```
Observed sequence: A → B (A before B, consistently)
→ Create causal link: A causes B (strength grows with repetitions)

Observed: A → B, but also C → B (multiple causes)
→ A causes B with strength 0.5
→ C causes B with strength 0.5

Intervention: Remove A, B still happens when C present
→ Weaken A→B, strengthen C→B
```

### From Prediction Errors

```
Prediction: Rain → Wet ground → Traffic
Reality: Rain, but NO traffic (road was covered/heated)
Error: Big surprise!

Update: There's an intervening variable:
  Rain → Wet ground → (IF road_uncovered) → Traffic
  
Model improved! Better predictions next time.
```

## Why This Matters

Without causal models, the brain is just a correlaton machine — "X and Y tend to appear together."

With causal models:
- "X **makes** Y happen **because** Z"
- "If we remove X, Y stops"
- "To get Y, we need X"
- "Y happened because of X, not coincidence"

This is engineering-level understanding. This is what lets humans build bridges, cure diseases, and plan for the future.

---

*Next: [Build — Predictive Coding Engine](04_build_predictive_coding.md)*
