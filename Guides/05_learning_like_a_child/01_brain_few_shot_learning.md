# 🧒 Brain Science: Few-Shot Learning — Learning from Almost Nothing

## The Most Impressive Thing Brains Do

A child sees **one** dog. Just one. And then correctly identifies every other dog they ever see — big ones, small ones, fluffy ones, hairless ones. From a single example.

Current AI needs **thousands** of labeled examples of dogs. That's not intelligence — that's memorization by brute force.

## How Does the Brain Do It?

### 1. Prototype Extraction

When you see your first dog, your brain doesn't store a pixel-perfect image. It extracts what makes a dog a DOG:

- Four legs
- Tail
- Snout
- Ears
- Moves independently
- Makes sounds

This becomes a **prototype** — a compressed representation of "dog-ness."

### 2. Feature Attention

Not all features are equal. The brain rapidly figures out which features **distinguish** this category from others:

```
Dog vs Cat:
  Shared:    four legs, tail, fur, ears        ← not useful for distinction
  Different: snout shape, behavior, sound      ← THIS is what matters
```

The brain **pays attention to distinguishing features** more than shared ones.

### 3. Comparison, Not Classification

Standard AI: "Here are 10,000 dogs, now learn a decision boundary."

Human brain: "Is this new thing **more similar** to my dog prototype or my cat prototype?"

That's it. Similarity comparison. No decision boundaries. No optimization.

### 4. One-Shot Anchoring

The first example becomes an **anchor point**. Each new example **adjusts** the prototype slightly:

```
Seen 1 dog (Golden Retriever):
  Prototype = {golden fur, big, floppy ears, friendly}

Seen 2nd dog (Chihuahua):
  Prototype adjusts = {has fur (any color), size varies, has ears, ...}

Seen 3rd dog (Poodle):
  Prototype adjusts = {fur type varies, size varies, ...}
```

The prototype gets **more abstract** with each example. But even with just 1 example, it works because...

### 5. Transfer from Prior Knowledge

A child who has seen cats, horses, and birds already knows:
- Animals have legs
- Animals have eyes
- Animals move

So "dog" only needs to encode **what's different from what they already know**.

```
New concept = delta from nearest known concept
```

This is incredibly efficient. You don't store "dog" from scratch — you store "like a cat but bigger, different face, different sound."

## What This Means for Rethink AI

We need to build:

| Component | Brain Does | Current AI Does |
|-----------|-----------|-----------------|
| **Prototype formation** | Extract key features from 1-3 examples | Need thousands of examples |
| **Feature attention** | Focus on distinguishing features | Treat all features equally |
| **Similarity matching** | Compare to prototypes | Learn decision boundaries |
| **Incremental update** | Adjust prototype with each new example | Retrain entire model |
| **Transfer** | Build on what's already known | Start from scratch each time |

## The Math Behind Prototypes

### Simple prototype:
$$\text{prototype} = \frac{1}{k} \sum_{i=1}^{k} \text{example}_i$$

Average of all examples seen so far. Even with k=1, it works.

### Online update (no need to re-average):
$$\text{prototype}_{new} = \text{prototype}_{old} + \frac{1}{k+1}(\text{new\_example} - \text{prototype}_{old})$$

### Similarity:
$$\text{similarity}(x, p) = \frac{x \cdot p}{|x| \cdot |p|}$$

Cosine similarity — how "aligned" is this input with the prototype?

## Why This is Revolutionary

If we can build a system that:
1. Forms prototypes from 1-3 examples
2. Knows which features matter
3. Classifies by similarity comparison
4. Builds on prior knowledge

...we'll have something that learns like a child, not like a machine.

---

*Next: [Prototype Theory — How the Brain Categorizes](02_brain_prototype_theory.md)*
