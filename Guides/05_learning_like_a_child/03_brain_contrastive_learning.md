# 🧠 Brain Science: Contrastive Learning — Learning What Makes Things Different

## The Problem with Just Similarity

If you only ask "how similar is this to the prototype?", you get confused fast:

```
Dog:  [4 legs, tail, fur, medium, loyal]
Cat:  [4 legs, tail, fur, small, independent]
Wolf: [4 legs, tail, fur, large, pack animal]
```

These are all VERY similar. Raw similarity won't separate them. The brain needs a second mechanism:

> **What makes this thing DIFFERENT from everything else I know?**

## How the Brain Does Contrastive Learning

### Neural Level: Lateral Inhibition (Again!)

Remember lateral inhibition from Phase 3? The same principle works for categories:

```
Input arrives → activates multiple prototypes
  Dog prototype: activation = 0.85
  Cat prototype: activation = 0.80
  Wolf prototype: activation = 0.75

Lateral inhibition kicks in:
  Dog  inhibits Cat  by 0.80 × 0.3 = −0.24
  Dog  inhibits Wolf by 0.75 × 0.3 = −0.23
  Cat  inhibits Dog  by 0.85 × 0.3 = −0.26
  ...

After inhibition:
  Dog: 0.85 - 0.24 - 0.23 = 0.38  ← WINNER (barely)
  Cat: 0.80 - 0.26 - 0.22 = 0.32
  Wolf: 0.75 - 0.25 - 0.24 = 0.26
```

Competition between similar categories forces the system to find **subtle differences**.

### Feature Level: Attention Amplification

The brain amplifies the features that successfully distinguished the winner:

```
Feature     | Dog  | Cat  | Difference | Attention Weight
------------|------|------|------------|----------------
Legs        | 4    | 4    | 0          | 0.1 (useless)
Tail        | 1    | 1    | 0          | 0.1 (useless)
Fur         | yes  | yes  | 0          | 0.1 (useless)
Size        | med  | small| HIGH       | 0.8 ← AMPLIFIED
Personality | loyal| indep| HIGH       | 0.9 ← AMPLIFIED
```

This happens through Hebbian learning:
- Features that fire when the correct category wins → strengthened
- Features that fire for multiple categories → weakened

### The Contrastive Update Rule

For each classification:

```
positive = distance(input, correct_prototype)    → should be SMALL
negative = distance(input, wrong_prototypes)      → should be LARGE

Learn:
  Pull correct prototype CLOSER to input
  Push wrong prototypes AWAY from input

  feature_weight[i] += learning_rate × |correct[i] - wrong[i]|
  (features with big differences get higher weight)
```

## The Mathematical Formulation

### Weighted Similarity

$$\text{sim}(x, p) = \frac{\sum_{i} w_i \cdot x_i \cdot p_i}{\sqrt{\sum_{i} w_i \cdot x_i^2} \cdot \sqrt{\sum_{i} w_i \cdot p_i^2}}$$

Where $w_i$ is the attention weight for feature $i$.

### Feature Weight Update

$$w_i \leftarrow w_i + \eta \cdot |p_{\text{correct},i} - p_{\text{wrong},i}|$$

Features that differ between correct and wrong prototypes get higher weight.

### Prototype Attraction/Repulsion

$$p_{\text{correct}} \leftarrow p_{\text{correct}} + \alpha \cdot (x - p_{\text{correct}})$$
$$p_{\text{wrong}} \leftarrow p_{\text{wrong}} - \beta \cdot (x - p_{\text{wrong}})$$

Correct prototype moves toward input. Wrong prototype moves away.

## Real Brain Evidence

### The Hippocampus Does Pattern Separation

The hippocampus has a region called **CA3** that specifically does pattern separation — making similar inputs MORE different:

```
Input A: [0.8, 0.7, 0.9, 0.3]
Input B: [0.7, 0.8, 0.9, 0.4]

After CA3 pattern separation:
Output A: [1.0, 0.0, 1.0, 0.0]
Output B: [0.0, 1.0, 1.0, 0.0]
```

Similar inputs get mapped to VERY different internal representations. This is the neural basis of contrastive learning.

### Dopamine as Error Signal

When you misclassify something, dopamine neurons fire:
- **Expected reward but didn't get it** → dopamine drops → "I was wrong about this"
- This drives attention toward the features that WOULD have helped classify correctly

Not backpropagation. Local error signal. Simple. Effective.

## Why This Matters for Rethink AI

Without contrastive learning, our prototype system would:
- Confuse dogs and cats (too similar)
- Need many examples to separate close categories
- Treat all features equally

WITH contrastive learning:
- 1-2 examples suffice (it learns WHAT to pay attention to)
- Close categories get separated by amplifying distinguishing features
- The system gets smarter about HOW it compares things

---

*Next: [Build — Prototype Learner](04_build_prototype_learner.md)*
