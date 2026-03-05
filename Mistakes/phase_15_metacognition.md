# Phase 15 — Metacognition: Mistakes & Pitfalls

> Rethink AI — V15

---

## Mistakes Made

### 1. Error Threshold Needs a Minimum
**Problem**: Initially, error detection compared `error_magnitude > baseline × 1.5`. But when baseline was 0.0 (no errors yet), any error — even tiny ones — counted as "above threshold," creating false alarms.
**Fix**: Added minimum threshold of 0.1: `if (threshold < 0.1f) threshold = 0.1f`. This means the first few errors are treated as normal baseline-building rather than emergencies.

### 2. Calibration Adjustment Needs Bounds
**Problem**: Without clamping, the confidence adjustment could drift arbitrarily far from zero, making all confidence outputs pinned at 0 or 1 regardless of evidence.
**Fix**: Clamped adjustment to [-0.3, +0.3]. Even severely miscalibrated confidence can only shift by 0.3, ensuring the raw evidence sources still dominate.

### 3. Strategy Value Division by Zero
**Problem**: If a strategy has avg_effort = 0.0, the value computation `success_rate / avg_effort` produces infinity, dominating softmax selection.
**Fix**: Added epsilon: `avg_effort + 0.01f`. This bounds value at 100× success_rate maximum, which is high but finite.

### 4. Performance History Ring Buffer Indexing
**Problem**: When computing prediction accuracy from recent history during introspection, naive `history[i]` access didn't account for the ring buffer wrapping.
**Fix**: Used modular indexing from the current write position backwards: `(history_idx - window + META_HISTORY_SIZE) % META_HISTORY_SIZE`.

### 5. Softmax Numerical Stability
**Problem**: With large value differences between strategies, `exp(value / temperature)` overflows for high values or underflows for low values.
**Fix**: Subtracted max value before exponentiating: `exp((value - max_val) / temperature)`. Standard numerical stability trick, but easy to forget.

---

## Things to Watch

| Issue | Risk | Mitigation |
|-------|------|------------|
| Strategy slots full | 8 strategies max — can't add more | Could implement LRU eviction for unused strategies |
| Self-model convergence | Slow convergence (α=0.1) means early self-model is inaccurate | Acceptable — matches developmental trajectory |
| Metacognitive overhead | Introspection runs every call — could be expensive | Currently trivial (~50 floating point ops) |
| Calibration cold-start | No calibration data at initialization | Starts at zero adjustment — neutral until data arrives |
| Stamina doesn't recover | Only decays, never increases | Would need a "rest" or "sleep" function to restore |
| No strategy deletion | Can add strategies but not remove them | Low priority — 8 slots rarely exhausted |

---

## What I'd Do Differently Next Time

1. **Add stamina recovery**: Connect stamina to `rethink_sleep()` so dreaming restores metacognitive resources.
2. **Track per-strategy confidence**: Each strategy should track how confident the brain is when using it, enabling finer metacognitive discrimination.
3. **Integrate with attention**: Error detection should directly boost attention budget (ACC → dlPFC → attention gain), creating the complete vigilance circuit.
4. **Add second-order metacognition**: "How confident am I in my confidence estimate?" — this recursive loop is what distinguishes expert metacognition from novice.
