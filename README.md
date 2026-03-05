# Nous — A Brain That Thinks

> **νοῦς** (Greek: *mind*) — AI built to reason like a brain, not predict like a spreadsheet.

**Nous** is a brain-inspired AI system written entirely in raw C. No Python. No PyTorch. No backpropagation. No dependencies. Just neurons, synapses, and neuroscience.

It perceives the world through vision, hearing, and touch. It learns from one or two examples. Its memories fade and strengthen naturally. It thinks spontaneously, dreams during sleep, feels emotions, reasons about cause and effect, models other minds, monitors its own confidence, and talks to you — all using mechanisms grounded in how the human brain actually works.

**~7,800 lines of C. 24 brain modules. Zero external libraries.**

---

## Talk To It

Nous understands natural language. No need to memorize commands.

```
nous> hi
  Hello! I'm Nous — a brain-inspired AI with 24 modules.
  I just came online. Everything's fresh. Teach me something!

nous> this is a cat
  Brain learned category 'cat'

nous> this is a dog
  Brain learned category 'dog'

nous> what do you know
  Learned categories (2):
    [0] cat  (1 examples)
    [1] dog  (1 examples)

nous> how are you
  I'm doing great! Feeling neutral (·_·).
  Confidence: 50% | Stamina: 100% | Memories: 0 | Categories: 2

nous> what are you
  I'm Nous (Greek for 'mind') — a brain-inspired AI.
  Built from scratch in raw C with zero dependencies.
  I have 24 modules modeling real brain regions...

nous> tell me something
  I use Hebbian learning — 'neurons that fire together, wire together.'
  (Confidence: 50% | Strategy: 'memory_lookup')

nous> bye
  Goodbye! I've learned 2 categories today. Thanks for teaching me!
```

It also accepts all 50+ exact commands (`teach`, `show`, `why`, `whatif`, `sleep`, `introspect`, etc.) for precise control. Type `help` to see them all.

---

## Quick Start

### Requirements

- **GCC** (any version supporting C11)
- **make**
- Nothing else. Seriously.

### Build & Run

```bash
cd src
make          # Builds librethink.a static library (24 modules)
make chat     # Builds the Nous chatbot
./chatbot     # Talk to Nous

make test     # Builds integration test
./test_brain  # Runs "A Day in the Life" demo
make clean    # Clean build artifacts
```

---

## 24 Brain Modules

Every module models a real brain region. No shortcuts. No black boxes.

| # | Module | Brain Region | What It Does |
|---|--------|-------------|-------------|
| 1 | **LIF Neuron** | Biological neuron | Leaky Integrate-and-Fire spiking |
| 2 | **Synapse** | Synaptic junction | Weighted connections with delays |
| 3 | **Network** | Neural circuit | Spiking network with spike routing |
| 4 | **Hebbian** | Cortical plasticity | "Fire together, wire together" |
| 5 | **STDP** | Synaptic timing | Spike-timing dependent plasticity |
| 6 | **Prototype** | Inferotemporal cortex | Few-shot category learning (1-3 examples) |
| 7 | **Retina** | Primary visual cortex (V1) | Edge detection + lateral inhibition |
| 8 | **SOM** | Cortical maps | Self-organizing topographic maps |
| 9 | **Auditory** | Primary auditory cortex (A1) | Cochlea frequency decomposition, onset detection |
| 10 | **Tactile** | Somatosensory cortex (S1) | 8-channel touch, homunculus weighting, pain fast-path |
| 11 | **Multi-Modal** | STS + Superior Colliculus | Bayesian precision-weighted multi-sensory binding |
| 12 | **Hopfield** | Hippocampus | Associative content-addressable memory |
| 13 | **Decay** | Hippocampus | Ebbinghaus forgetting curve + consolidation |
| 14 | **Spontaneous** | Default Mode Network | Thinking without input, mental modes |
| 15 | **Dream** | Sleep circuits | NREM replay + REM recombination + insight |
| 16 | **Brain Bus** | Thalamus | Signal routing + attention gating |
| 17 | **Emotion** | Amygdala | Fear/reward processing, emotional memory |
| 18 | **Attention** | Dorsal/ventral networks | Biased competition, gain modulation, limited budget |
| 19 | **Predictor** | Prefrontal cortex | Hierarchical predictive coding |
| 20 | **Causal** | Prefrontal + temporal | Forward/backward causal reasoning + counterfactuals |
| 21 | **Comm** | Broca/Wernicke areas | Grounded symbol communication |
| 22 | **Motor** | Basal ganglia + cerebellum | GO/NO-GO action selection, forward model, RPE |
| 23 | **Social** | TPJ + mPFC + vmPFC | Theory of Mind, trust, mirror neurons, empathy |
| 24 | **Meta** | aPFC + ACC + dlPFC | Confidence monitoring, strategy selection, self-model |

One `rethink_experience()` call triggers the full pipeline: **perceive → predict → emote → classify → remember → reason**.

---

## What Makes Nous Different

| Traditional AI | Nous |
|---------------|------|
| Needs millions of examples | Learns from 1-3 examples |
| Backpropagation | Hebbian + STDP (biologically plausible) |
| Static between queries | Thinks spontaneously, even without input |
| No real memory | Memories fade, strengthen with use, consolidate in sleep |
| Predicts tokens | Reasons about causes and effects |
| Flat emotion (none) | Amygdala modulates attention and learning |
| No self-awareness | Monitors own confidence, adapts strategies |
| No social understanding | Models other minds, tracks trust, feels empathy |
| Python + GPU + frameworks | Raw C. Zero dependencies. ~7,800 lines. |
| Command-only interface | Understands natural language conversation |

---

## Training Nous

### Interactive Teaching

```
nous> teach cat
nous> teach dog
nous> this is a bird
nous> show fluffy_thing
nous> what
  → 'cat' (confidence: 0.95)
```

### Batch Training from File

Create a training file (see `data/` folder for examples):

```
# Format: one fact per line
teach cat
teach dog
cause rain -> wet_ground
cause wet_ground -> slipping
vocab sunshine
vocab thunder
```

Then:

```
nous> train data/animals.txt
```

### Training Data Sources

For larger-scale training, these open datasets can be converted to Nous's format:

| Dataset | What It Contains | URL |
|---------|-----------------|-----|
| **ConceptNet** | Commonsense knowledge (IsA, HasA, UsedFor, Causes) | https://conceptnet.io |
| **WordNet** | Lexical database — word meanings, hierarchies | https://wordnet.princeton.edu |
| **Simple English Wikipedia** | Human-readable concept descriptions | https://simple.wikipedia.org/wiki/Main_Page |
| **NELL** | Never-Ending Language Learner — extracted facts | http://rtw.ml.cmu.edu/rtw/ |
| **DailyDialog** | 13k conversational exchanges | http://yanran.li/dailydialog |
| **Open Mind Common Sense** | Crowd-sourced commonsense | https://github.com/commonsense/conceptnet5 |

**Conversion guide**: See [Guides/training_data_guide.md](Guides/training_data_guide.md) for how to convert these datasets into Nous training format.

---

## Project Structure

```
nous/
├── src/                          # All C source code (~7,800 lines)
│   ├── neurons/                  # neuron.c, network.c, synapse.h
│   ├── learning/                 # hebbian.c, stdp.c, prototype.c
│   ├── perception/               # retina.c, som.c, auditory.c, tactile.c, multimodal.c
│   ├── memory/                   # hopfield.c, decay.c
│   ├── thinking/                 # spontaneous.c, dream.c
│   ├── modular/                  # brain.c, emotion.c, attention.c
│   ├── understanding/            # predictor.c, causal.c
│   ├── communication/            # comm.c
│   ├── motor/                    # motor.c
│   ├── social/                   # social.c
│   ├── meta/                     # meta.c
│   ├── rethink_brain.c/.h        # Master integration (24 modules → 1 brain)
│   ├── chatbot.c                 # Nous interactive chatbot (NLU + commands)
│   ├── test_brain.c              # Full integration test
│   ├── data/                     # Training data files
│   └── Makefile                  # Build system
├── Guides/                       # 86 brain-science learning guides
│   ├── 00_getting_started/       # Philosophy + C setup
│   ├── 01-15_*/                  # One folder per brain phase
│   └── training_data_guide.md    # How to train Nous
├── Learnings/                    # Personal insight journals
├── Mistakes/                     # Error logs (mistakes are progress)
├── archive/                      # Version history + changelog
├── STATUS.md                     # Detailed project status
└── README.md                     # This file
```

---

## Design Principles

1. **Brain science first.** Every module starts with neuroscience, not algorithms.
2. **No backpropagation.** All learning is biologically plausible (Hebbian + STDP).
3. **No Python.** Raw C11 for speed, transparency, and total control.
4. **No external libraries.** Only `stdlib.h`, `math.h`, `stdio.h`, `string.h`.
5. **Memory is natural.** Memories fade, strengthen with use, consolidate during sleep.
6. **Few-shot learning.** Categories learned from 1-3 examples, not millions.
7. **Thinking is spontaneous.** The brain has internal activity even without input.
8. **Emotion is computational.** Fear boosts attention. Reward drives learning.
9. **Language is grounded.** Words mean things because they're tied to experience.
10. **The brain monitors itself.** Confidence, error detection, strategy adaptation.

---

## Research Directions

Active areas for future versions:

| Direction | Brain Basis | Status |
|-----------|------------|--------|
| **Consciousness** | Global Workspace Theory, Integrated Information (IIT) | Planned |
| **Creativity** | Bisociation, analogy, divergent thinking | Planned |
| **Developmental Learning** | Critical periods, curriculum, self-directed exploration | Planned |
| **Neuroplasticity** | Synaptogenesis, pruning, structural change | Planned |
| **Curiosity** | Intrinsic motivation, novelty-seeking, surprise-driven learning | Planned |

---

## Philosophy

> *"I see AI as a part of our brain and I want it to process the input as our brain does physically instead of some stupid algorithms. It should understand instead of just prediction."*

Nous is an experiment in building AI from the first principles of neuroscience — not linear algebra. It's intentionally different from everything else out there. It doesn't need GPUs, cloud APIs, or terabytes of training data. It needs neurons, synapses, and the courage to build something new.

---

## Stats

| Metric | Value |
|--------|-------|
| Language | C (C11) |
| Total lines | ~7,800 |
| Source files | 52 (25 .c + 25 .h + Makefile + test) |
| Brain modules | 24 |
| External dependencies | 0 |
| Chatbot commands | 50+ (exact) + natural language |
| Guide files | 86 |
| Build time | < 3 seconds |

---

## License

MIT License. See [LICENSE](LICENSE) for details.

---

*Built with neurons, not matrices. Named after the Greek word for mind.*
*Nous V1 — by Ritesh.*
