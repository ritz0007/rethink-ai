# 🧠 How the Visual Cortex Works

## From Light to Meaning

Right now, your eyes are receiving millions of tiny light signals. Photons hit your retina, trigger electrical signals, and travel through the optic nerve to the back of your brain — the **visual cortex**.

But you don't see "millions of individual light points." You see a screen, words, a room, a world. How?

---

## The Visual Processing Pipeline

Your visual cortex processes information in **layers**, from simple to complex:

```
RETINA → V1 → V2 → V4 → IT (Inferotemporal Cortex)
  |       |      |      |          |
Pixels  Edges  Shapes  Colors   Objects & Faces
                Curves  Textures  "That's a cat"
```

### Layer 1: Retina (the camera)
- 130 million photoreceptor cells
- Converts light → electrical signals
- Already does some processing: detects contrast, brightness changes

### Layer 2: V1 (Primary Visual Cortex — edge detector)
- First stop in the brain
- Neurons here respond to **edges at specific angles**
- One neuron fires for vertical edges, another for horizontal, another for 45°
- They only "see" a tiny patch of the image — their **receptive field**

```
This V1 neuron     This V1 neuron     This V1 neuron
fires for:         fires for:         fires for:
    |               ──                 /
    |                                 /
    |               ──               /
  vertical        horizontal       diagonal
```

### Layer 3: V2 (Shape combinations)
- Combines edges into **corners, curves, junctions**
- Started seeing "parts": the corner of a box, the curve of a circle

### Layer 4: V4 (Color + texture)
- Combines shapes with color and texture information
- Sees "a red curved thing" or "a fuzzy vertical thing"

### Layer 5: IT Cortex (Object recognition)
- The highest visual area
- Responds to **whole objects and faces**
- "That's a CAT" — regardless of angle, lighting, or size
- Some neurons (famously) respond ONLY to specific faces ("the Jennifer Aniston neuron")

---

## Key Principles We'll Steal

### 1. Receptive Fields
Each neuron only "sees" a small patch of the image. It doesn't process the whole picture — just its tiny window. This is like how a CNN uses small filters, but the brain version is more flexible and self-organizing.

### 2. Hierarchical Processing
Simple features → complex features → objects. Edges combine into shapes, shapes combine into objects. Each layer builds on the previous one.

### 3. Lateral Inhibition
Neighboring neurons COMPETE. When one neuron fires strongly, it suppresses its neighbors. This makes edges sharper and features more distinct. It's why you see optical illusions — your neurons are exaggerating contrast.

```
Input:    55555 | 22222    (slight difference in brightness)
                  
After lateral inhibition:
          55556 | 12222    (edge is now exaggerated!)
                  ↑↑
              These neurons were EXTRA suppressed by their bright neighbors
```

### 4. Self-Organization
Nobody PROGRAMS which V1 neuron responds to which edge angle. The neurons organize themselves through experience — they see edges, and through Hebbian learning, different neurons specialize for different edge orientations.

---

## What We'll Build

1. **A retina** — converts a pixel grid to neural signals, detects edges
2. **A Self-Organizing Map (SOM)** — neurons that organize themselves to represent patterns
3. **Pattern recognition** — classify handwritten digits WITHOUT backpropagation

---

*Next: [02_brain_receptive_fields.md](02_brain_receptive_fields.md) — How neurons "see" tiny patches*
