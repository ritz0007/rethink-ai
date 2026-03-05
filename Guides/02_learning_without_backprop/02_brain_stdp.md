# 🧠 STDP — Spike-Timing Dependent Plasticity (Timing Is Everything)

## Beyond "Fire Together, Wire Together"

Basic Hebbian learning says: if A and B fire together → strengthen the connection.

But "together" is vague. What if A fires 1 millisecond before B? What about 50ms before? What if A fires AFTER B?

In the late 1990s, scientists discovered that the **exact timing** between spikes matters enormously. This is called **Spike-Timing Dependent Plasticity (STDP)**.

---

## The Rule

It's beautifully simple:

### If A fires BEFORE B → Strengthen the connection A→B
> "A caused B to fire" → A should get credit → make the connection stronger

### If A fires AFTER B → Weaken the connection A→B  
> "A didn't cause B" → A is irrelevant to B's firing → make the connection weaker

### The closer in time, the bigger the change
> Spikes 1ms apart → big change  
> Spikes 50ms apart → tiny change  
> Spikes 100ms apart → basically no change

---

## Visual: The STDP Window

```
Connection 
strength change
(Δw)
     ^
     |    
  +  |  ╲
  i  |    ╲      "A fired BEFORE B"
  n  |      ╲    → STRENGTHEN (LTP)
  c  |        ╲
  r  |          ╲
  e  |            ╲───────────
  a  |                         
  s  |                          
  e  ├─────────────────────────────────→ Time difference
     |            0                       (Δt = t_B - t_A)
  d  |                          
  e  |            ╱───────────
  c  |          ╱
  r  |        ╱   "A fired AFTER B"
  e  |      ╱     → WEAKEN (LTD) 
  a  |    ╱
  s  |  ╱
  e  |╱
  -  |
```

The curve looks like this:
- **Left side (Δt > 0, A fires before B)**: weight INCREASES — this is called **Long-Term Potentiation (LTP)**
- **Right side (Δt < 0, A fires after B)**: weight DECREASES — this is called **Long-Term Depression (LTD)**
- **Peak**: closest in time = biggest change
- **Tails**: far apart in time = no change

---

## The Math

```
Δt = t_post - t_pre    (when did the target fire minus when did the source fire)

If Δt > 0 (source fired first → strengthen):
    Δw = A_plus × exp(-Δt / tau_plus)

If Δt < 0 (source fired second → weaken):
    Δw = -A_minus × exp(Δt / tau_minus)
```

Where:
- **A_plus** = maximum strengthening amount (e.g., 0.01)
- **A_minus** = maximum weakening amount (e.g., 0.012 — slightly bigger, which prevents runaway growth)
- **tau_plus** = time window for strengthening (e.g., 20ms)
- **tau_minus** = time window for weakening (e.g., 20ms)
- **exp** = exponential decay — the further apart in time, the less effect

### Example:

```
Neuron A fires at t = 10ms
Neuron B fires at t = 13ms

Δt = 13 - 10 = 3ms (positive → A fired first → strengthen!)

Δw = 0.01 × exp(-3 / 20) = 0.01 × 0.86 = 0.0086

→ Weight increases by 0.0086 (strong change, very close in time)
```

```
Neuron A fires at t = 25ms
Neuron B fires at t = 10ms

Δt = 10 - 25 = -15ms (negative → A fired AFTER B → weaken)

Δw = -0.012 × exp(-15 / 20) = -0.012 × 0.47 = -0.0057

→ Weight decreases by 0.0057 (moderate change)
```

---

## Why STDP Is Powerful

### 1. It discovers CAUSATION
If A consistently fires before B, the connection strengthens. If A fires randomly relative to B, changes cancel out (some strengthening, some weakening). Only reliable causal relationships survive.

### 2. It's completely LOCAL
Each synapse only needs to know:
- When did MY source neuron fire?
- When did MY target neuron fire?
- How far apart in time?

No global error signal. No backward pass. Just local timing.

### 3. It creates SEQUENCES
If A→B→C always fire in that order, STDP strengthens A→B and B→C. The network learns temporal sequences — like the notes of a melody.

### 4. It's biologically REAL
STDP has been measured in actual neurons in the lab. This isn't a theory — it's a measured physical process in real brains.

---

## How STDP + Hebbian Work Together

| Learning Rule | What It's Good At |
|--------------|-------------------|
| **Basic Hebbian** | Detecting things that happen together (associations) |
| **STDP** | Detecting things that happen in ORDER (sequences, causation) |

In our network, we'll use both:
- Hebbian for basic pattern learning
- STDP for learning sequences and causal structures

---

## Parameters We'll Use

| Parameter | Value | What It Controls |
|-----------|-------|-----------------|
| `A_plus` | 0.01 | Max strengthening per spike pair |
| `A_minus` | 0.012 | Max weakening per spike pair (slightly stronger to prevent runaway) |
| `tau_plus` | 20.0ms | Time window for strengthening |
| `tau_minus` | 20.0ms | Time window for weakening |
| `w_max` | 5.0 | Maximum allowed weight |
| `w_min` | 0.0 | Minimum allowed weight (no negative weights for excitatory) |

---

## Coming Up

In the build guides, we'll implement both Hebbian learning and STDP, and demonstrate:
1. A network that learns to associate two patterns (Hebbian)
2. A network that learns a sequence (STDP)
3. The difference between them

---

*Next: [03_brain_why_backprop_is_not_real.md](03_brain_why_backprop_is_not_real.md) — Why backpropagation doesn't happen in real brains*
