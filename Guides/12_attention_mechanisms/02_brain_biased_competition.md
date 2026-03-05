# Phase 12 — Brain Science: Biased Competition & Gain Control

## The Competition Problem

At any moment, the brain receives hundreds of stimuli. The retina sends visual information, the cochlea sends auditory signals, the skin sends touch. Each stimulus wants to be the one that gets processed, remembered, and acted upon.

**But only a few can.** The brain's processing bandwidth is finite.

---

## How Competition Works in the Brain

### Neural Competition (Lateral Inhibition)

Neurons representing different stimuli literally **suppress each other**:

```
Stimulus A → Neurons_A ⟶⟶ inhibits Neurons_B
Stimulus B → Neurons_B ⟶⟶ inhibits Neurons_A
```

The one with stronger activation wins. This is **winner-take-all** (softened in practice to winner-take-most).

### The Role of Receptive Fields

When two stimuli fall within the **same receptive field** of a neuron, they compete directly. When they're in separate receptive fields, they compete less. This is why attention helps most when the scene is **cluttered**.

---

## Biased Competition — The Full Model

### Step 1: Sensory Encoding
Raw input creates activation patterns. Multiple objects generate overlapping activations.

### Step 2: Mutual Suppression
Activations compete via inhibitory connections. Without bias, the strongest input wins.

### Step 3: Top-Down Bias
Prefrontal cortex sends a **bias signal** that adds activation to the target:

$$a_i^{biased} = a_i + \beta \cdot \text{goal\_match}(i)$$

Where $\beta$ is the bias strength and $\text{goal\_match}$ is how well stimulus $i$ matches the current attention goal.

### Step 4: Resolution
The biased stimulus wins the competition and gets:
- Full processing through the cortical hierarchy
- Access to working memory
- Ability to drive motor responses

### Step 5: Feedback
The winning stimulus sends **feedback signals** that further suppress losers. This creates a self-reinforcing loop.

---

## Gain Modulation — The Thalamic Gate

### What Is Gain?

Every signal passing through the thalamus is **multiplied by a gain factor**:

$$\text{output} = \text{input} \times \text{gain}(\text{attention})$$

- `gain > 1.0` → signal is **amplified** (attended)
- `gain = 1.0` → signal passes unchanged
- `gain < 1.0` → signal is **suppressed** (unattended)
- `gain ≈ 0.0` → signal is effectively **silenced**

### How Gain Changes with Attention

**Without attention** (uniform gain):
```
Stimulus A (strength 0.8) × gain 1.0 = 0.80
Stimulus B (strength 0.6) × gain 1.0 = 0.60
Stimulus C (strength 0.3) × gain 1.0 = 0.30
```

**With attention on B** (biased gain):
```
Stimulus A (strength 0.8) × gain 0.5 = 0.40  ← suppressed
Stimulus B (strength 0.6) × gain 2.0 = 1.20  ← amplified!
Stimulus C (strength 0.3) × gain 0.5 = 0.15  ← suppressed
```

B now has the highest effective activation despite having weaker raw input.

### Divisive Normalization

The brain uses **divisive normalization** — each activation is divided by the total:

$$r_i = \frac{a_i^n}{\sigma^n + \sum_j a_j^n}$$

This ensures:
- Responses are relative (not absolute)
- Attention to one thing suppresses response to others
- The total neural activity stays bounded

---

## Attention Capture — The Surprise Override

Some things grab attention automatically, bypassing the goal-driven system:

### What Captures Attention

1. **Abrupt onsets** — something suddenly appearing
2. **Unique features** — the one red item among green items
3. **Motion** — especially in the periphery
4. **Threat** — emotional/survival-relevant stimuli
5. **Your name** — cocktail party effect

### The Priority Map

The brain maintains a **priority map** that combines:
- Bottom-up salience (how much each location stands out)
- Top-down relevance (how much each location matches the goal)
- Emotional salience (how threatening/rewarding something is)

$$\text{priority}(i) = \alpha \cdot \text{salience}(i) + \beta \cdot \text{relevance}(i) + \gamma \cdot \text{emotion}(i)$$

The highest-priority location wins the competition and gets attended.

---

## Our Implementation Design

### AttentionSystem Structure

```
AttentionSystem
├── goals[]           — what we're looking for (top-down)
├── spotlight          — current focus position
├── items[]            — competing stimuli
├── gains[]            — per-item gain modulation
├── budget             — remaining attention capacity
├── salience_weights   — bottom-up capture sensitivity
└── priority_map[]     — combined priority scores
```

### The Attention Cycle (per tick)

1. **Compute salience** for each item (novelty, onset, surprise)
2. **Compute relevance** for each item (goal match)
3. **Build priority map** (salience + relevance + emotion)
4. **Compete**: items suppress each other via inhibition
5. **Select winner**: highest priority item gets full gain
6. **Modulate**: apply gain to all items (winner amplified, others suppressed)
7. **Output**: modulated features for downstream processing
8. **Update budget**: winner consumes attention resources

---

## Key Insight

> **Attention is competitive resource allocation.** The brain treats processing power like money — there's only so much to go around. Attention is the bidding system: bottom-up salience places bids, top-down goals place bids, and the highest bidder wins access to the brain's full processing pipeline.

Our existing thalamus already does basic gating. V12 replaces the simple float with real competition, real gain modulation, and a real resource budget.
