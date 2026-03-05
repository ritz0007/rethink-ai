# 🧠 Receptive Fields and Lateral Inhibition

## Receptive Fields — Each Neuron Has Its Own "Window"

A single neuron in V1 doesn't see the entire image. It only responds to stimuli in a tiny region — its **receptive field**.

```
Imagine the image is a 28×28 grid (like an MNIST digit):

┌────────────────────────────────┐
│                                │
│    ┌──────┐                    │
│    │ THIS │  ← This V1 neuron  │
│    │ AREA │    only "sees"      │
│    │ ONLY │    this 5×5 patch   │
│    └──────┘                    │
│                                │
│              ┌──────┐          │
│              │ AND  │          │
│              │ THIS │ ← Different│
│              │ ONE  │   neuron  │
│              └──────┘          │
└────────────────────────────────┘
```

Different neurons have overlapping receptive fields. Together, they "tile" the entire image. Like a mosaic — each piece covers one small area, but combined they cover everything.

---

## Edge Detection — How V1 Neurons Work

A V1 neuron with a vertical edge receptive field works like this:

```
Receptive field (3×3 patch):

Excitatory  Inhibitory
  region      region
   (+)         (-)
   
  +1  0  -1     If the left side is bright
  +1  0  -1  →  and right side is dark
  +1  0  -1     → STRONG response (vertical edge!)
```

This is exactly a convolution kernel! But in the brain:
- These filters SELF-ORGANIZE (no one programs them)
- They're not neat 3×3 grids — they're messy, overlapping, various sizes
- The neuron doesn't output a number — it FIRES or stays silent

---

## Lateral Inhibition — Competition Between Neighbors

When a neuron fires strongly, it SUPPRESSES its neighbors through **inhibitory connections**. This is called **lateral inhibition**.

Why? Because it:
1. **Sharpens edges** — makes boundaries between features crisper
2. **Reduces redundancy** — nearby neurons don't all encode the same thing
3. **Creates competition** — only the "winner" gets to represent that region

```
Without lateral inhibition:
  Neuron A: "I see an edge!" (strength 0.9)
  Neuron B: "I also see an edge!" (strength 0.85)
  Neuron C: "Me too!" (strength 0.7)
  → Redundant. All three say the same thing.

With lateral inhibition:
  Neuron A: "I see an edge!" (strength 0.9)
  Neuron A suppresses B and C
  Neuron B: "..." (suppressed)
  Neuron C: "..." (suppressed)
  → Clean. One neuron represents the edge.
```

This is the brain's version of "winner takes all."

---

## What We'll Implement

In `retina.c`:
- Take a grid of pixel values (0-255)
- Apply simple edge detection (horizontal, vertical, diagonal)
- Lateral inhibition to sharpen features
- Output: which "edge neurons" fire for a given image

In `som.c`:
- Feed edge-detected features to a Self-Organizing Map
- The SOM learns to represent different patterns
- No backprop — competitive Hebbian learning

---

*Next: [03_brain_lateral_inhibition.md](03_brain_lateral_inhibition.md) → [04_build_retina.md](04_build_retina.md)*
