# C Setup and Tools

## What You Need

We're writing raw C. No fancy frameworks, no package managers. Just a compiler, a debugger, and a text editor.

---

## 1. Install the C Compiler (GCC)

GCC (GNU Compiler Collection) turns your `.c` files into programs your computer can run.

### On Ubuntu/Debian Linux:
```bash
sudo apt update
sudo apt install build-essential gdb
```

This installs:
- `gcc` — the C compiler
- `make` — build automation tool
- `gdb` — debugger (helps you find bugs step by step)

### On macOS:
```bash
xcode-select --install
```
This gives you `clang` (Apple's C compiler, works the same as GCC for our purposes).

### Verify it works:
```bash
gcc --version
make --version
gdb --version   # or lldb --version on macOS
```

---

## 2. Text Editor / IDE

Use whatever you're comfortable with. Recommended:

- **VS Code** with C/C++ extension — good balance of simplicity and features
- **Vim/Neovim** — if you're a terminal person
- **CLion** — full IDE, heavy but powerful

### VS Code Setup:
1. Install VS Code
2. Install the "C/C++" extension by Microsoft
3. Install the "Code Runner" extension (optional, for quick runs)

---

## 3. Project Structure

Create the project directory:

```bash
cd ~/Desktop/"My Own Understanding of AI"/rethink-ai
mkdir -p src/core src/learning src/perception src/memory src/cognition src/modules src/comm
```

Your `src/` folder will look like this:
```
src/
├── Makefile              # Build instructions
├── main.c                # Entry point — runs the rethink brain
├── core/                 # Neurons, synapses, networks, math
│   ├── neuron.h / .c
│   ├── synapse.h / .c
│   ├── network.h / .c
│   └── math_utils.h / .c
├── learning/             # How the brain learns
│   ├── hebbian.h / .c
│   └── stdp.h / .c
├── perception/           # How it sees the world
│   ├── retina.h / .c
│   └── som.h / .c
├── memory/               # How it remembers (and forgets)
│   ├── hopfield.h / .c
│   ├── working_mem.h / .c
│   └── decay.h / .c
├── cognition/            # How it thinks
│   ├── prototype.h / .c
│   ├── prediction.h / .c
│   ├── curiosity.h / .c
│   └── dreaming.h / .c
├── modules/              # Modular brain architecture
│   ├── router.h / .c
│   └── brain.h / .c
└── comm/                 # Communication system
    ├── symbols.h / .c
    └── language.h / .c
```

---

## 4. Your First C Program (Sanity Check)

Create a file `src/main.c`:

```c
#include <stdio.h>

int main(void) {
    printf("=== RETHINK AI ===\n");
    printf("Building a brain, not an algorithm.\n");
    printf("Version: V1\n");
    printf("==================\n");
    return 0;
}
```

Compile and run:
```bash
cd src
gcc -o rethink_brain main.c -lm -Wall -Wextra -std=c11 -O2
./rethink_brain
```

You should see:
```
=== RETHINK AI ===
Building a brain, not an algorithm.
Version: V1
==================
```

### What those flags mean:
- `-o rethink_brain` — name the output program "rethink_brain"
- `-lm` — link the math library (we'll need `sin`, `exp`, `sqrt` etc.)
- `-Wall -Wextra` — show ALL warnings (catches bugs early)
- `-std=c11` — use the C11 standard
- `-O2` — optimize for speed

---

## 5. Quick C Refresher

If you need a refresher on C, here's what matters most for this project:

### Structs (our building blocks)
```c
// A neuron is a struct — a bundle of data
typedef struct {
    float voltage;      // Current charge level
    float threshold;    // Fires when voltage exceeds this
    float leak_rate;    // How fast charge leaks away
    int is_firing;      // 1 if currently firing, 0 if not
} Neuron;
```

### Pointers (connections between things)
```c
// A synapse connects two neurons
typedef struct {
    Neuron *source;     // Points to the source neuron
    Neuron *target;     // Points to the target neuron
    float weight;       // Strength of connection
} Synapse;
```

### Dynamic Memory (creating neurons at runtime)
```c
// Create a network of N neurons
Neuron *neurons = malloc(N * sizeof(Neuron));
// ... use them ...
free(neurons);  // Always free what you malloc
```

### Header Files (organizing code)
```c
// neuron.h — declares WHAT exists
#ifndef NEURON_H
#define NEURON_H

typedef struct { ... } Neuron;
Neuron neuron_create(float threshold, float leak_rate);
void neuron_step(Neuron *n, float input_current, float dt);

#endif

// neuron.c — defines HOW it works
#include "neuron.h"

Neuron neuron_create(float threshold, float leak_rate) {
    Neuron n = {0};
    n.threshold = threshold;
    n.leak_rate = leak_rate;
    return n;
}
```

---

## 6. Debugging with GDB

When things go wrong (they will), GDB is your friend:

```bash
# Compile with debug info
gcc -o rethink_brain main.c -lm -Wall -Wextra -std=c11 -g

# Run with GDB
gdb ./rethink_brain

# Inside GDB:
(gdb) break main        # Stop at main()
(gdb) run               # Start the program
(gdb) next              # Execute next line
(gdb) print voltage     # Print a variable
(gdb) continue          # Resume running
(gdb) quit              # Exit GDB
```

---

## 7. Version Control (Git)

Track every change:

```bash
cd ~/Desktop/"My Own Understanding of AI"/rethink-ai
git init
git add .
git commit -m "V1: Project structure created - rethink AI begins"
```

After each phase milestone:
```bash
git add .
git commit -m "V2: Hebbian learning working - neurons wire together"
git tag V2
```

---

*Next: [03_project_structure.md](03_project_structure.md) — How the code is organized and why*
