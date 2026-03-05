# Phase 15 — Metacognition: Learnings

> Rethink AI — V15

---

## What I Learned

### 1. Confidence Is Multi-Source, Not Monolithic
Real confidence comes from three independent sources: consistency (are my recent answers stable?), prediction accuracy (have my predictions been right?), and memory strength (how strong are the relevant memories?). Combining them with weighted sums (0.40/0.35/0.25) produces nuanced confidence that can be high in one source and low in another. This matches the aPFC's convergent input from multiple cortical systems.

### 2. Calibration Requires Feedback Loops
A system that says "I'm 90% confident" but is only right 60% of the time is useless. Calibration correction — slowly adjusting confidence based on prediction-outcome mismatches — is essential. The 0.05 calibration rate means it takes ~20 observations to correct systematic bias, matching the gradual nature of human metacognitive calibration.

### 3. Error Streaks Matter More Than Single Errors
One error is noise. Two consecutive errors are a signal. Three are an alarm. The error streak counter (ACC model) captures this — a single large error resets the streak if the next observation is fine, but sustained errors trigger strategy reassessment. This is exactly how the real ACC works: sustained error-related negativity potentials trigger increased cognitive control.

### 4. Strategy Selection Is Reinforcement Learning at the Meta-Level
The brain doesn't just learn *content* (what is a cat?) — it learns *how to learn* (should I use memory lookup or causal reasoning?). Strategy selection uses the same softmax + value computation as action selection in the motor system (V13), confirming that the brain reuses circuit motifs across levels. dlPFC and basal ganglia share computational architecture.

### 5. Temperature Adapts to Metacognitive State
When confident → low temperature → exploit the best strategy. When errors detected → moderate temperature → explore alternatives. This automatic temperature control is the implementation of cognitive flexibility — the ability to switch strategies when needed without switching too often when things are fine.

### 6. Learning Rate Adaptation Is Multiplicative
Surprise × uncertainty = effective learning rate modifier. This means: confident + unsurprised → learn slowly (protect knowledge), uncertain + surprised → learn fast (absorb data). The product structure ensures both conditions must be present for extreme rates, preventing either factor from dominating alone.

### 7. Self-Model Starts at Maximum Uncertainty
Starting all self-model dimensions at 0.5 (pure uncertainty) is biologically accurate — a newborn has no idea whether it's good at perceiving, remembering, or reasoning. The slow convergence (α=0.1) ensures the self-model reflects accumulated experience rather than recent flukes. This is why children develop self-awareness gradually.

---

## Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| 8 strategy slots | Rich enough for diverse approaches, bounded for efficiency |
| 64-entry performance history | ~Last hour of cognitive activity at one event per tick |
| 16-entry error window | Recent errors only — older ones are obsolete |
| Stamina min at 0.1 | Brain never fully shuts down (even exhausted, 10% baseline) |
| 4 default strategies | memory_lookup, causal_reason, pattern_match, exploration — covers the main cognitive tools |
| Curiosity range [0.1, 1.0] | Always some curiosity (never stops exploring entirely) |

---

## Brain Regions → Code Mapping

| Brain Region | Code Component | Function |
|---|---|---|
| aPFC (BA10) | `ConfidenceMonitor` | Multi-source confidence with calibration |
| ACC | `ErrorMonitor` | Error streak, conflict, cognitive load |
| dlPFC | `CognitiveStrategy[]` + `meta_select_strategy` | Value-based strategy selection |
| vmPFC | `strategy.value` | Success/effort ratio for strategies |
| AIC | `SelfModel` | Internal self-awareness and capability model |
| Dopamine system | `meta_adapt_learning` | Adaptive learning rate from surprise × uncertainty |
