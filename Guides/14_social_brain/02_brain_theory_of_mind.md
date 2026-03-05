# Theory of Mind, Trust, and Imitation — The Mathematics

> Rethink AI — Phase 14, Guide 2 of 5

---

## Theory of Mind (TPJ)

### What Is Theory of Mind?

Theory of Mind is the ability to attribute *mental states* to others: beliefs, desires, intentions, knowledge. It lets you predict behavior by reasoning about internal states you can't directly observe.

**Formally**: For each agent $a$, we maintain a model $M_a$ consisting of:
- **Beliefs** $B_a = \{(concept, certainty)\}$ — what they know
- **Desires** $D_a \in \mathbb{R}^d$ — what they want (feature vector)
- **Emotional state** $e_a \in [-1, 1]$ — how they feel

### Belief Tracking

When we observe agent $a$ experiencing event $x$:

$$B_a(x) \leftarrow B_a(x) + \alpha \cdot (1 - B_a(x))$$

When time passes without reinforcement:

$$B_a(x) \leftarrow B_a(x) \cdot \gamma \quad (\gamma < 1)$$

This mirrors our own memory decay — we assume others forget at similar rates.

### Prediction from Mental Models

To predict what agent $a$ will do, we simulate their decision using their mental model:

$$\text{predicted\_action}(a) = \arg\max_i \left[ D_a \cdot F_i \cdot B_a(\text{relevant}) \right]$$

Where $F_i$ is the feature vector of action $i$. We predict they'll choose the action most aligned with their desires, weighted by what they believe is possible.

**Prediction accuracy** tracks how well our model matches reality:

$$\text{accuracy}_a \leftarrow \text{accuracy}_a + \alpha \cdot (\text{correct} - \text{accuracy}_a)$$

---

## Trust Computation (vmPFC)

### Bayesian Trust Model

Trust is fundamentally a **prediction about future cooperation**. The brain tracks this as a running estimate, updated by experience.

**Initial trust**: $T_0 = 0.5$ (neutral — no evidence either way)

**After cooperative interaction**:
$$T_{n+1} = T_n + \alpha_{\text{trust}} \cdot (1 - T_n) \cdot r$$

Where $r$ is the cooperation quality $[0, 1]$.

**After defective interaction**:
$$T_{n+1} = T_n - \beta_{\text{trust}} \cdot T_n \cdot |d|$$

Where $d$ is the defection severity $[0, 1]$.

**Key asymmetry**: Trust is slow to build, fast to destroy. This matches the neuroscience — the amygdala reacts strongly to betrayal, creating a negativity bias:

$$\beta_{\text{trust}} > \alpha_{\text{trust}}$$

In our implementation: $\alpha_{\text{trust}} = 0.1$, $\beta_{\text{trust}} = 0.3$ (3× faster to destroy).

### Trust-Gated Behavior

Trust modulates how we interact:
- **High trust** ($T > 0.7$): Cooperate freely, share information, accept help
- **Medium trust** ($0.3 < T < 0.7$): Cautious, verify claims, limited sharing
- **Low trust** ($T < 0.3$): Guard information, verify everything, expect defection

---

## Mirror Neuron System — Imitation Learning

### How Mirror Neurons Work

When we observe agent $a$ perform action $i$:

1. **Match**: Find the closest action in our motor repertoire
   $$j^* = \arg\max_j \text{cosine}(F_{\text{observed}}, F_j)$$

2. **Activate**: Mirror neuron fires proportional to match quality
   $$\text{activation} = \text{cosine}(F_{\text{observed}}, F_{j^*})$$

3. **Learn** (if activation exceeds threshold): Strengthen the matching action
   $$GO_{j^*} \leftarrow GO_{j^*} + \alpha_{\text{mirror}} \cdot \text{activation} \cdot T_a$$

Note: Trust gates imitation learning. We imitate trusted agents more readily.

### Imitation as Social Learning

If the observed action doesn't match anything in our repertoire (all similarities low), we **create a new action** — learning entirely by observation:

$$\text{if } \max_j \text{cosine}(F_{\text{observed}}, F_j) < \theta_{\text{novel}}$$
$$\rightarrow \text{motor\_add\_action}(\text{observed\_name}, F_{\text{observed}})$$

This is how children learn most of their actions — not by trial-and-error, but by watching others.

---

## Social Reward Signals

### The Neurochemistry

- **Oxytocin**: Released during cooperation → enhances trust, bonding
- **Dopamine** (VTA): Social reward — cooperation activates the same reward circuits as food/water
- **Serotonin**: Regulates social hierarchy position and mood
- **Cortisol**: Stress hormone when social bonds are threatened

### Implementation

Social reward is computed as:

$$R_{\text{social}} = R_{\text{helping}} \cdot \text{empathy} + R_{\text{being\_helped}} \cdot \text{need} + R_{\text{cooperation}} \cdot T_a$$

Where:
- $R_{\text{helping}}$ = reward for helping others (scaled by empathy)
- $R_{\text{being\_helped}}$ = reward for receiving help (scaled by need)
- $R_{\text{cooperation}}$ = reward for mutual cooperation (scaled by trust)

Social reward feeds into the same dopamine system as motor reward (V13), creating a unified reward signal that values both physical and social outcomes.

---

## Empathy as Simulation

### The Simulation Theory

The brain understands others' emotions by *simulating* them internally:

1. **Observe**: STS detects another agent's emotional expression
2. **Map**: Mirror system maps the expression onto our own emotional circuits
3. **Feel**: Anterior insula generates a weakened version of the same feeling
4. **Modulate**: Empathy level scales the intensity

$$e_{\text{empathic}} = e_{\text{observed}} \cdot \text{empathy\_level} \cdot \text{familiarity}_a$$

High empathy + high familiarity = strong shared emotion.

---

## Familiarity — The Key Modulator

Everything in social cognition improves with familiarity:

| Feature | Low Familiarity | High Familiarity |
|---------|----------------|-----------------|
| Belief tracking | Poor (generic assumptions) | Accurate (calibrated model) |
| Desire prediction | Generic (average desires) | Specific (personalized) |
| Trust precision | Wide variance | Narrow variance |
| Empathy | Weaker | Stronger |
| Imitation gating | Higher threshold | Lower threshold |

Familiarity grows with interaction:

$$F_a \leftarrow F_a + \alpha_F \cdot (1 - F_a) \quad \text{per interaction}$$

And slowly decays without contact:

$$F_a \leftarrow F_a \cdot \gamma_F \quad \text{per time step without interaction}$$

---

## From Brain to Code — Design Summary

```
SocialSystem
├── AgentModel[8] ─── TPJ: beliefs, desires, emotional state
│                  ├── vmPFC: trust score + cooperation history
│                  ├── familiarity (interaction count)
│                  └── prediction accuracy
├── MirrorNeurons ─── Premotor: match observed actions → motor repertoire
│                  ├── activation threshold
│                  └── trust-gated imitation learning
├── SocialReward ──── VTA: helping, being helped, cooperation bonuses
│                  └── empathy level (anterior insula)
└── InteractionHistory ─── Ring buffer of past social events
```

---

*Next: [03_build_social.md](03_build_social.md) — Header design and struct definitions*
