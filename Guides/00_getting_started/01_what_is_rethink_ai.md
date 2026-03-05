# What is Rethink AI?

## The Problem with Current AI

Today's AI (ChatGPT, image generators, self-driving cars) is built on one core idea:

> **"Give the machine a million examples, and it will find patterns."**

That's it. That's fundamentally all modern AI does — it finds patterns in data using math (specifically, matrix multiplication + an algorithm called backpropagation). It doesn't "understand" anything. It doesn't "think." It doesn't "know" what it knows.

When ChatGPT says "the sky is blue," it has ZERO idea what a sky is, what blue looks like, or what "is" means. It simply learned that the word "blue" appears near "sky" very often in its training data. It's a very sophisticated pattern-matching machine. Nothing more.

### What's Wrong With This?

1. **It needs MILLIONS of examples** — A child sees ONE dog and knows all dogs. GPT-4 needed billions of sentences.
2. **It doesn't understand, it predicts** — Ask it "why is the sky blue?" and it produces text that statistically resembles a correct answer. It has no model of light, atmosphere, or physics.
3. **It has no real memory** — Each conversation starts from zero. "Context windows" are just feeding text back in, not real memory.
4. **It can't think without input** — Close your eyes. You're still thinking. AI does NOTHING without a prompt.
5. **It's one giant blob** — Your brain has specialized regions (sight, hearing, language, planning). AI is just one big neural network doing everything.
6. **It forgets catastrophically** — Teach it something new and it forgets old things. Your brain gracefully fades unimportant memories while keeping critical ones.

---

## What Rethink AI IS

Rethink AI is a project to build artificial intelligence that works **like a physical brain**, not like a statistics engine.

### Core Philosophy

> **AI should UNDERSTAND, not just predict.**
> **AI should work like a brain — physically — not like an algorithm.**

This means:

1. **Real neurons** — Not `output = weights × input + bias`. Real spiking neurons that accumulate charge, fire when threshold is reached, and go silent during refractory periods. Just like the 86 billion neurons in your skull.

2. **Brain-like learning** — No backpropagation. The brain doesn't calculate error gradients backward through every neuron. Instead, it uses Hebbian learning: "neurons that fire together, wire together." The connection between two neurons gets stronger when they fire at the same time.

3. **Memory that lives and fades** — Not a tensor stored forever. Real memory: things you just heard (working memory), things you know for a while (short-term), things burned in forever (long-term). And most memories FADE. That's not a bug — that's how you survive without drowning in useless information.

4. **Few-shot learning** — See 3 cats, know ALL cats. By extracting the "essence" (prototype), not memorizing every pixel.

5. **Thinking without input** — The system has spontaneous activity. It "dreams" — replaying memories during idle time to strengthen important ones. It can "imagine" — simulate scenarios it hasn't experienced.

6. **Specialized modules** — Separate perception, memory, reasoning, and communication modules that talk to each other through a routing system (attention). Just like the visual cortex talks to the prefrontal cortex.

7. **Understanding through prediction** — The system predicts what will happen next. When reality doesn't match the prediction → surprise → learn. THIS is understanding — having a model of the world and updating it.

---

## What We're Building

Over 10 phases, we'll build a complete brain-inspired AI system in **C** (no libraries, no frameworks):

| Phase | What We Build | Brain Analogy |
|-------|--------------|---------------|
| 0 | Project setup | — |
| 1 | Spiking neurons & networks | Individual neurons and synapses |
| 2 | Hebbian & STDP learning | How connections strengthen/weaken |
| 3 | Visual perception system | Visual cortex (V1, V2) |
| 4 | Memory with decay | Hippocampus + memory consolidation |
| 5 | Few-shot category learning | How children learn concepts |
| 6 | Spontaneous activity & dreaming | Default mode network, sleep |
| 7 | Modular brain architecture | Specialized brain regions |
| 8 | Predictive coding & curiosity | Prediction error learning |
| 9 | Symbol grounding & communication | Language areas, Broca's/Wernicke's |
| 10 | The Rethink Brain (integrated) | The whole brain working together |

---

## Why C?

- **Speed** — Brain simulations need to be FAST. Thousands of neurons firing in real-time.
- **Control** — We own every byte of memory, every clock cycle. No garbage collector, no hidden magic.
- **No dependencies** — Just `math.h` and `stdlib.h`. Everything else, WE build.
- **Closeness to hardware** — The brain is physical. Our code should be as close to the metal as possible.

---

## How This Guide Works

Every phase follows the same pattern:

1. **🧠 Brain Science** — How does the real brain do this? (simple language, no jargon)
2. **💡 The Idea** — What's the key insight we're implementing?
3. **🔨 Build It** — Step-by-step C code with explanations
4. **🧪 Test It** — Run it, see it work, break it on purpose
5. **📝 Reflect** — What did you learn? What surprised you? (→ `Learnings/`)
6. **❌ Mistakes** — What went wrong? What did you misunderstand? (→ `Mistakes/`)

---

## One Rule

> **If it doesn't exist yet, try it anyway.**
>
> This is YOUR version of AI. You're not following a textbook. You're rethinking how intelligence works from first principles. Some ideas will fail. That's called progress.

---

*Next: [02_c_setup_and_tools.md](02_c_setup_and_tools.md) — Setting up your C development environment*
