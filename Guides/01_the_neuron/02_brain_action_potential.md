# 🧠 The Action Potential — How a Spike Actually Happens

## The Spike in Detail

When we said "the neuron fires," what actually happens? Let's zoom in.

---

## The Resting State

When a neuron is just sitting there doing nothing, it has a **resting voltage** of about **-70 millivolts** (mV). 

Why negative? Because there's a difference in electrical charge between the inside and outside of the cell. The inside is slightly more negative. Think of it as the "zero" point — the baseline.

```
Voltage
  ^
  |
  |  Resting: -70mV
  |  ─────────────────────────── (baseline, doing nothing)
  |
  +──────────────────────────────► Time
```

---

## The Buildup

Signals from other neurons arrive. Each one adds a little charge:

- **Excitatory signal** → voltage goes UP (toward 0 and beyond)
- **Inhibitory signal** → voltage goes DOWN (more negative)

Meanwhile, the leak is draining charge back toward -70mV.

```
Voltage
  ^
  |          signal!  signal!
  |            ↓        ↓
 -55|  - - - - - - THRESHOLD - - - - - -
  |         /\    /\
 -70|  ────/  \──/  \────── (didn't reach threshold, leaked back)
  |
  +──────────────────────────────► Time
```

---

## The Spike

If enough signals arrive fast enough to push the voltage PAST the threshold (-55mV), something dramatic happens:

1. **Rapid rise** — Voltage shoots up from -55mV to about +40mV in less than 1 millisecond
2. **Peak** — Hits maximum (+40mV)
3. **Rapid fall** — Voltage plummets back down, actually going BELOW resting (-80mV)
4. **Recovery** — Slowly returns to resting (-70mV)

```
Voltage
  ^
 +40|           /\      ← THE SPIKE (Action Potential)
  |          /  \
  |         /    \
  0|        /      \
  |       /        \
 -55|  - -/- THRESHOLD \ - - - - - - -
  |     /            \
 -70|  ──/              \────── ← Back to resting
 -80|                    \___/ ← Undershoot (refractory)
  |
  +──────────────────────────────► Time
     |←─────── ~2ms ──────→|
```

The whole thing takes about **2 milliseconds**. It's incredibly fast.

---

## Key Properties of the Spike

### 1. All-or-Nothing
The spike is always the same size. There's no "small fire" or "big fire." Either threshold is crossed and you get a full spike, or threshold isn't crossed and you get nothing.

This is FUNDAMENTALLY different from artificial neurons that output smooth values like 0.73.

### 2. The Refractory Period
Right after the spike, the neuron enters two phases:

- **Absolute refractory** (~1ms): The neuron absolutely CANNOT fire. No amount of input will make it fire.
- **Relative refractory** (~2-4ms): The neuron CAN fire, but needs a STRONGER than normal signal.

This is like a gun that needs to be reloaded. It creates a natural **maximum firing rate** (about 500 spikes per second, though most neurons fire much slower).

### 3. Information is in the TIMING
Since every spike is the same size, how does the brain encode information? Through **timing**:

- **Fire rate**: More spikes per second = stronger signal (rate coding)
- **Exact timing**: WHEN the spike happens matters (temporal coding)
- **Patterns**: Groups of neurons firing in specific sequences

This is why OUR neurons will track time, not just compute numbers.

---

## What We'll Model in Code

For Rethink AI, we'll use the **Leaky Integrate-and-Fire (LIF)** model. It captures the essential behavior without the full biological complexity:

```
For each tiny time step (dt):
    
    1. Add up all incoming signals
    2. Leak: voltage moves toward resting value
    3. Add input: voltage += incoming signals
    4. Check: if voltage > threshold AND not refractory:
         → FIRE! Set spike = 1
         → Reset voltage to reset value (below resting)
         → Start refractory timer
    5. If refractory: count down timer, ignore input
```

### The LIF Equation (don't worry about the math, we'll build it step by step):

$$\tau_m \frac{dV}{dt} = -(V - V_{rest}) + R \cdot I(t)$$

In plain English:
- **The voltage changes over time**
- **It naturally decays toward resting** (the leak — first term)
- **Input current pushes it up or down** (second term)
- **τ (tau)** controls how fast the leak happens (big τ = slow leak, small τ = fast leak)

When $V > V_{threshold}$: SPIKE! Then reset $V = V_{reset}$.

---

## The Parameters We Need

| Parameter | What It Is | Typical Value | In Our Code |
|-----------|-----------|---------------|-------------|
| `V_rest` | Resting voltage | -70 mV | -70.0 |
| `V_threshold` | Fire when exceeded | -55 mV | -55.0 |
| `V_reset` | Voltage after firing | -75 mV | -75.0 |
| `tau_m` | Membrane time constant (leak speed) | 10-20 ms | 20.0 |
| `R` | Resistance (how much input affects voltage) | 10 MΩ | 10.0 |
| `t_refractory` | Refractory duration | 2-5 ms | 2.0 |
| `dt` | Simulation time step | 0.1-1.0 ms | 0.1 |

---

## Why NOT Use the Full Hodgkin-Huxley Model?

The real biological spike involves sodium and potassium ion channels opening and closing. The full Hodgkin-Huxley model captures this with 4 differential equations. It won a Nobel Prize.

But for Rethink AI, the LIF model is enough because:
1. It captures the **essential behavior** — accumulate, leak, fire, refractory
2. It's **fast to compute** — important when simulating thousands of neurons
3. The **interesting stuff** happens at the NETWORK level, not inside individual neurons
4. We can always upgrade later if needed

---

## Coming Next

In the next guide, we'll look at why standard "artificial neurons" are wrong, and then we'll BUILD our first real spiking neuron in C.

---

*Next: [03_brain_vs_artificial_neuron.md](03_brain_vs_artificial_neuron.md) — Why the standard AI neuron is a lie*
