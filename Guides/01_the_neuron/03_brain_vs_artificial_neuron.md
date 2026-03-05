# 🧠 Brain vs Artificial Neuron — Why Standard AI Neurons Are "Wrong"

## Two Models, Side by Side

Let's compare what most AI uses vs what your brain actually does.

---

## The Standard Artificial Neuron (Perceptron)

Invented by Frank Rosenblatt in 1958. Simple, elegant, and **not a neuron**:

```
inputs:  x₁ = 0.5,  x₂ = 0.8,  x₃ = 0.2
weights: w₁ = 0.3,  w₂ = 0.7,  w₃ = 0.1
bias:    b = 0.1

Step 1: Weighted sum = (0.5×0.3) + (0.8×0.7) + (0.2×0.1) + 0.1
                     = 0.15 + 0.56 + 0.02 + 0.1
                     = 0.83

Step 2: Activation = sigmoid(0.83) = 0.696

Output: 0.696
```

That's it. One multiplication, one addition, one function call. Done.

### What's Going On:
- Takes inputs as **numbers** (continuous values)
- Multiplies each by a **weight** (how important that input is)
- Adds them all up plus a **bias** (offset)
- Passes through an **activation function** (sigmoid, ReLU, etc.)
- Outputs a **number** (continuous value)

### Learning:
- Calculate how wrong the output was (loss)
- Use **backpropagation** — send the error backwards through the whole network
- Adjust weights using **gradient descent** — calculus-based optimization
- Repeat millions of times with millions of examples

---

## The Real Biological Neuron (What We're Building)

```
Time: 0.0ms  Voltage: -70.0mV  (resting, waiting)
Time: 0.1ms  Input arrives from neuron #47: +3.2mV
Time: 0.1ms  Voltage: -66.8mV  (accumulated, still below threshold)
Time: 0.2ms  Leak: -0.15mV
Time: 0.2ms  Voltage: -66.95mV (leaked a bit)
Time: 0.3ms  Input from neuron #12: +5.1mV
Time: 0.3ms  Voltage: -61.85mV (getting close to -55 threshold...)
Time: 0.4ms  Input from neuron #89: +4.0mV
Time: 0.4ms  Voltage: -57.85mV (closer!)
Time: 0.5ms  Leak: -0.14mV
Time: 0.5ms  Voltage: -57.99mV
Time: 0.6ms  Input from neuron #3: +4.5mV
Time: 0.6ms  Voltage: -53.49mV  >>> THRESHOLD CROSSED! <<<
Time: 0.6ms  >>> SPIKE! <<<
Time: 0.6ms  Voltage reset to: -75.0mV
Time: 0.6ms  Refractory period: 2.0ms (cannot fire)
Time: 0.7ms  Input from neuron #22: +6.0mV  (IGNORED — refractory)
Time: 0.8ms  Still refractory...
...
Time: 2.6ms  Refractory period over. Ready to receive again.
```

### What's Going On:
- Takes inputs as **spikes** (binary events — happened or didn't)
- **Accumulates** charge over time
- **Leaks** charge continuously
- **Fires** a spike when threshold is crossed (all-or-nothing)
- Goes into **refractory** period after firing
- Information is in the **TIMING** of spikes, not in continuous values

### Learning:
- **Hebbian**: If neuron A fires and causes neuron B to fire → strengthen A→B connection
- **STDP**: If A fires BEFORE B → strengthen. If A fires AFTER B → weaken.
- **No backward pass**: Learning happens LOCALLY at each connection, based only on the two neurons involved
- **No calculus**: No gradients, no loss functions, no optimization
- Needs **very few examples**: See one pattern a few times → learned it

---

## The Comparison

| Feature | Artificial Neuron | Real Neuron (Our Model) |
|---------|------------------|------------------------|
| **Input** | Numbers (0.5, 0.8) | Spikes (fired / not fired) |
| **Processing** | Instant: weighted sum + activation | Over time: accumulate, leak, threshold |
| **Output** | A number (0.696) | A spike (fired / not fired) |
| **Time** | Doesn't exist — one computation | Central — everything happens in time |
| **Memory** | None (stateless per forward pass) | Voltage carries "short-term memory" |
| **Learning rule** | Backpropagation (global error signal) | Hebbian/STDP (local co-firing) |
| **Learning speed** | Needs millions of examples | Can learn from few examples |
| **Connectedness** | Layer-by-layer only | Any neuron can connect to any other |
| **Refractory** | No | Yes — can't fire right after firing |
| **Inhibition** | Negative weights (same math) | Distinct inhibitory neurons |

---

## Why Does Standard AI Use the Fake Neuron?

Because it works incredibly well for certain tasks! The artificial neuron + backpropagation is:

- **Easy to compute** — just matrix multiplication (GPUs love this)
- **Easy to optimize** — calculus gives us exact gradients
- **Scalable** — stack millions of fake neurons in layers
- **Proven** — powers GPT, image recognition, self-driving cars

But it has fundamental limitations:
- Needs insane amounts of data
- Doesn't actually understand anything
- Catastrophic forgetting
- No concept of time
- No spontaneous activity
- One big blob, not modular

---

## The Rethink AI Bet

We're betting that going back to **biological principles** — even simplified ones — will lead to AI that:

1. **Learns faster** (fewer examples)
2. **Remembers better** (graceful forgetting, not catastrophic)
3. **Actually understands** (predicts and models, not just pattern-matches)
4. **Thinks on its own** (spontaneous activity, dreaming)

Is this guaranteed to work? No. That's why we log mistakes. That's why we version everything. That's science.

---

## Important Note

We're NOT saying standard neural networks are useless. They're amazing tools. We're saying they're **not the only way** — and for understanding and general intelligence, the brain's approach might teach us something fundamentally different.

---

*Next: [04_build_lif_neuron.md](04_build_lif_neuron.md) — Let's build our first real spiking neuron in C!*
