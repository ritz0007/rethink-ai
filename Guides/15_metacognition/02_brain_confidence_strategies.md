# Confidence, Strategies, and Learning-to-Learn — The Mathematics

> Rethink AI — Phase 15, Guide 2 of 5

---

## Confidence Monitoring (aPFC)

### What Is Metacognitive Confidence?

Confidence is a second-order judgment: not "what is this?" but "how sure am I?" The aPFC computes this by aggregating evidence from multiple sources.

### Multi-Source Confidence Computation

For any cognitive decision $d$, confidence is computed from three signals:

$$C(d) = w_1 \cdot C_{\text{consistency}} + w_2 \cdot C_{\text{prediction}} + w_3 \cdot C_{\text{memory}}$$

Where:

**1. Consistency confidence** — How consistent were recent answers?

$$C_{\text{consistency}} = 1 - \sigma^2_{\text{recent}}$$

Where $\sigma^2_{\text{recent}}$ is the variance of recent prediction errors. High variance → low confidence. The system tracks a rolling window of recent errors.

**2. Prediction confidence** — How well did I predict outcomes?

$$C_{\text{prediction}} = \frac{1}{N}\sum_{i=1}^{N}(1 - |e_i|)$$

Where $e_i$ is the signed prediction error. If predictions have been accurate, confidence is high.

**3. Memory confidence** — How strong is my relevant memory?

$$C_{\text{memory}} = \text{avg}(\text{decay\_strength of relevant memories})$$

Stronger memories → higher confidence in memory-based decisions.

**Default weights**: $w_1 = 0.4, w_2 = 0.35, w_3 = 0.25$

### Confidence Calibration

Well-calibrated confidence means: when confidence is 80%, accuracy should be ~80%. Calibration error:

$$\text{ECE} = \sum_{b=1}^{B} \frac{n_b}{N} |acc(b) - conf(b)|$$

We track calibration and adjust confidence outputsif they're systematically over- or under-confident:

$$C_{\text{adjusted}} = C_{\text{raw}} + \alpha \cdot (\text{accuracy} - C_{\text{avg}})$$

---

## Error & Conflict Monitoring (ACC)

### Error Detection

The ACC monitors prediction errors and flags anomalies:

$$\text{error\_signal} = \frac{|e_{\text{current}}|}{\bar{|e|}_{\text{history}} + \epsilon}$$

When $\text{error\_signal} > \theta_{\text{error}}$: the current error is much larger than typical → trigger error response:
1. Boost attention (deploy more cognitive resources)
2. Slow down (increase caution in next decision)
3. Log the error context (for strategy evaluation)

### Conflict Detection

When multiple strategies produce conflicting answers:

$$\text{conflict} = 1 - \max(\text{strategy\_agreement})$$

High conflict → dlPFC should consider switching strategies.

### Cognitive Load Tracking

Effort is tracked by monitoring resource consumption:

$$\text{effort} = \frac{\text{active\_processes}}{\text{max\_capacity}} \cdot \frac{1}{\text{available\_attention}}$$

High effort signals the need for more efficient strategies or rest.

---

## Strategy Selection (dlPFC)

### Strategy Repertoire

Each cognitive strategy has:

```c
typedef struct {
    char  name[32];          /* "memory_lookup", "causal_reason", "analogy", etc. */
    float success_rate;      /* Running average of success [0-1] */
    float avg_effort;        /* Average cognitive cost [0-1] */
    float value;             /* success_rate / (avg_effort + epsilon) */
    int   usage_count;       /* Times selected */
    int   active;
} CognitiveStrategy;
```

### Value Computation (vmPFC)

Strategy value balances success against effort:

$$V_s = \frac{\text{success\_rate}_s}{\text{avg\_effort}_s + \epsilon} \cdot (1 + \text{bonus}_{\text{context}})$$

Context bonus reflects whether this strategy has worked for similar problems before.

### Selection (softmax with temperature)

Strategy selected probabilistically:

$$P(s) = \frac{e^{V_s / \tau}}{\sum_j e^{V_j / \tau}}$$

Temperature $\tau$ is controlled by metacognitive state:
- **High confidence** → low temperature → exploit best strategy
- **Low confidence** → high temperature → explore different strategies
- **After error** → moderate temperature → try alternatives

### Strategy Update After Outcome

$$\text{success\_rate}_s \leftarrow \text{success\_rate}_s + \alpha_s \cdot (\text{outcome} - \text{success\_rate}_s)$$

$$\text{avg\_effort}_s \leftarrow \text{avg\_effort}_s + \alpha_e \cdot (\text{effort} - \text{avg\_effort}_s)$$

---

## Learning-to-Learn (Meta-Learning)

### Adaptive Learning Rate

The learning rate should adapt to metacognitive state:

$$\alpha_{\text{effective}} = \alpha_{\text{base}} \cdot \text{surprise\_factor} \cdot \text{uncertainty\_factor}$$

Where:
- $\text{surprise\_factor} = 1 + \beta \cdot \text{surprise}$ — Surprising events deserve faster learning
- $\text{uncertainty\_factor} = 0.5 + 0.5 \cdot (1 - C)$ — Low confidence → higher learning rate

When confident and unsurprised → learn slowly (preserve good knowledge).
When uncertain and surprised → learn fast (rapidly integrate new data).

### Performance Tracking

$$\text{learning\_trajectory} = \frac{\text{recent\_performance} - \text{past\_performance}}{\text{time\_window}}$$

Positive trajectory → we're improving → maintain current strategy.
Negative trajectory → we're getting worse → trigger strategy reassessment.
Flat trajectory → we've plateaued → try new approach or increase difficulty.

---

## Self-Model (aPFC + AIC)

### Internal Representation of "Self"

The brain builds a model of its own capabilities:

```c
typedef struct {
    float perception_accuracy;     /* How well can I perceive? */
    float memory_reliability;      /* How reliable is my memory? */
    float reasoning_skill;         /* How good am I at reasoning? */
    float learning_speed;          /* How fast do I learn? */
    float social_skill;            /* How well do I model others? */
    float motor_skill;             /* How good are my actions? */
    float overall_capability;      /* Aggregate self-assessment */
    float cognitive_stamina;       /* How long can I focus? */
} SelfModel;
```

Each dimension updates based on actual performance:

$$\text{skill}_d \leftarrow \text{skill}_d + \alpha \cdot (\text{actual\_performance}_d - \text{skill}_d)$$

### Metacognitive Accuracy

How well does the self-model predict actual performance?

$$\text{meta\_accuracy} = 1 - \text{mean}(|\text{predicted\_performance} - \text{actual\_performance}|)$$

High metacognitive accuracy = the brain knows its own strengths and weaknesses. This is the ultimate sign of a mature cognitive system.

---

## From Brain to Code — Design Summary

```
MetaSystem
├── ConfidenceMonitor ─── aPFC: consistency + prediction + memory sources
│                      ├── calibration tracking (ECE)
│                      └── confidence adjustment
├── ErrorMonitor ──────── ACC: error magnitude, conflict detection
│                      ├── cognitive load
│                      └── error response triggers
├── StrategySelector ──── dlPFC: strategy repertoire with value-based selection
│                      ├── softmax with temperature
│                      ├── temperature adapts to confidence
│                      └── success/effort tracking per strategy
├── MetaLearner ───────── adaptive learning rate
│                      ├── surprise × uncertainty factor
│                      └── learning trajectory tracking
├── SelfModel ─────────── aPFC + AIC: internal capability model
│                      ├── per-domain skill estimates
│                      └── metacognitive accuracy tracking
└── PerformanceHistory ── Ring buffer of recent outcomes
```

---

*Next: [03_build_meta.md](03_build_meta.md) — Header design and struct definitions*
