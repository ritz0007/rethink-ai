# Phase 13 — The Motor System: Brain Science

> How the brain plans, executes, and learns from actions

---

## The Problem

Phases 1–12 built a brain that can **perceive, remember, reason, and communicate** — but it can't **DO** anything. It has no way to:

- Select an action from many possibilities
- Plan a sequence of movements (reach → grasp → lift)
- Predict the outcome of an action BEFORE doing it
- Learn from the consequences of actions (rewards and errors)

In biology, the motor system is what converts thought into action. Without it, the brain is a prisoner inside its skull.

---

## Motor Cortex — The Action Commander

### Primary Motor Cortex (M1)

The motor cortex sits just in front of the somatosensory cortex (they're neighbors for a reason — sensation and action are deeply linked).

**Key properties:**
- **Somatotopic organization**: Like the sensory homunculus, there's a MOTOR homunculus. The hand and mouth have massive cortical representation because they need fine control.
- **Population coding**: A single neuron doesn't command a single muscle. Instead, a POPULATION of neurons votes on the direction and force of movement.
- **Direct pathway**: M1 → spinal cord → muscles. Fast but crude.

### Premotor Cortex (PMC)

Sits in front of M1. Does action PLANNING, not execution.

**Key properties:**
- **Action selection**: Which action to perform (reach vs. avoid vs. push)
- **Contextual modulation**: The same visual input can trigger different actions depending on context
- **Observed actions**: PMC activates when WATCHING others act → mirror neuron foundation

### Supplementary Motor Area (SMA)

Handles sequences — putting individual actions in ORDER.

**Key properties:**
- **Sequence planning**: "First reach, then grasp, then lift" is planned here
- **Internally generated actions**: SMA is more active for self-initiated than externally triggered actions
- **Bilateral coordination**: Coordinates both hands (e.g., opening a jar)

---

## Basal Ganglia — The Action Selector

The basal ganglia is the brain's **action selection circuit**. It decides WHICH action to execute based on reward history.

### The Circuit

```
Cortex (many possible actions)
    ↓
Striatum (receives all options)
    ↓ ↓
Direct pathway    Indirect pathway
(GO signal)       (NO-GO signal)
    ↓                 ↓
GPi / SNr (output — default is INHIBIT everything)
    ↓
Thalamus → Motor Cortex → Execute!
```

### Key Principle: Disinhibition

The basal ganglia works by **REMOVING inhibition**, not by exciting:

1. By default, GPi/SNr INHIBITS the thalamus → no action
2. When the direct pathway activates, it INHIBITS GPi → removes the inhibition → action happens
3. The indirect pathway STRENGTHENS the inhibition → suppresses competing actions

**This is a competitive selection mechanism** — similar to our attention system!

### Dopamine and Learning

The substantia nigra pars compacta (SNc) releases **dopamine** into the striatum:

- **Unexpected reward** → dopamine burst → strengthen the GO pathway for this action
- **Expected reward** → normal dopamine → maintain current behavior
- **Reward omission** → dopamine DIP → weaken the GO pathway

This is the **Reward Prediction Error (RPE)**:

$$RPE = R_{actual} - R_{predicted}$$

- $RPE > 0$: Better than expected → learn to do this more
- $RPE = 0$: As expected → no update
- $RPE < 0$: Worse than expected → learn to avoid this

---

## Cerebellum — The Error Corrector

The cerebellum contains **more neurons than the rest of the brain combined** (~80%). Its job: precision and timing.

### Forward Model

The cerebellum maintains a **forward model** — it predicts the sensory consequences of an action:

$$\hat{s}_{t+1} = f(s_t, a_t)$$

Where:
- $s_t$ = current sensory state
- $a_t$ = planned action
- $\hat{s}_{t+1}$ = predicted next state

### Error Correction

After execution:

$$e = s_{actual} - \hat{s}_{predicted}$$

This **sensory prediction error** updates the forward model via climbing fiber signals from the inferior olive. Over time, the cerebellum makes actions smoother and more accurate.

### Timing

The cerebellum is the brain's **master clock** for motor timing. It learns when to start, when to stop, and how to coordinate sub-movements.

---

## Motor Sequences — Chunking

Complex actions are built from **chunks** — sequences of primitive actions that become automated:

1. **Learning phase**: Each step is consciously controlled (slow, effortful)
2. **Chunking phase**: Steps merge into a single unit (faster, less effort)
3. **Automatic phase**: The whole sequence fires as one chunk (fast, effortless)

**Example**: Learning to drive:
- Step 1: Turn key → check mirrors → press clutch → shift gear (each step conscious)
- Step 2: "Start car" = one chunk, "Begin driving" = another chunk
- Step 3: You drive without thinking about individual steps

The basal ganglia handles chunk selection, the SMA handles sequence ordering, and the cerebellum handles timing within chunks.

---

## Our Implementation Plan

We'll build a motor system with:

| Component | Brain Region | Our Implementation |
|-----------|-------------|-------------------|
| Action representation | M1 | Feature vectors for actions |
| Action selection | Basal ganglia | Reward-based selection with disinhibition |
| Sequence planning | SMA | Ordered step chains |
| Forward model | Cerebellum | Predict outcome before executing |
| Error correction | Cerebellum | Learn from prediction errors |
| Motor learning | Dopamine/RPE | Reward prediction error updates |
| Action repertoire | Procedural memory | Library of learned actions |

The key insight: **action is not just output — it's a loop**. The brain predicts what will happen, acts, compares reality to prediction, and updates. This is active inference.

---

## References (Brain Science)

- Georgopoulos et al. (1982) — Population coding in motor cortex
- Alexander & Crutcher (1990) — Basal ganglia direct/indirect pathways
- Schultz (1997) — Dopamine and reward prediction error
- Wolpert & Ghahramani (2000) — Computational principles of movement neuroscience
- Doya (1999) — Complementary roles of basal ganglia and cerebellum in learning
- Hikosaka et al. (2002) — Motor sequence learning and the basal ganglia
