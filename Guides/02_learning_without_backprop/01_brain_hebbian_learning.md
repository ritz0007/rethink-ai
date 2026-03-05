# 🧠 Hebbian Learning — "Neurons That Fire Together, Wire Together"

## The Core Idea

In 1949, a psychologist named Donald Hebb proposed a simple idea:

> **"When neuron A repeatedly helps fire neuron B, the connection from A to B gets stronger."**

This is often shortened to: **"Neurons that fire together, wire together."**

Think about it: when you study multiplication tables, you repeat "7 × 8 = 56" over and over. Each time, the same neurons fire together. Each repetition STRENGTHENS those connections. Eventually, "7 × 8" automatically triggers "56" — the pathway is burned in.

---

## Why This Matters

This is how YOUR brain learns. Not backpropagation. Not gradient descent. Not loss functions.

Just this one simple rule: **if A and B fire at the same time, strengthen the wire between them.**

And the opposite: **if A fires but B doesn't (or vice versa), weaken the wire.**

That's it. From this one rule, your brain learns to:
- Recognize your mother's face
- Catch a ball thrown at you
- Speak a language
- Play piano

---

## The Math (Simple Version)

The basic Hebbian learning rule:

```
Δw = η × (activity of source neuron) × (activity of target neuron)
```

Where:
- **Δw** = how much to change the connection weight
- **η** (eta) = learning rate — how fast to learn (small number like 0.01)
- **activity** = did the neuron fire? (1 if yes, 0 if no)

### Example:

```
Neuron A fires (activity = 1)
Neuron B fires (activity = 1)
η = 0.01

Δw = 0.01 × 1 × 1 = 0.01

Connection A→B weight increases by 0.01
```

```
Neuron A fires (activity = 1)  
Neuron B does NOT fire (activity = 0)
η = 0.01

Δw = 0.01 × 1 × 0 = 0.00

Connection doesn't change (they didn't fire together)
```

---

## The Problem with Pure Hebbian Learning

There's a catch: weights only ever INCREASE. If A and B keep firing together, the weight grows forever → the network goes haywire (every neuron fires all the time).

Real brains solve this with several mechanisms:

### 1. Weight Normalization
After updating, normalize all weights so they don't exceed a maximum. Like saying "you only have so much 'connection budget' to spend."

### 2. Decay
Weights slowly decrease over time unless reinforced. "Use it or lose it."

### 3. Inhibitory Competition
Inhibitory neurons prevent everything from firing at once. Only the strongest patterns survive.

### 4. Oja's Rule (Modified Hebbian)
A version that naturally prevents weights from exploding:

```
Δw = η × target_activity × (source_activity - w × target_activity)
```

The extra term `- w × target_activity` pulls the weight back down as it grows. It self-balances.

---

## How This Differs from Backpropagation

| Hebbian Learning | Backpropagation |
|-----------------|----------------|
| **Local**: only uses info from the two connected neurons | **Global**: needs error from the OUTPUT to flow backwards through EVERY layer |
| **No teacher needed**: learns from co-occurrence | **Needs labels**: "this image is a cat" |
| **Real-time**: learns as events happen | **Batch**: processes data, computes loss, then updates |
| **Unsupervised**: finds patterns without being told what they are | **Supervised**: told exactly what the right answer is |
| **Biological**: this is how real synapses change | **Not biological**: no known brain mechanism sends error gradients backward |
| **Simple**: one multiplication | **Complex**: chain rule through entire network |

---

## What We'll Build

In the next guide, we'll implement:

1. **Basic Hebbian rule** — strengthen connections between co-firing neurons
2. **Weight normalization** — prevent explosion
3. **Weight decay** — unused connections weaken
4. **A demo** — feed patterns to a network, watch it learn associations

The network from Phase 1 already has neurons and synapses. Now we're adding the ability to CHANGE those synapses based on activity. The network will learn.

---

## Real-World Analogy

Imagine you're learning guitar:
- First time: fingers fumble, sounds terrible → neural connections are weak
- 10th time: a bit smoother → connections getting stronger (Hebbian learning at work)
- 100th time: fingers move automatically → connections are strong, pattern is "burned in"
- Stop playing for 5 years: rusty → connections weakened (decay)
- Pick it up again: comes back faster than the first time → old connections reactivate

This is EXACTLY what our neural network will do.

---

*Next: [02_brain_stdp.md](02_brain_stdp.md) — Spike-Timing Dependent Plasticity (timing matters even more!)*
