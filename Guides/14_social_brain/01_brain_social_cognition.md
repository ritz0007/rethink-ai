# The Social Brain — Neuroscience of Social Cognition

> Rethink AI — Phase 14, Guide 1 of 5

---

## Why a Social Brain?

Humans are the most social species on Earth. We spend enormous neural resources — roughly 10% of cortical volume — on understanding other minds. This isn't optional; it's fundamental. Intelligence evolved *because* of social pressure.

The "Social Brain Hypothesis" (Robin Dunbar, 1998) argues that primate brains got large not for tool use or foraging — but to track increasingly complex social relationships. Every human navigates a web of alliances, rivalries, debts, and reputations that requires constant mental modeling.

A brain without social cognition is like a chess engine that can't model its opponent. It can compute, but it can't anticipate, cooperate, or learn from others.

---

## The Social Brain Network

Several brain regions form a tightly connected "social brain network":

### 1. Temporoparietal Junction (TPJ) — Theory of Mind

**Location**: Junction of temporal and parietal lobes, bilateral
**Function**: Reasoning about others' *beliefs*, even when they differ from reality

The TPJ is the hardware for **Theory of Mind** (ToM) — the ability to understand that other agents have their own knowledge, desires, and intentions that may differ from yours.

**Classic example**: Sally-Anne test
- Sally puts a marble in a basket, then leaves the room
- Anne moves the marble to a box
- Where will Sally look for the marble?
- TPJ damage → "the box" (can't separate Sally's belief from reality)
- Healthy TPJ → "the basket" (models Sally's false belief)

**Key insight**: The TPJ maintains *separate mental models* for each known agent, distinct from our own knowledge state.

### 2. Medial Prefrontal Cortex (mPFC) — Mentalizing

**Location**: Midline, frontal lobe
**Function**: Thinking about others' mental states (desires, personality, preferences)

The mPFC activates when we:
- Judge someone's personality traits
- Predict what someone *wants* (not just what they know)
- Consider how someone will *feel* about an outcome
- Distinguish self from other

**Dorsal mPFC** → reasoning about dissimilar others (effortful, deliberate)
**Ventral mPFC** → reasoning about similar others (fast, intuitive)

### 3. Mirror Neuron System — Action Understanding

**Location**: Premotor cortex (F5) + inferior parietal lobule (IPL)
**Function**: Fire both when performing AND observing the same action

Discovered by Rizzolatti et al. (1996) in macaque monkeys:
- Neuron fires when monkey grasps a peanut
- *Same neuron* fires when monkey *watches* someone else grasp a peanut

This creates an automatic mapping: **seeing = doing** at the neural level.

Functions:
- **Action understanding**: Recognize *what* someone is doing by simulating it
- **Intention reading**: Differentiate "grasping to eat" from "grasping to place"
- **Imitation learning**: Copy observed actions into own motor repertoire
- **Empathy**: Map observed facial expressions to own emotional circuitry

### 4. Superior Temporal Sulcus (STS) — Social Perception

**Location**: Upper temporal lobe
**Function**: Detect biological motion, eye gaze, facial expressions

The STS is the entry point of social cognition:
- Distinguishes animate from inanimate motion (a person walking vs. a ball rolling)
- Tracks eye gaze direction → infers what someone is attending to
- Reads facial expressions → initial emotional classification
- Processes voice prosody (emotional tone)

### 5. Anterior Insula + Anterior Cingulate Cortex — Empathy

**Location**: Deep within lateral sulcus (insula), midline frontal (ACC)
**Function**: Map others' pain/emotions onto own body

When you see someone get hurt, your anterior insula activates — you literally *feel* a shadow of their pain. This isn't metaphor; it's measurable fMRI activation.

- **Anterior insula**: Generates the subjective feeling ("this hurts watching")
- **ACC**: Monitors the conflict/distress signal
- Together: The neural basis of empathy

### 6. Ventromedial PFC (vmPFC) — Trust and Social Reward

**Location**: Bottom-midline of frontal lobe
**Function**: Compute social value, trust, reputation

The vmPFC:
- Integrates social signals (facial expression, past behavior, group membership)
- Computes a **trust score** for each known individual
- Generates **social reward** signals (cooperation feels good)
- Tracks **reputation** (has this person been reliable?)

Damage to vmPFC → inability to build trust, poor social decisions (Phineas Gage)

---

## How the Social Network Connects

```
                    ┌─────────────┐
     Observation →  │     STS     │ ← Social perception
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
     ┌────────────┐  ┌──────────┐  ┌──────────┐
     │   Mirror   │  │   TPJ    │  │   mPFC   │
     │  Neurons   │  │  (ToM)   │  │ (Mental) │
     └─────┬──────┘  └────┬─────┘  └────┬─────┘
           │              │              │
           ▼              ▼              ▼
     ┌──────────┐  ┌──────────────┐  ┌──────────┐
     │ Imitate  │  │ Agent Model  │  │  Predict │
     │ (Motor)  │  │ (Beliefs)    │  │  (Desire)│
     └──────────┘  └──────────────┘  └──────────┘
                          │
                    ┌─────┴─────┐
                    ▼           ▼
              ┌──────────┐ ┌──────────┐
              │  Insula  │ │  vmPFC   │
              │ (Empathy)│ │ (Trust)  │
              └──────────┘ └──────────┘
```

**Flow**: STS detects a social event → Mirror neurons simulate the action → TPJ models the agent's beliefs → mPFC models their desires → Insula generates empathy → vmPFC updates trust.

---

## Key Principles for Our Implementation

1. **Separate agent models**: Each known agent gets its own mental model (beliefs + desires + trust)
2. **Mirror neurons**: Observed actions map to our motor repertoire (connects to V13)
3. **Bayesian trust**: Trust updates like a Bayesian prior — cooperative actions increase trust, defection decreases it
4. **Social reward**: Cooperation generates internal reward signals (connects to dopamine/emotion)
5. **Empathy as simulation**: We model others' emotions by running them through our own emotion system
6. **Familiarity matters**: Better predictions for well-known agents (more data → better model)

---

## What Makes This Hard (and Interesting)

- **Recursive mentalizing**: "I think that you think that I think..." (2nd-order ToM is common, 3rd-order is rare even in humans)
- **False beliefs**: Modeling what someone *wrongly* believes requires separating your knowledge from theirs
- **Deception detection**: Noticing when someone's behavior doesn't match their stated intentions
- **In-group/Out-group**: We model familiar agents far better than strangers

We'll implement first-order Theory of Mind (I model what you know) with trust and imitation. Higher-order recursion can come later.

---

*Next: [02_brain_theory_of_mind.md](02_brain_theory_of_mind.md) — The mathematics of mental models*
