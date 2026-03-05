# 🧠 Brain Science: How We Hear — The Auditory Cortex

## You Already See. Now You Need to Hear.

In Phase 3, we built a retina — edge detection inspired by the primary visual cortex (V1). But the brain doesn't just see. It hears, touches, smells, tastes. Each sense has its own dedicated processing pipeline, and they all converge.

Vision is spatial: "what's where."  
Hearing is temporal: "what's when."

This difference is fundamental. The auditory system is optimized for patterns in **time**, not space.

## The Auditory Pipeline

Sound enters the ear as pressure waves. By the time it reaches your conscious awareness, it's been through at least 6 processing stages:

### 1. Cochlea (Inner Ear)
The cochlea is a spiral tube filled with fluid. Different positions along the spiral resonate at different frequencies — high frequencies near the base, low frequencies at the tip.

This is a **frequency decomposition**, mathematically similar to a Fourier transform. The brain doesn't hear "sound" — it hears a spectrum of frequencies.

**Key insight**: The cochlea is a biological spectrogram.

### 2. Cochlear Nucleus (Brainstem)
The first relay. Neurons here detect:
- **Onset**: When a sound starts (burst detectors)
- **Offset**: When a sound stops
- **Steady state**: Sustained tones vs. transient clicks

This is already categorizing sounds into temporal patterns.

### 3. Superior Olive (Brainstem)
This is where the brain computes **where** a sound comes from:
- **Interaural time difference (ITD)**: Sound arrives at the closer ear first (microsecond precision!)
- **Interaural level difference (ILD)**: Sound is louder in the closer ear

The brain uses these tiny differences to build a spatial map of sound sources.

### 4. Inferior Colliculus (Midbrain)
Integrates all the information so far:
- Frequency content (what)
- Temporal pattern (how it changes)
- Spatial location (where)

Begins to detect complex patterns like frequency sweeps (rising/falling tones).

### 5. Medial Geniculate Nucleus (Thalamus)
The auditory thalamus — gatekeeper for what reaches consciousness. This is where attention filters sound. You can be in a noisy room and still hear your name — the thalamus is doing that filtering.

**Connection to Phase 7**: Our thalamus router already does priority-based gating. Sound would flow through this same gate.

### 6. Primary Auditory Cortex (A1)
The final processing stage before conscious perception. Organized **tonotopically** — neighboring neurons respond to neighboring frequencies, just like the retina is organized retinotopically.

A1 neurons detect:
- **Pure tones** at specific frequencies
- **Frequency combinations** (harmonics = same sound)
- **Temporal modulations** (rhythm patterns)
- **Spectro-temporal features** (rising, falling, flat)

## What Makes Hearing Different from Vision

| Property | Vision (V1) | Hearing (A1) |
|----------|-------------|-------------|
| Primary dimension | Space (2D pixel grid) | Frequency (1D spectrum) |
| Secondary dimension | Color / intensity | Time (onset, duration) |
| Feature detection | Edges, corners | Onsets, harmonics |
| Temporal resolution | ~30 Hz (video rate) | ~1000 Hz (millisecond) |
| Key processing | Convolution (spatial filters) | FFT (frequency analysis) |
| Object binding | Spatial proximity | Temporal coherence |

## Tonotopic Maps — The Auditory Equivalent of Retinotopy

Just as V1 has a map of visual space, A1 has a map of frequency space. Low-frequency neurons are at one end, high-frequency at the other. This is called a **tonotopic map**.

Our SOM (Self-Organizing Map) from Phase 3 would work perfectly here — let it self-organize on frequency features instead of spatial features.

## Biological Inspiration for Our Code

What we need to model:

1. **Frequency decomposition**: Convert a time-domain signal into frequency bins (like the cochlea)
2. **Onset/offset detection**: Detect when sounds start and stop (like the cochlear nucleus)
3. **Temporal patterns**: Track how the frequency spectrum changes over time (like A1)
4. **Feature extraction**: Produce a feature vector that represents "what this sounds like"

We don't need to model the full pipeline. We need the **computational essence**:
- **Cochlea** → frequency bin decomposition
- **Temporal processing** → onset detection + spectral change tracking
- **A1 features** → spectro-temporal feature vector output

## Why This Matters for Rethink AI

A brain with only vision is like a person who can see but not hear. It can recognize objects but not:
- Detect sounds as warnings (emotional system should respond to loud bangs)
- Associate sounds with visual objects (a cat looks like X AND sounds like "meow")
- Process temporal information at the resolution needed for language

Adding hearing makes multi-modal binding possible — and that's where real understanding begins.

---

*Next: [Brain Science — Touch and the Somatosensory Cortex](02_brain_somatosensory.md)*
