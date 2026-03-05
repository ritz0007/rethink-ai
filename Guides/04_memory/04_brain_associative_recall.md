# 🧠 Associative Recall — How 3 Notes Bring Back a Whole Song

## Content-Addressable Memory

Normal computer memory works by ADDRESS: "give me what's stored at location #4507." You need to know WHERE something is.

Your brain works differently. It uses **content-addressable memory**: "give me the memory that contains these features." You provide CONTENT and get back the full memory.

- See half a face → recall the whole face
- Smell something → recall the entire scene where you smelled it
- Hear 3 notes → recall the entire melody
- Read "to be or not to" → "be, that is the question"

---

## How Pattern Completion Works in the Brain

The hippocampus stores memories as **patterns of neural activity**. When you encounter a partial cue:

1. The partial cue activates SOME of the neurons in the original pattern
2. Those neurons activate their neighbors through learned connections (Hebbian!)
3. The neighbors activate THEIR neighbors
4. The network quickly "fills in" the missing parts
5. The full original pattern is restored

It's like starting a chain reaction. You provide the spark (partial cue), and the network's connections do the rest.

---

## Hopfield Networks — The Implementation

A Hopfield network stores patterns in its connection weights:

### Storing a pattern:
```
Pattern: [+1, -1, +1, +1, -1]  (binary: active/inactive)

For every pair of neurons (i, j):
    weight[i][j] += pattern[i] × pattern[j]

This is PURE Hebbian learning:
    - Both active (+1 × +1 = +1): strengthen
    - Both inactive (-1 × -1 = +1): strengthen  
    - One active, one not (+1 × -1 = -1): weaken
```

### Recalling from partial input:
```
Corrupted input: [+1, ?, +1, ?, -1]  (30% missing)
Fill unknowns with random: [+1, +1, +1, -1, -1]

Iterate until stable:
    For each neuron i:
        sum = Σ weight[i][j] × state[j]   (for all j ≠ i)
        new_state[i] = (sum > 0) ? +1 : -1

After a few iterations:
    Output: [+1, -1, +1, +1, -1]  ← The original pattern!
```

The network "fell into" the nearest stored pattern, like a ball rolling into the nearest valley.

---

## Energy Landscape

Think of memory recall as a **landscape with valleys**:

```
Energy
  ^
  |    /\        /\        /\        /\
  |   /  \      /  \      /  \      /  \
  |  /    \    /    \    /    \    /    \
  | /      \  /      \  /      \  /      \
  |/  mem1  \/  mem2  \/  mem3  \/  mem4  \
  +──────────────────────────────────────────→ State space
         ↑         
    Partial input lands here → rolls into nearest valley (mem1)
```

Each stored memory is a valley. Partial/noisy input lands somewhere on the landscape and "rolls downhill" into the nearest valley = the recalled memory.

---

## Capacity

A Hopfield network with N neurons can reliably store about **N / (2 × ln(N))** patterns. Store more → memories interfere with each other → false recalls.

For 100 neurons: ~11 patterns
For 1000 neurons: ~72 patterns

This matches the brain! The hippocampus has limited capacity, which is why consolidation (moving to long-term cortical storage) is essential.

---

*Next: [05_build_hopfield_memory.md](05_build_hopfield_memory.md) — Build the associative memory system*
