# Phase 13 — Action Selection & Reward Learning

> The basal ganglia model: how the brain picks actions and learns from outcomes

---

## The Core Problem: Action Selection

At any moment, the brain has many possible actions. The basal ganglia solves:

> Given the current state and multiple possible actions, which one should I execute?

This is different from classification (which is "what is this?"). Action selection is "what should I DO?"

---

## Disinhibition Model

### The Default: Everything is Blocked

The basal ganglia's output nuclei (GPi/SNr) tonically INHIBIT the thalamus:

```
GPi → Thalamus: "STOP. Don't move. Don't do anything."
```

This is the resting state. Without input, the animal is motionless.

### Direct Pathway: GO

When a desirable action is recognized:

```
Cortex → Striatum (D1 neurons) → inhibit GPi → release Thalamus → GO!
```

Double negative: inhibiting the inhibitor = excitation. This is **disinhibition**.

### Indirect Pathway: NO-GO

When an undesirable action should be suppressed:

```
Cortex → Striatum (D2 neurons) → GPe → STN → excite GPi → stronger STOP
```

This makes the inhibition STRONGER for competing actions.

### Net Effect: Competition

```
Action A:  GO signal = 0.8,  NO-GO signal = 0.2  → NET = 0.6  → EXECUTE
Action B:  GO signal = 0.3,  NO-GO signal = 0.7  → NET = -0.4 → SUPPRESS
Action C:  GO signal = 0.5,  NO-GO signal = 0.5  → NET = 0.0  → HOLD
```

Winner is the action with highest NET = GO - NO-GO.

---

## Reward Prediction Error

### Dopamine Signal

The SNc computes a **prediction error**:

$$\delta = r + \gamma V(s') - V(s)$$

Where:
- $r$ = actual reward received
- $V(s')$ = value of new state
- $V(s)$ = value of old state (what was predicted)
- $\gamma$ = discount factor

### Three Cases

| Signal | Meaning | Dopamine | Learning |
|--------|---------|----------|----------|
| $\delta > 0$ | Better than expected | Burst | Strengthen GO |
| $\delta = 0$ | As expected | Baseline | No change |
| $\delta < 0$ | Worse than expected | Dip | Strengthen NO-GO |

### The Update Rule

For the GO pathway:
$$w_{GO} \leftarrow w_{GO} + \alpha \cdot \delta \cdot e$$

For the NO-GO pathway:
$$w_{NOGO} \leftarrow w_{NOGO} - \alpha \cdot \delta \cdot e$$

Where:
- $\alpha$ = learning rate
- $\delta$ = reward prediction error
- $e$ = eligibility trace (which action was taken)

When $\delta > 0$: GO gets stronger, NO-GO gets weaker → more likely to repeat
When $\delta < 0$: GO gets weaker, NO-GO gets stronger → less likely to repeat

---

## Forward Model (Cerebellum)

Before executing an action, the cerebellum predicts the outcome:

```
Current state: "hand at rest, ball on table"
Action: "reach forward"
Predicted outcome: "hand near ball"
```

### Prediction Error → Correction

After executing:
- **Predicted**: hand stops at ball position
- **Actual**: hand overshoots
- **Error**: overshoot by X amount
- **Update**: reduce reach magnitude by X

Over many trials, the forward model becomes accurate and movements become smooth.

### Our Implementation

$$\hat{r} = \mathbf{w} \cdot \mathbf{f}_{action}$$

Where:
- $\hat{r}$ = predicted reward
- $\mathbf{w}$ = forward model weights (learned)
- $\mathbf{f}_{action}$ = action feature vector

The prediction error:

$$\delta = r_{actual} - \hat{r}$$

Updates the forward model:

$$\mathbf{w} \leftarrow \mathbf{w} + \alpha \cdot \delta \cdot \mathbf{f}_{action}$$

---

## Motor Sequences

### Sequence Representation

A motor sequence is an ordered list of primitive actions:

```
"Pick up cup" = [reach, open_hand, close_hand, lift]
```

Each primitive action has:
- Feature vector (what kind of action)
- Duration (how long it takes)
- Preconditions (what state must be true)
- Expected effect (what changes)

### Chunking

As a sequence is repeated, it becomes a **chunk** — a single unit:

1. First execution: [reach][pause][open][pause][close][pause][lift] — 7 steps
2. After practice: [reach+open][close+lift] — 2 chunks
3. Expert: [pick_up] — 1 chunk

Chunking is implemented by:
- Detecting temporal adjacency (A always follows B)
- Merging adjacent actions into a super-action
- The super-action inherits combined features

---

## Connection to Other Systems

| Connection | Direction | Purpose |
|-----------|-----------|---------|
| Perception → Motor | Input | "I see a ball" triggers "reach for ball" |
| Emotion → Motor | Bias | Fear inhibits approach, reward facilitates approach |
| Attention → Motor | Filter | Only attended objects trigger actions |
| Causal → Motor | Planning | "If I push, it will fall" → plan accordingly |
| Memory → Motor | Recall | "Last time I reached, I got reward" → repeat |
| Motor → Prediction | Update | Action outcomes update the forward model |

---

## What We'll Build

```
MotorSystem
├── Action Repertoire (library of known actions)
│   ├── action[0]: features[32], go_weight, nogo_weight, reward_avg
│   ├── action[1]: ...
│   └── action[N]: ...
├── Sequence Planner
│   ├── current_plan: steps[], num_steps, expected_reward
│   └── step: name, action_id, duration, expected_effect
├── Forward Model
│   ├── weights[32] (predict reward from features)
│   └── prediction_error (last error)
├── Selector (basal ganglia)
│   ├── evaluate(state, actions) → best action
│   └── exploration vs exploitation (epsilon)
└── Learner (dopamine)
    ├── compute RPE
    ├── update GO/NOGO weights
    └── update forward model
```
