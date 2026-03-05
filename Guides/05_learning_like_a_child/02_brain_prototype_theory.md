# 🧠 Brain Science: Prototype Theory — How the Brain Categorizes

## Eleanor Rosch's Prototype Theory (1973)

This isn't speculation — it's experimentally validated cognitive science.

The brain does NOT store categories as rigid rules like:
```
Bird = {has wings, has feathers, can fly, has beak}
```

Because then a penguin wouldn't be a bird. Neither would an ostrich.

Instead, the brain stores a **prototype** — the most typical example — and judges membership by **similarity to the prototype**.

## How It Works

### 1. The Prototype is the "Average"

```
Robin       → very birdy  (close to prototype)
Sparrow     → very birdy  (close to prototype)
Eagle       → birdy       (moderately close)
Penguin     → barely birdy (far from prototype, but still in category)
Bat         → NOT a bird   (similar in some ways, but closer to mammal prototype)
```

There's no hard boundary. It's a **gradient of similarity**.

### 2. Typicality Gradient

Experiments show people respond faster when asked "Is a robin a bird?" than "Is a penguin a bird?" This proves the brain uses distance-from-prototype.

```
                    PROTOTYPE (robin)
                         |
              ←— more typical ——→ less typical
              
sparrow---robin---eagle---chicken---penguin---bat(NOT)
  0.95     1.0    0.85     0.70      0.35     0.10
```

### 3. Family Resemblance

Category members don't share one defining feature. They share **overlapping** features:

```
Feature      Robin  Eagle  Penguin  Ostrich
───────────  ─────  ─────  ───────  ───────
Flies        ✓      ✓      ✗        ✗
Feathers     ✓      ✓      ✓        ✓
Small        ✓      ✗      ✗        ✗
Beak         ✓      ✓      ✓        ✓
Swims        ✗      ✗      ✓        ✗
```

No single feature defines "bird." But there's a **family resemblance** — each member shares SOME features with the prototype.

## Hierarchical Prototypes

The brain doesn't just have one level. It has a **hierarchy**:

```
        Living Thing
        /          \
    Animal        Plant
    /    \        /    \
  Bird   Mammal  Tree  Flower
  / \     / \
Robin Dog  Cat Horse
```

Each level has its own prototype. You can compare at any level:
- "Is it an animal?" → compare to animal prototype
- "Is it a bird?" → compare to bird prototype
- "Is it a robin?" → compare to robin prototype

The brain automatically picks the **most useful level** based on context.

## Contrastive Learning: What Makes Things Different

A critical complement to prototypes: the brain doesn't just learn what things ARE, it learns **what makes them different from each other**.

### The Contrast Signal

```
Dog prototype:  [legs=4, tail=1, fur=yes, loyal=yes, size=medium]
Cat prototype:  [legs=4, tail=1, fur=yes, loyal=no,  size=small]
                                          ^^^^^^^^    ^^^^^^^^^
                                          THESE features matter!
```

The brain amplifies features that **distinguish** categories and suppresses features that are shared. This is incredibly efficient:

- Shared features (legs, tail, fur) → low attention weight
- Distinguishing features (loyalty, size) → high attention weight

### This is NOT Backpropagation

This happens through:
1. **Lateral inhibition** — similar prototypes compete, forcing differentiation
2. **Hebbian learning** — distinguishing features get stronger connections
3. **Attention gating** — context determines which features to weigh

## Implications for Our System

### What We Need to Build

```
ProtoLearner:
  - prototypes[]       Array of prototype vectors
  - counts[]           How many examples per prototype
  - feature_weights[]  Which features matter (attention)
  
  store(pattern, category):
    if new category:
      create new prototype = pattern
    else:
      update prototype with online average
    update feature weights via contrastive analysis
    
  classify(pattern):
    for each prototype:
      similarity = weighted_cosine(pattern, prototype, feature_weights)
    return argmax(similarity)
```

### Key Design Decisions

1. **No training phase** — prototypes form immediately
2. **No fixed categories** — new categories emerge as needed
3. **Feature weighting is learned** — not all dimensions are equal
4. **Incremental** — each new example refines, never retrains
5. **Similarity-based** — no decision boundaries to learn

## The Gap from Current AI

| Property | Brain | Standard AI | Rethink AI |
|----------|-------|-------------|------------|
| Examples needed | 1-5 | 1000+ | 1-5 |
| New category | Instant | Retrain | Instant |
| Features | Weighted by relevance | All equal (or learned slowly) | Weighted by contrast |
| Classification | Similarity to prototype | Decision boundary | Similarity to prototype |
| Memory | Prototypes (compressed) | All training data | Prototypes (compressed) |

---

*Next: [Contrastive Learning — Learning What Makes Things Different](03_brain_contrastive_learning.md)*
