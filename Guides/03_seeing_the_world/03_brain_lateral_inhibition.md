# 🧠 Lateral Inhibition — Neurons Competing for Attention

## The Concept

Lateral inhibition is when active neurons SUPPRESS their neighbors. It's one of the brain's most powerful tricks.

### In the Retina
Even before signals reach the brain, your retina is already using lateral inhibition. It's why you can see sharp edges even in dim lighting — your neurons exaggerate contrast at boundaries.

### The Mach Band Illusion
Look at this gradient:

```
████████░░░░░░░░░
```

You probably see a bright band right at the edge where dark meets light, and a dark band on the other side. That's lateral inhibition — your neurons at the boundary are exaggerating the difference.

### How It Works

```
5 neurons in a row, each seeing a different brightness:

Input brightness:    [80]  [80]  [80]  [20]  [20]

Without inhibition:  [80]  [80]  [80]  [20]  [20]  (boring, accurate)

With inhibition (each neuron subtracts neighbors × 0.2):
  Neuron 1: 80 - 80×0.2         = 64    (suppressed by neighbor)
  Neuron 2: 80 - 80×0.2 - 80×0.2 = 48  (suppressed by both neighbors)
  Neuron 3: 80 - 80×0.2 - 20×0.2 = 60  (less suppressed — near the edge!)
  Neuron 4: 20 - 80×0.2 - 20×0.2 = 0   (heavily suppressed by bright neighbor!)
  Neuron 5: 20 - 20×0.2         = 16    (slightly suppressed)

Result:              [64]  [48]  [60]  [0]   [16]
                                  ↑      ↑
                              BRIGHT   DARK    ← Edge exaggerated!
```

The edge is now MUCH more obvious. Neuron 3 (bright side of edge) is relatively strong. Neuron 4 (dark side of edge) is suppressed to nearly zero.

---

## Why This Matters for Rethink AI

1. **Feature sharpening** — edges and boundaries pop out from the background
2. **Sparse coding** — only a few neurons fire (efficient!)
3. **Competition** — neurons compete to represent features (specialization emerges)

We'll implement lateral inhibition in our retina module to preprocess visual input before it reaches the learning modules.

---

*Next: [04_build_retina.md](04_build_retina.md) — Build the retina simulator in C*
