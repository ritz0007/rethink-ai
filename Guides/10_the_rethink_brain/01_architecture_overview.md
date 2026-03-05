# 🧠 Architecture Overview: The Complete Rethink Brain

## The Full Picture

We've built 9 phases of brain components. Now it's time to connect them into a single, coherent system — **The Rethink Brain**.

## System Architecture

```
                    ┌──────────────────────────────────┐
                    │      COMMUNICATION (Phase 9)      │
                    │   Symbol Grounding + Encode/Decode │
                    └──────────┬───────────┬───────────┘
                               │           │
                    ┌──────────▼───────────▼───────────┐
                    │     UNDERSTANDING (Phase 8)       │
                    │  Predictive Coding + Causal Net   │
                    └──────────┬───────────┬───────────┘
                               │           │
            ┌──────────────────▼───────────▼──────────────────┐
            │              MODULAR BRAIN (Phase 7)             │
            │         Thalamus Routing + Signal Bus            │
            ├──────────┬──────┬──────┬──────┬────────┬────────┤
            │ Visual   │Memory│Learn │Think │Emotion │Execute │
            │ Region   │Region│Region│Region│Region  │Region  │
            ├──────────┼──────┼──────┼──────┼────────┼────────┤
            │ Retina   │Hopf. │Proto │Spont │Amygd.  │Causal  │
            │ SOM      │Decay │STDP  │Dream │Valence │Predict │
            │(Phase 3) │(P.4) │(P5/2)│(P.6) │(P.7)  │(P.8)   │
            └──────────┴──────┴──────┴──────┴────────┴────────┘
                    Built on: Spiking Neurons (Phase 1)
                              Hebbian Learning (Phase 2)
```

## Data Flow: A Complete Thought

When The Rethink Brain receives input, here's what happens:

### 1. Perception (50ms)
```
Raw input → Retina (edge detection, lateral inhibition)
         → SOM (feature clustering, spatial mapping)
         → Visual Region receives processed features
```

### 2. Recognition (100ms)
```
Visual features → Prototype Learner (which category?)
               → Hopfield Memory (have I seen this before?)
               → Emotion System (is this dangerous/rewarding?)
```

### 3. Understanding (200ms)
```
Recognized pattern → Predictive Coding (was this expected?)
                  → Causal Network (what caused this? what will it cause?)
                  → If unexpected: attention spike (thalamus prioritizes)
```

### 4. Response (300ms)
```
Understanding + Emotion → Executive Region (what should I do?)
                       → Communication (generate description/response)
                       → Motor output (act)
```

### 5. Learning (ongoing)
```
Everything → Hebbian/STDP updates (strengthen active connections)
          → Memory storage (important events get stored)
          → Prototype update (category knowledge evolves)
          → Causal learning (new cause-effect links)
```

### 6. Consolidation (sleep/idle)
```
Dream Engine → Replay important memories
            → Prune weak connections
            → Discover new patterns
            → Recombine old memories creatively
```

## Module Connections

| From | To | What's Sent | Why |
|------|----|-------------|-----|
| Retina | SOM | Edge features | Spatial mapping |
| SOM | Visual Region | Cluster IDs | Recognition |
| Visual Region | Prototype | Feature vectors | Classification |
| Visual Region | Hopfield | Patterns | Memory retrieval |
| Prototype | Thalamus | Category + confidence | Routing decision |
| Hopfield | Thalamus | Match + familiarity | Novelty detection |
| Thalamus | Predictive Net | Routed signals | Prediction update |
| Predictive Net | Causal Net | Prediction errors | Causal reasoning |
| Causal Net | Executive | Predicted effects | Action planning |
| Amygdala | Thalamus | Emotional weight | Attention control |
| Everything | Hebbian/STDP | Activity patterns | Learning |
| Dream Engine | All memories | Replay signals | Consolidation |
| CommEngine | Executive | Decoded symbols | Language input |
| Executive | CommEngine | Internal state | Language output |

## Design Principles

### 1. No Central Controller
There is no "main brain" function that orchestrates everything. Like the real brain, computation is **distributed**. Each module processes its own inputs and sends signals to others.

### 2. Competition, Not Instruction
Modules compete for attention through the thalamus. The most important/surprising/emotional signal wins. Nobody "tells" the brain what to focus on.

### 3. Learning Is Always On
Every signal that flows through the system changes it slightly. Hebbian learning and STDP are always active, always strengthening what's used and weakening what's not.

### 4. Memory Is Reconstruction
Nothing is stored as a perfect copy. Everything is reconstructed from patterns, which is why memory is creative and sometimes wrong — and that's a feature.

### 5. Understanding Is Prediction
The brain "understands" something when it can predict it. Surprise signals drive learning. Low surprise means the brain has a working model.

## What Makes This Different from Standard AI

| Standard AI | Rethink Brain |
|------------|---------------|
| Forward pass → backward pass | Continuous spiking activity |
| Backpropagation | Hebbian + STDP |
| Trained once, deployed | Always learning |
| Needs millions of examples | Few-shot from prototypes |
| No internal thought | Spontaneous activity + dreams |
| Tokens = arbitrary IDs | Grounded symbols |
| Doesn't know why | Causal reasoning |
| One monolithic model | Specialized cooperating regions |
| Static memory | Dynamic decay + consolidation |
| No emotion | Emotional modulation |

---

*Next: [Build — The Rethink Brain](02_build_rethink_brain.md)*
