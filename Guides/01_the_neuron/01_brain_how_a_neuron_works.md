# 🧠 How a Real Neuron Works

## Forget Everything You Know About "Artificial Neurons"

If you've seen any AI tutorial, you've seen this:

```
output = activation(weights × inputs + bias)
```

**That is NOT how a neuron works.** That's a mathematical shortcut invented in the 1940s. It's useful — it powers all of modern AI — but it's a cartoon of a neuron, not a neuron.

Let's look at the real thing.

---

## The Real Neuron

Your brain has about **86 billion neurons**. Each one is a tiny living cell that does one simple thing: it collects electrical signals, and if enough signals arrive, it FIRES — sends its own electrical signal to other neurons.

Here's the anatomy:

```
                    ┌─────────────────────┐
   Inputs from      │                     │
   other neurons    │      CELL BODY      │     Output to
   ─────────────►   │      (Soma)         │     other neurons
   ─────────────►   │                     │──────────────────►
   ─────────────►   │  Collects charge.   │
                    │  If charge > threshold:   AXON
  DENDRITES         │     → FIRE!         │  (long wire that
  (input wires)     │  If not:            │   carries the spike)
                    │     → keep waiting  │
                    └─────────────────────┘
```

### The Parts

1. **Dendrites** (input wires) — Receive signals from other neurons. A single neuron can have thousands of dendrites, connecting to thousands of other neurons.

2. **Cell Body (Soma)** — Where the signals combine. Think of it as a bucket that collects water (electrical charge). The charge is called the **membrane potential** (voltage).

3. **Axon** (output wire) — A long thin cable. When the neuron fires, the electrical signal travels down the axon to reach other neurons.

4. **Synapse** (connection point) — Where the axon of one neuron meets the dendrite of another. This is where the signal gets passed. There's a tiny gap (synaptic cleft) — the signal crosses it using chemicals called neurotransmitters.

---

## How Firing Works

Here's the key process, step by step:

### Step 1: Charge Accumulates
- Signals arrive from other neurons through dendrites
- Each signal adds (or subtracts!) a little charge to the cell body
- Some neurons are **excitatory** (add charge) and some are **inhibitory** (subtract charge)
- The current charge level is called the **membrane potential**

### Step 2: Leak
- The neuron is leaky! Charge slowly drains away over time
- If no new signals arrive, the voltage drifts back to resting level
- This is like a bucket with a small hole — you need to keep pouring water in to fill it

### Step 3: Threshold Check
- The neuron has a **threshold** voltage (typically around -55mV in real neurons)
- If the accumulated charge EXCEEDS the threshold → **FIRE!**
- If not → keep collecting, keep leaking

### Step 4: The Spike (Action Potential)
- When the threshold is crossed, the neuron generates a **spike** — a sudden, brief burst of electrical activity
- This spike is **all-or-nothing**: it's always the same strength. The neuron either fires or it doesn't. There's no "fire a little bit."
- The spike travels down the axon to reach other neurons

### Step 5: Refractory Period
- After firing, the neuron goes into a **refractory period** — it CANNOT fire again for a short time (about 1-2 milliseconds)
- During this time, the voltage resets to below resting level
- This prevents the neuron from firing continuously and gives a natural rhythm

### Step 6: Signal Reaches Other Neurons
- The spike arrives at the synapse
- Chemicals (neurotransmitters) cross the gap
- The signal arrives at the next neuron's dendrites
- The cycle starts again

---

## The Key Differences from "Artificial Neurons"

| Real Neuron | Artificial Neuron (standard AI) |
|------------|-------------------------------|
| Fires discrete SPIKES (on/off) | Outputs a continuous number (0.73, 0.28, etc.) |
| Has a LEAK — charge drains over time | No concept of time or leaking |
| Has REFRACTORY period — can't fire right after firing | Can output any time, no cooldown |
| Timing matters — WHEN signals arrive matters | Only cares about total weighted sum |
| Thousands of inputs from different neurons | Receives input from one layer only |
| Both excitatory (+) and inhibitory (-) connections | Weights can be positive or negative, but no biological distinction |
| Runs in REAL TIME — events are continuous | Processes in discrete forward passes |

---

## Why This Matters for Rethink AI

We're going to build neurons that actually:

- **Accumulate charge** over time (not just compute a weighted sum)
- **Leak** (charge decays if no input arrives)
- **Fire spikes** when threshold is crossed (not output a smooth number)
- **Go refractory** after firing (can't fire again immediately)
- **Live in time** — the simulation runs in tiny time steps (dt)

This is called the **Leaky Integrate-and-Fire (LIF)** model. It's the simplest model that captures the essential behavior of real neurons.

---

## In Simple Terms

Think of a neuron as a **water bucket with a hole**:

1. Other neurons pour water in (signals)
2. Water slowly drains through the hole (leak)  
3. If water level reaches the rim → **OVERFLOW!** (fire/spike)
4. After overflow, the bucket is emptied and blocked for a moment (refractory period)
5. Then it starts collecting again

That's it. 86 billion of these tiny buckets, connected to each other, creating everything you think, feel, remember, and imagine.

---

*Next: [02_brain_action_potential.md](02_brain_action_potential.md) — Deep dive into the spike itself*
