# Rethink AI

**AI built to mimic how the brain actually works — not statistical algorithms.**

Rethink AI is a brain-inspired artificial intelligence system written entirely in raw C. No Python. No PyTorch. No backpropagation. Just neurons, synapses, and biology.

It perceives, learns from few examples, remembers and forgets naturally, thinks without input, dreams, feels emotions, reasons about cause and effect, and communicates — all using mechanisms grounded in real neuroscience.

---

## Why This Exists

Current AI is impressive at prediction. But it doesn't *understand*. It trains on millions of examples, has no real memory, can't explain why something happened, and has no internal life when you're not prompting it.

The human brain does all of this with 20 watts of power.

Rethink AI asks: **what if we built AI the way the brain actually works?**

- Spiking neurons instead of matrix multiplication
- Hebbian learning instead of backpropagation
- Memory that fades, strengthens with use, and consolidates during sleep
- Emotion that modulates attention and learning
- Causal reasoning, not just correlation
- Grounded language, not statistical token prediction

---

## What's Inside

### 17 Brain Modules (all in `src/`)

| Module | Brain Region | What It Does |
|--------|-------------|-------------|
| **LIF Neuron** | Biological neuron | Leaky Integrate-and-Fire spiking |
| **Synapse** | Synaptic junction | Weighted + delayed connections |
| **Network** | Neural circuit | Spiking network with spike routing |
| **Hebbian** | Cortical plasticity | "Fire together, wire together" |
| **STDP** | Synaptic timing | Spike-timing dependent plasticity |
| **Prototype** | Inferotemporal cortex | Few-shot category learning |
| **Retina** | Visual cortex (V1) | Edge detection + lateral inhibition |
| **SOM** | Cortical maps | Self-organizing topographic maps |
| **Hopfield** | Hippocampus | Associative content-addressable memory |
| **Decay** | Hippocampus | Ebbinghaus forgetting + consolidation |
| **Spontaneous** | Default Mode Network | Thinking without input |
| **Dream** | Sleep circuits | NREM replay + REM recombination |
| **Brain Bus** | Thalamus | Signal routing + attention gating |
| **Emotion** | Amygdala | Fear/reward processing |
| **Predictor** | Prefrontal cortex | Hierarchical predictive coding |
| **Causal** | Prefrontal + temporal | Causal reasoning + counterfactuals |
| **Comm** | Broca/Wernicke areas | Grounded symbol communication |

All modules are connected through `rethink_brain.c` — one `rethink_experience()` call triggers the entire perception → emotion → memory → understanding pipeline.

### 10-Phase Learning Guide (56 markdown files)

Each phase follows the pattern: **learn the brain science → understand why it matters → build the code → verify with a milestone.**

| Phase | Title | You Learn | You Build |
|-------|-------|-----------|-----------|
| 00 | Getting Started | Philosophy, C setup | Project structure |
| 01 | The Neuron | Action potentials, LIF model | Spiking neurons + network |
| 02 | Learning Without Backprop | Hebbian, STDP, why backprop is fake | Learning rules |
| 03 | Seeing the World | Visual cortex, receptive fields | Retina + SOM |
| 04 | Memory | Hippocampus, forgetting curves | Hopfield + decay memory |
| 05 | Learning Like a Child | Few-shot, prototype theory | Prototype learner |
| 06 | Thinking Without Input | Default Mode Network, dreaming | Spontaneous activity + dreams |
| 07 | The Modular Brain | Brain regions, thalamus routing | Brain bus + emotion |
| 08 | Understanding | Predictive coding, causality | Predictor + causal network |
| 09 | Communication | Symbol grounding, language areas | Communication engine |
| 10 | The Rethink Brain | Full integration | Complete brain |

---

## Quick Start

### Requirements

- **GCC** (any version supporting C11)
- **make**
- No other dependencies. Seriously.

### Build & Run

```bash
cd src
make          # Builds librethink.a static library
make test     # Builds and links the integration test
./test_brain  # Runs "A Day in the Life of the Rethink Brain"
make clean    # Removes build artifacts
```

### What You'll See

```
=== RETHINK BRAIN — A Day in the Life ===

--- Learning phase ---
Learned 3 categories: cat, dog, bird

--- Experience phase ---
Saw 'fluffy_thing': surprise=0.754, understanding=0.570
Saw 'barking_thing': surprise=0.313, understanding=0.761

--- Classification ---
fluffy_thing -> 'cat' (confidence=1.00)
barking_thing -> 'dog' (confidence=1.00)

--- Causal reasoning ---
Why slipping? 'slipping' because: wet_ground(60%) <- rain(90%)
What if rain? 'rain' will cause: wet_ground(100%), slipping(97%), umbrella(99%)

--- Communication ---
Brain says: "cat dog"

--- Sleep cycle ---
Slept for 10 cycles
Dream insight! novelty=0.81 stability=0.50
```

---

## Project Structure

```
rethink-ai/
├── Guides/                     # 56 brain-science-first learning guides
│   ├── 00_getting_started/     # Philosophy + C setup
│   ├── 01_the_neuron/          # Spiking neurons
│   ├── 02_learning_without_backprop/
│   ├── 03_seeing_the_world/    # Perception
│   ├── 04_memory/              # Hippocampus + forgetting
│   ├── 05_learning_like_a_child/  # Few-shot learning
│   ├── 06_thinking_without_input/ # Default Mode + dreams
│   ├── 07_the_modular_brain/   # Regions + emotion
│   ├── 08_understanding/       # Prediction + causality
│   ├── 09_communication/       # Grounded language
│   └── 10_the_rethink_brain/   # Full integration
├── Learnings/                  # Personal insight journals (per phase)
├── Mistakes/                   # Error logs (per phase)
├── src/                        # All C source code
│   ├── neurons/                # neuron.c, network.c, synapse.h
│   ├── learning/               # hebbian.c, stdp.c, prototype.c
│   ├── perception/             # retina.c, som.c
│   ├── memory/                 # hopfield.c, decay.c
│   ├── thinking/               # spontaneous.c, dream.c
│   ├── modular/                # brain.c, emotion.c
│   ├── understanding/          # predictor.c, causal.c
│   ├── communication/          # comm.c
│   ├── rethink_brain.c/.h      # Master integration
│   ├── test_brain.c            # Integration test
│   └── Makefile
├── archive/                    # Version history + changelog
├── STATUS.md                   # Current project status + roadmap
└── README.md                   # This file
```

---

## Key Design Principles

1. **Brain science first.** Every module starts with neuroscience, not algorithms.
2. **No backpropagation.** All learning is biologically plausible (Hebbian + STDP).
3. **No Python.** Raw C11 for speed, transparency, and control.
4. **No external libraries.** Only `stdlib.h`, `math.h`, `stdio.h`, `string.h`.
5. **Memory is natural.** Memories fade, strengthen with use, consolidate during sleep.
6. **Few-shot learning.** Categories learned from 1-3 examples, not millions.
7. **Thinking is spontaneous.** The brain has internal activity even without input.
8. **Emotion is computational.** Fear boosts attention. Reward drives learning.
9. **Language is grounded.** Words mean something because they're tied to experience.
10. **Never delete, always version.** Every iteration (V1-V10+) is preserved.

---

## Roadmap

**Completed:** V1-V10 (Phases 0-10)

| Version | What's Next |
|---------|------------|
| V11 | Sensory expansion — auditory, tactile, multi-modal binding |
| V12 | Real attention mechanisms — spotlight, feature-based, temporal |
| V13 | Motor system — action planning, sequences, embodied cognition |
| V14 | Social brain — Theory of Mind, imitation, trust |
| V15 | Metacognition — confidence monitoring, learning-to-learn, self-model |

Long-term: consciousness, creativity, developmental learning, neuroplasticity.

See [STATUS.md](STATUS.md) for the detailed task checklist.

---

## What This Proves

| Claim | Evidence |
|-------|---------|
| You don't need Python/PyTorch | Entire system is raw C11 |
| You don't need backpropagation | Hebbian + STDP learn effectively |
| You don't need millions of examples | Prototype learner works from 1 example |
| You don't need transformers | Spiking neurons + predictive coding work |
| AI can understand, not just predict | Causal reasoning + grounded symbols |
| Memory can be natural | Decay, consolidation, reconstruction |
| Thinking can be spontaneous | Default Mode Network, dream engine |
| Emotion is computational | Amygdala modulates attention + learning |

---

## Philosophy

> "I see AI as a part of our brain and I want it to process the input as our brain does physically instead of some stupid algorithms. It should understand instead of just prediction."

This project is an experiment in building AI from first principles — not the first principles of linear algebra, but the first principles of neuroscience. It's intentionally incomplete, intentionally experimental, and intentionally different from everything else out there.

If a direction seems crazy, try it. That's the point.

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

*Built with neurons, not matrices.*
