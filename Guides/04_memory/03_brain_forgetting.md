# 🧠 Forgetting Is a Feature, Not a Bug

## The Ebbinghaus Forgetting Curve

In 1885, Hermann Ebbinghaus memorized lists of nonsense syllables and tested how fast he forgot them. The result:

```
Memory
Strength
  100% |*
       | *
   80% |  *
       |   *
   60% |    *
       |      *
   40% |         *
       |             *
   20% |                    *
       |                              *
    0% +──────────────────────────────────→ Time
       0    1h    1d    2d    1w    1m
```

You forget FAST at first, then the curve flattens. After a month, what remains is fairly stable.

### But Here's the Key: Each Recall Resets the Curve

```
Memory
Strength
  100% |*                 *              *
       | *               * *            * *
   80% |  *             *   *          *   *
       |   *           *     *        *     *
   60% |    *         *       *      *       *
       |      *      *         *    *         *
   40% |         *  *           *  *
       |           *             *
   20% |
    0% +──────────────────────────────────→ Time
       Store   Recall    Recall     Recall
               #1        #2         #3
```

Every time you recall a memory, the strength jumps back up AND the decay becomes slower. This is **spaced repetition** — the most effective learning technique known.

---

## What We'll Model

### Decay Function

Each memory has a **strength** value. It decays over time:

```c
strength = initial_strength × exp(-time / decay_constant)
```

- `initial_strength` = how strong the memory was when formed (affected by emotion, attention)
- `decay_constant` = how fast it fades (increases with each recall)
- When `strength < threshold` → memory is effectively forgotten

### Recall Strengthening

When a memory is recalled:
```c
strength = max_strength;          // Reset to full
decay_constant *= 1.5;           // Future decay is slower
recall_count++;                   // Track how many times recalled
```

### Importance Tagging

Not all memories are equal. Memories formed during high emotional/importance states decay SLOWER:
```c
if (importance > HIGH) {
    decay_constant *= 2.0;  // Important memories last longer
}
```

---

## Implementation Preview

Our memory system will have:
1. **Hopfield network** — stores patterns as attractors
2. **Strength tracking** — each stored pattern has a decay timer
3. **Recall strengthening** — recalled patterns get reinforced
4. **Capacity management** — when memory is full, weakest patterns get overwritten
5. **Dream/consolidation mode** — replay strong memories, let weak ones fade

---

*Next: [04_brain_associative_recall.md](04_brain_associative_recall.md) → [05_build_hopfield_memory.md](05_build_hopfield_memory.md)*
