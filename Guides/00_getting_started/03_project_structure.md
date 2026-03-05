# Project Structure — How the Code is Organized

## The Big Picture

The Rethink AI code is organized to mirror the brain itself. Each folder = a brain system:

```
src/
├── Makefile
├── main.c                 ← The "consciousness" — ties everything together
│
├── core/                  ← The biology: neurons, synapses, basic wiring
│   ├── neuron.h / .c        Individual brain cells
│   ├── synapse.h / .c       Connections between brain cells
│   ├── network.h / .c       Groups of connected neurons
│   └── math_utils.h / .c    Math helpers (random numbers, matrix ops)
│
├── learning/              ← How the brain learns (NO backpropagation)
│   ├── hebbian.h / .c       "Fire together, wire together"
│   └── stdp.h / .c          Timing-based learning
│
├── perception/            ← How the brain sees the world
│   ├── retina.h / .c        Turns pixels into neural signals
│   └── som.h / .c           Self-organizing maps (pattern discovery)
│
├── memory/                ← How the brain remembers (and forgets)
│   ├── hopfield.h / .c      Associative memory (recall from partial input)
│   ├── working_mem.h / .c   Short-term "thinking" memory
│   └── decay.h / .c         Memory fading over time
│
├── cognition/             ← How the brain thinks
│   ├── prototype.h / .c     Few-shot learning (learn from few examples)
│   ├── prediction.h / .c    Predict what happens next
│   ├── curiosity.h / .c     Seek out surprising new info
│   └── dreaming.h / .c      Replay memories, think without input
│
├── modules/               ← The architecture: how brain regions connect
│   ├── router.h / .c        Routes info to the right brain region
│   └── brain.h / .c         The whole brain — all modules connected
│
└── comm/                  ← How it communicates
    ├── symbols.h / .c       Connects internal patterns to labels
    └── language.h / .c      Multi-agent communication
```

---

## How Brain Parts Map to Code

| Brain Region | Code Module | What It Does |
|-------------|-------------|-------------|
| Individual neurons | `core/neuron.c` | Accumulate charge, fire spikes, leak |
| Synapses (connections) | `core/synapse.c` | Carry signals between neurons with delay and strength |
| Neural circuits | `core/network.c` | Groups of neurons wired together |
| Synaptic plasticity | `learning/hebbian.c` | Strengthen connections that co-activate |
| Synaptic timing rules | `learning/stdp.c` | Strengthen if source fires BEFORE target |
| Visual cortex (V1/V2) | `perception/retina.c` | Detect edges, shapes from raw pixels |
| Cortical maps | `perception/som.c` | Self-organize to represent patterns |
| Hippocampus | `memory/hopfield.c` | Store and recall patterns |
| Prefrontal cortex WM | `memory/working_mem.c` | Hold current "thoughts" |
| Memory consolidation | `memory/decay.c` | Fade unimportant, strengthen important |
| Concept formation | `cognition/prototype.c` | Build prototypes from few examples |
| Predictive processing | `cognition/prediction.c` | Predict next input, learn from surprise |
| Curiosity drive | `cognition/curiosity.c` | Seek out novel, surprising information |
| Default mode network | `cognition/dreaming.c` | Fire spontaneously, replay memories |
| Thalamus (relay) | `modules/router.c` | Route information between modules |
| Whole brain | `modules/brain.c` | All systems integrated |
| Broca's area | `comm/symbols.c` | Ground symbols in experience |
| Language centers | `comm/language.c` | Communicate between agents |

---

## Build System

The `Makefile` handles compiling everything:

```
make              → Build the whole project
make test         → Run tests
make clean        → Remove compiled files
make phase1       → Build just Phase 1 (neurons)
make phase2       → Build just Phase 2 (learning)
... etc
```

Each phase adds new source files. Older phases keep working.

---

## File Naming Convention

- **`.h` files** = declarations (WHAT exists) — like a table of contents
- **`.c` files** = implementations (HOW it works) — the actual logic
- **Every `.c` file includes its own `.h`** — keeps things organized
- **`main.c`** includes what it needs from each module

---

## How to Add New Code

When you build something new:

1. Create `new_thing.h` with the struct and function declarations
2. Create `new_thing.c` with the implementations
3. Add the `.c` file to the Makefile
4. Include the `.h` in whatever needs it
5. Rebuild with `make`

Example:
```c
// core/neuron.h
#ifndef NEURON_H
#define NEURON_H

typedef struct { /* ... */ } Neuron;
Neuron neuron_create(float threshold, float leak_rate);

#endif

// core/neuron.c
#include "neuron.h"
Neuron neuron_create(float threshold, float leak_rate) { /* ... */ }

// main.c
#include "core/neuron.h"
int main() {
    Neuron n = neuron_create(1.0f, 0.1f);
    // ...
}
```

---

## Version Tagging

After each phase milestone:
```bash
git add .
git commit -m "VX: [milestone description]"
git tag VX
```

The changelog at `archive/changelog.md` tracks WHY each version exists.

---

*You're ready. Let's build neurons. → [Phase 1: The Neuron](../01_the_neuron/01_brain_how_a_neuron_works.md)*
