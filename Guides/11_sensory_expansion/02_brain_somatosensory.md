# 🧠 Brain Science: Touch — The Somatosensory Cortex

## The Forgotten Sense

Vision and hearing get all the attention in AI. But touch is how the brain grounds itself in physical reality. You can close your eyes and still know where your body is, what you're holding, whether it's hot or cold.

Touch is not one sense — it's at least four:
- **Pressure** (mechanoreception)
- **Temperature** (thermoreception)
- **Pain** (nociception)
- **Proprioception** (body position sense)

## The Somatosensory Pipeline

### 1. Skin Receptors

The skin contains specialized neurons, each tuned to different aspects of touch:

| Receptor | Detects | Speed | Receptive Field |
|----------|---------|-------|----------------|
| **Merkel cells** | Sustained pressure, edges, texture | Slow | Small, precise |
| **Meissner corpuscles** | Light touch, vibration (10-50Hz) | Fast | Small |
| **Pacinian corpuscles** | Deep vibration (100-300Hz) | Very fast | Large, diffuse |
| **Ruffini endings** | Skin stretch, sustained pressure | Slow | Large |

**Key insight**: This is parallel processing. Four different receptor types analyze the same stimulus simultaneously, just like our retina uses 4 edge kernels on the same image.

### 2. Spinal Cord (Dorsal Horn)

Touch signals enter the spinal cord and split into two pathways:
- **Fast pathway** (dorsal column): Fine touch, pressure, vibration → goes straight to thalamus
- **Slow pathway** (spinothalamic): Pain, temperature → crosses to opposite side, reaches thalamus later

**Why two pathways?** Speed vs. urgency. You need to know the shape of what you're holding (fast, precise). You also need to know if it's burning you (slow, but triggers withdrawal reflex BEFORE reaching consciousness).

This maps directly to our amygdala fast-path from Phase 7.

### 3. Thalamus (VPL Nucleus)

Touch signals pass through the thalamus — just like vision and hearing. The same attention gating applies: you don't notice your clothes touching your skin because the thalamus filters it out.

### 4. Primary Somatosensory Cortex (S1)

Organized **somatotopically** — there's a map of your body surface in your brain. This is the famous **sensory homunculus**: areas with more nerve endings (fingertips, lips) get more cortical space.

S1 neurons detect:
- **Edge orientation** on the skin (similar to visual V1!)
- **Texture gradients** (rough vs. smooth)
- **Pressure patterns** (shape recognition by touch)
- **Temperature changes** (warming vs. cooling)

### 5. Secondary Somatosensory Cortex (S2)

Higher-level processing:
- **Bilateral integration**: Combine left hand + right hand into one percept
- **Object recognition by touch**: "This feels like a pen" (haptic recognition)
- **Texture classification**: Categorizing surfaces

## The Homunculus — Biological Feature Weighting

The sensory homunculus is nature's attention weighting. Body parts with more receptors get more brain space:

```
Fingertips:  ████████████████████  (huge cortical area)
Palm:        ████████
Forearm:     ███
Back:        ██
```

This is equivalent to an **attention weight vector** — not all inputs are equally important. Our tactile module should model this by assigning different sensitivities to different input channels.

## What We'll Model

We don't need to simulate actual skin. We need the **computational principles**:

1. **Multi-channel sensing**: Multiple feature types from the same input (pressure, temperature, texture)
2. **Spatial resolution weighting**: Some channels more sensitive than others (homunculus)
3. **Temporal dynamics**: Adaptation (sustained touch fades, onsets are amplified)
4. **Feature extraction**: Convert raw tactile input into a feature vector compatible with our brain bus

### Tactile Feature Space

Each tactile experience becomes a vector:
```
[pressure, temperature, texture_roughness, texture_regularity, 
 onset_strength, vibration_freq, pain_level, proprioceptive_angle]
```

This is analogous to the retina's edge features — a compact representation the rest of the brain can work with.

## Touch and Emotion

Touch has a privileged connection to emotion:
- **Gentle stroking** activates C-tactile fibers → pleasant feeling
- **Sharp pain** → amygdala fast-path → fear/withdrawal
- **Temperature extremes** → danger signals

Our amygdala module should evaluate tactile features just like it evaluates visual ones. A "hot" signal should trigger stronger emotional response than a "warm" signal.

## Touch and Memory

"I've felt this before" — tactile memories are strong and persistent. The texture of tree bark, the warmth of a coffee mug. These get stored as feature vectors in our decay memory with importance weighted by emotional content.

---

*Next: [Brain Science — Multi-Modal Binding: How the Brain Integrates Senses](03_brain_multisensory_binding.md)*
