# Phase 12 — Brain Science: Attention Networks

## Why Attention?

The brain receives **millions** of sensory signals every second — but you're only aware of a tiny fraction. Attention is the brain's **selection mechanism**: it decides what gets fully processed and what gets ignored.

Without attention, everything would be equally important — which means nothing would be.

---

## The Two Attention Networks

The brain has two major attention systems, discovered by Michael Posner and later refined by Corbetta & Shulman:

### 1. Dorsal Attention Network (Top-Down / Goal-Driven)

**Location**: Intraparietal sulcus (IPS) → Frontal eye fields (FEF)

**What it does**: You decide what to pay attention to.
- "I'm looking for my red bag" → you attend to red things
- "Listen for your name being called" → you attend to specific sound patterns
- Task-driven, voluntary, endogenous

**Properties**:
- **Slow to engage** (~200-300ms)
- **Sustained** — stays focused as long as the goal exists
- **Uses working memory** to maintain the attention template
- **Biases sensory processing** toward goal-relevant features

### 2. Ventral Attention Network (Bottom-Up / Stimulus-Driven)

**Location**: Temporoparietal junction (TPJ) → Ventral frontal cortex (VFC)

**What it does**: Something unexpected **grabs** your attention automatically.
- A sudden loud noise → your head turns
- Motion in peripheral vision → your eyes snap to it
- Pain → all other processing stops

**Properties**:
- **Fast** (~50-100ms, can interrupt ongoing processing)
- **Transient** — brief burst, then fades
- **Automatic** — you can't choose NOT to attend to a gunshot
- **Novelty-driven** — repeated stimuli lose their attention capture

---

## Biased Competition Model (Desimone & Duncan, 1995)

The dominant theory of how attention works at the neural level:

### Core Idea
Multiple stimuli **compete** for neural representation. Attention **biases** the competition in favor of one stimulus.

### How It Works

1. **Sensory input arrives** → multiple objects activate neural populations
2. **Competition**: neurons representing different objects suppress each other (lateral inhibition)
3. **Bias signal**: top-down input from prefrontal cortex amplifies the target population
4. **Winner is selected**: the biased population wins the competition
5. **Winner gains access** to memory, decision-making, motor output

### The Math of Competition

Think of N stimuli with activations $a_1, a_2, \ldots, a_N$:

- **Without attention**: winner = max(activations by input strength alone)
- **With bias**: $a_i' = a_i \cdot (1 + \text{bias}_i)$, winner = max(biased activations)

The bias can be:
- **Feature-based**: boost all "red" neurons across the visual field
- **Spatial**: boost all neurons in a specific location (spotlight)
- **Object-based**: boost the entire object once any part is selected

---

## Gain Modulation — How Attention Changes Processing

Attention doesn't change **what** a neuron detects; it changes **how strongly** it responds.

### Multiplicative Gain
$$\text{response} = \text{input} \times \text{gain}$$

- Attended neurons: gain > 1.0 (amplified)
- Unattended neurons: gain < 1.0 (suppressed)

This is exactly what the thalamus already does in our system — it's a gain control. V12 makes this principled.

### Contrast Gain
The more attention, the **sharper** the response:
- Attended stimuli are processed as if they were higher contrast/louder/stronger
- This is achieved by shifting the neuron's response curve leftward

---

## Attention as a Limited Resource

### The Bottleneck

Attention has **finite capacity** (maybe ~4 items simultaneously):
- **Change blindness**: if you're attending to one thing, you literally don't see changes elsewhere
- **Attentional blink**: after detecting one target, there's a ~200-500ms window where a second target is missed
- **Inattentional blindness**: the famous gorilla experiment

### Resource Allocation

The brain dynamically allocates attention:
- Difficult tasks demand more attention → less available for other things
- Well-practiced tasks need less → frees up capacity
- Emotional stimuli "steal" attention even from important tasks

---

## Three Types of Attention

### 1. Spatial Attention (WHERE)
- "Spotlight" that moves across the scene
- Enhances processing at a location
- Can be **overt** (eyes move) or **covert** (eyes stay but attention shifts)

### 2. Feature-Based Attention (WHAT)
- Boost a specific feature across the entire sensory field
- "Attend to red" enhances all red objects everywhere
- Works across spatial locations

### 3. Temporal Attention (WHEN)
- Focus processing at a specific moment in time
- "The light will flash... NOW" → enhanced processing at the expected moment
- Critical for rhythm, speech, music

---

## Our Implementation Strategy

We map these brain mechanisms to code:

| Brain Mechanism | Our Code | What It Does |
|----------------|----------|-------------|
| Dorsal attention (top-down) | `AttentionGoal` | User/brain sets a goal, biases processing |
| Ventral attention (bottom-up) | `attention_capture()` | Surprise/novelty grabs attention automatically |
| Biased competition | `attention_compete()` | Multiple stimuli compete, one wins |
| Gain modulation | `attention_modulate()` | Winner gets amplified, losers suppressed |
| Limited capacity | `attention_budget` | Finite pool, allocated dynamically |
| Feature attention | Feature bias weights | Boost specific feature dimensions |
| Spatial attention | Spatial spotlight | Boost processing at a location |
| Temporal attention | Temporal window | Boost processing at a time point |

---

## Key Insight

> **Attention isn't just "paying attention" — it's the brain's resource allocation system.** It decides what gets processed deeply and what gets ignored. Without it, every stimulus would compete equally, and the brain would be paralyzed by information overload.

The thalamus already gates signals by priority and surprise. V12 makes this a principled, multi-dimensional attention system with real competition dynamics.
