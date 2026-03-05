# 🧠 Types of Memory — How Your Brain Stores the World

## You're Using All Three Right Now

As you read this sentence:

1. **Working Memory** — holds THIS sentence in your mind right now. Capacity: ~4-7 items. Duration: seconds. If I asked you to remember "7382" while reading, that's working memory. It's fragile — any distraction erases it.

2. **Short-Term Memory** — what you had for breakfast. The conversation you had an hour ago. Duration: minutes to hours. It's stronger than working memory but still temporary.

3. **Long-Term Memory** — your name. Your childhood home. How to ride a bike. Duration: years to forever. This is information that got **consolidated** — moved from temporary to permanent storage.

---

## How Memories Move Between Systems

```
                    ┌──────────────┐
                    │   SENSORY    │  Everything your senses pick up
                    │   INPUT      │  (99.9% ignored immediately)
                    └──────┬───────┘
                           │
                           ▼ (attention selects what matters)
                    ┌──────────────┐
                    │   WORKING    │  What you're actively thinking about
                    │   MEMORY     │  (4-7 items, seconds)
                    └──────┬───────┘
                           │
                           ▼ (rehearsal / importance)
                    ┌──────────────┐
                    │  SHORT-TERM  │  Recent events, facts
                    │   MEMORY     │  (minutes to hours)
                    └──────┬───────┘
                           │
                           ▼ (consolidation during sleep!)
                    ┌──────────────┐
                    │  LONG-TERM   │  Skills, facts, experiences
                    │   MEMORY     │  (years to lifetime)
                    └──────────────┘
```

### The Hippocampus — The Memory Switchboard

Deep in your brain sits the **hippocampus** (shaped like a seahorse). It's the KEY to memory formation:

- New experiences go to the hippocampus FIRST
- The hippocampus acts as a temporary storage and index
- During sleep, the hippocampus "replays" important memories and transfers them to long-term storage in the cortex
- Damage the hippocampus → you can't form NEW long-term memories (but old ones survive)

---

## Forgetting — The Most Important Feature

Here's the insight you had: **"We remember everything but fade out most things, and still manage to survive."**

This is PROFOUND. Forgetting is NOT a failure of memory. It's a **feature**:

### Why Forgetting Is Essential

1. **Noise reduction** — If you remembered every face on every street, you couldn't find your friend in a crowd. Forgetting irrelevant faces makes important ones stand out.

2. **Generalization** — If you remembered every specific dog you ever saw in perfect detail, you'd struggle to recognize a NEW dog breed. Forgetting the details and keeping the essence ("four legs, fur, tail, barks") is what lets you generalize.

3. **Efficiency** — Your brain uses ~20 watts of power. Storing everything forever would require way more. Forgetting is energy-efficient.

4. **Emotional health** — Weakening painful memories over time is how you heal from trauma. (When this system fails → PTSD)

5. **Relevance** — What matters NOW is more important than what happened years ago. Memory decay ensures recent info is more accessible.

### How Forgetting Works

Memories don't "delete" — they **fade**:
- The neural connections that encode the memory gradually WEAKEN
- If the memory is recalled (rehearsed), the connections STRENGTHEN again
- Memories that are never recalled slowly fade to zero
- Emotional memories decay SLOWER (adrenaline strengthens consolidation)
- Sleep sorts "keep" from "discard"

This is EXACTLY what we'll build.

---

## What We'll Implement

| Memory Type | Implementation | Brain Analog |
|------------|----------------|--------------|
| **Associative memory** | Hopfield Network | Hippocampal pattern completion |
| **Memory decay** | Weight decay over time | Synaptic weakening |
| **Consolidation** | Replay + strengthen important memories | Sleep/dreaming |
| **Working memory** | Active firing buffer | Prefrontal sustained activity |
| **Partial recall** | Noisy input → clean output | Pattern completion |

---

*Next: [02_brain_hippocampus.md](02_brain_hippocampus.md) — The hippocampus and memory consolidation*
