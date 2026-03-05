# 🧠 Brain Science: Predictive Coding — The Brain as a Prediction Machine

## The Core Theory

Your brain is not a passive receiver. It's an **active prediction engine** that constantly generates expectations about what will happen next.

### The Hierarchy

```
HIGH LEVEL:  "I'm at home, it's evening, I expect dinner soon"
    ↓ predictions flow DOWN
MID LEVEL:   "I expect to see the kitchen, smell food"
    ↓ predictions flow DOWN  
LOW LEVEL:   "I expect to see the stove, counter, fridge"
    ↓ predictions flow DOWN
SENSORY:     "I expect these specific edges, colors, sounds"

    ↑ only ERRORS flow UP
    ↑ "Wait, the fridge is open — that's unexpected"
    ↑ "Why is there smoke? PREDICTION ERROR!"
```

### How It Works

1. **Top-down predictions** flow from higher to lower levels
2. Each level compares prediction to actual input
3. Only **prediction errors** (surprises) flow upward
4. Higher levels update their models to reduce future errors

```
Input: actual sensory data
Prediction: what the brain expected
Error = Input - Prediction

If error ≈ 0:  "As expected" → minimal processing
If error ≫ 0:  "SURPRISE!" → heavy processing, learning, attention
```

## Why This Is Genius

### 1. Massive Compression

Instead of processing every pixel of your visual field every moment, you only process what's DIFFERENT from expectations:

```
Normal scene: 1,000,000 pixels × 30fps = 30 million data points/second
Predictive coding: Only ~5% is surprising = 1.5 million data points/second

20× compression just by predicting!
```

### 2. Understanding = Good Predictions

If you can predict what happens next, you **understand** the situation:

```
Understanding physics: "I dropped the glass → I predict it shatters"
Understanding people: "She's frowning → I predict she's upset"  
Understanding language: "The cat sat on the ___" → I predict "mat"

Prediction ≈ Understanding
```

### 3. Learning = Reducing Prediction Errors

The brain's learning objective is simple:

$$\text{minimize} \sum_{t} (\text{prediction}_t - \text{reality}_t)^2$$

When prediction errors are small, the model is good. When they're large, the model updates.

This is DIFFERENT from backpropagation:
- Backprop: global error signal flows backward
- Predictive coding: **local** prediction errors at each level
- Each level only needs to compare its prediction vs. its input

### 4. Attention = Precision-Weighted Prediction Errors

Not all prediction errors are equal. The brain weighs them by **precision** (confidence in the prediction):

```
High precision prediction: "Gravity pulls things down"
  → small error: normal (wind, etc.)
  → large error: EXTREMELY SURPRISING (magic? broken physics?)

Low precision prediction: "The stock market will go up"
  → small error: whatever
  → large error: not that surprising (markets are unpredictable)
```

Attention goes to high-precision prediction errors — things that SHOULD be predictable but aren't.

## The Mathematical Framework

### At Each Level

$$\text{error}_l = \text{input}_l - g(\text{prediction}_{l+1})$$

Where $g()$ is a generative function that converts higher-level representations to lower-level predictions.

### Update Rule

$$\text{model}_{l+1} \leftarrow \text{model}_{l+1} + \alpha \cdot \text{precision}_l \cdot \text{error}_l$$

- $\alpha$ is learning rate
- $\text{precision}_l$ is confidence (inverse variance)
- $\text{error}_l$ is prediction error at level $l$

### Precision (Attention)

$$\text{precision}_l = \frac{1}{\text{variance}(\text{error}_l)}$$

Levels with consistently small errors have high precision → their errors matter more.

## How This Connects to Our System

### What We Already Have That Helps

| Existing Module | Role in Predictive Coding |
|----------------|--------------------------|
| Hopfield memory | Generates predictions from partial input |
| Prototype learner | Predicts category → expects category features |
| Spontaneous activity | Internal "predictions" that run without input |
| Emotion (amygdala) | Modulates precision (emotional events → high precision) |
| Attention (thalamus) | Routes prediction errors based on precision |

### What We Need to Add

```c
typedef struct {
    float prediction[MAX_FEATURES];    /* What I expect */
    float actual[MAX_FEATURES];        /* What really happened */
    float error[MAX_FEATURES];         /* The surprise */
    float precision[MAX_FEATURES];     /* How confident am I */
    float model[MAX_FEATURES];         /* My internal model */
} PredictiveLayer;
```

### The Key Insight

> **Understanding is the absence of surprise.**
> 
> When your predictions consistently match reality, you understand.
> When they don't, you're learning.

This gives us a **measurable** definition of understanding — no philosophy required.

---

*Next: [Causal Reasoning — Knowing Why, Not Just What](03_brain_causal_reasoning.md)*
