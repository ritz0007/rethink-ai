# Contributing to Rethink AI

This is a personal research project — a brain-inspired AI system built from scratch in C. Contributions that align with the project's philosophy are welcome.

## Core Philosophy

Before contributing, understand what this project is and isn't:

- **Brain science first.** Every feature must be grounded in real neuroscience. No "it works because the math says so" — explain *which brain region* does this and *how*.
- **No backpropagation.** All learning must be biologically plausible.
- **No external libraries.** Only C standard library (`stdlib.h`, `math.h`, `stdio.h`, `string.h`).
- **Raw C (C11).** No C++, no Python wrappers.
- **Never delete, always version.** Old approaches go to `archive/`, not the trash.

## How to Contribute

### 1. Report Issues
- Describe what you expected vs what happened
- Include the output of `make test && ./test_brain`
- Mention your GCC version and OS

### 2. Suggest Brain Modules
If you know a brain mechanism that should be implemented:
- Open an issue with the brain science (cite a paper or textbook)
- Explain what the module would do
- Suggest which existing modules it would connect to

### 3. Submit Code
1. Fork the repository
2. Create a feature branch (`git checkout -b v11-auditory`)
3. Follow the existing code style:
   - Module = one `.h` + one `.c` in the appropriate `src/` subdirectory
   - All structs and functions prefixed with the module name (e.g., `hopfield_create`, `hopfield_destroy`)
   - `_create()` / `_destroy()` lifecycle for heap-allocated modules
   - No global state
4. Add a guide file in `Guides/` explaining the brain science
5. Update `rethink_brain.h` and `rethink_brain.c` if the module integrates
6. Update the Makefile
7. Make sure `make` compiles with zero errors and `make test` passes
8. Submit a pull request

### 4. Write Guides
Guides follow this pattern:
1. **Brain science** — How does the real brain do this?
2. **Why it matters** — What problem does this solve?
3. **Build** — The C implementation with inline comments
4. **Milestone** — Verification checklist

## Code Style

```c
/* Module-prefixed names */
HopfieldNet *hopfield_create(int num_neurons);
void hopfield_destroy(HopfieldNet *net);
void hopfield_store(HopfieldNet *net, const float *pattern);

/* Constants as #defines in headers */
#define HOPFIELD_MAX_NEURONS 256

/* Lifecycle: create → use → destroy */
/* No global mutable state */
/* Comments explain the brain science, not just the code */
```

## Compiler Flags

All code must compile cleanly with:
```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -lm
```

## Questions?

Open an issue. There are no stupid questions — only unexplored brain regions.
