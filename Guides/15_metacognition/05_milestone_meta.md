# V15 Milestone — Metacognition Verification

> Rethink AI — Phase 15, Guide 5 of 5

---

## Build Verification

### Compilation
- [ ] `meta.h` compiles without errors
- [ ] `meta.c` compiles without errors
- [ ] `make` produces 24 object files → `librethink.a`
- [ ] `make test` builds and links `test_brain`
- [ ] `make chat` builds and links `chatbot`
- [ ] Only pre-existing harmless warnings

---

## Chatbot Commands

### Confidence
- [ ] `confident` → shows current confidence (consistency, prediction, memory sources)
- [ ] After several correct classifications, confidence rises
- [ ] After errors, confidence drops

### Error Monitoring
- [ ] `errors` → shows error monitor state (baseline, streak, cognitive load)
- [ ] Repeated errors trigger error detection alert

### Strategy Selection
- [ ] `strategies` → lists all strategies with success rate, effort, value
- [ ] `select_strategy` → selects strategy based on current metacognitive state
- [ ] After errors, strategy switches more readily (higher temperature)

### Self-Model
- [ ] `self` → shows self-model (perception, memory, reasoning, etc.)
- [ ] After performing well in a domain, that skill estimate rises
- [ ] After performing poorly, skill estimate drops

### Introspection
- [ ] `introspect` → runs full metacognitive loop
- [ ] Reports confidence, error status, active strategy, learning rate, trajectory

### Meta-Learning
- [ ] `meta_lr` → shows effective learning rate
- [ ] When surprised + uncertain: learning rate is high
- [ ] When confident + unsurprised: learning rate is low

---

## Expected Output Example

```
brain> introspect
  === Metacognitive Introspection (#1) ===
  Confidence:     0.50 (consistency=0.50, prediction=0.50, memory=0.50)
  Error monitor:  baseline=0.000, streak=0, load=0.000
  Strategy:       'memory_lookup' (success=0.50, value=50.00)
  Learning rate:  0.100 (base=0.10 × surprise=1.00 × uncertainty=1.00)
  Trajectory:     0.000 [flat — no data yet]
  Stamina:        0.99
  Curiosity:      0.50

brain> teach cat
brain> show fluffy_thing
brain> what
  fluffy_thing -> 'cat' (confidence=1.00)
brain> introspect
  === Metacognitive Introspection (#2) ===
  Confidence:     0.62 (consistency=0.70, prediction=0.60, memory=0.55)
  ...
  Strategy:       'pattern_match' (success=0.65, value=65.00)
  Learning rate:  0.085 (base=0.10 × surprise=0.85 × uncertainty=0.80)
  Trajectory:     +0.05 [improving]
```

---

## Integration Tests

### With Predictive Coding (V8)
- [ ] Prediction errors feed into meta_record_error()
- [ ] Surprise feeds into meta_adapt_learning()

### With Attention (V12)
- [ ] Error detection boosts attention level

### With Motor (V13)
- [ ] Motor confidence feeds into self-model motor_skill
- [ ] Motor RPE feeds into error monitor

### With Social (V14)
- [ ] Social prediction accuracy feeds into self-model social_skill

---

## What This Proves

V15 demonstrates that our brain can:
1. **Monitor its own confidence** — calibrated second-order judgment
2. **Detect its own errors** — ACC-style error/conflict monitoring
3. **Select cognitive strategies** — value-based softmax with metacognitive temperature
4. **Adapt its learning rate** — surprise × uncertainty → meta-learning
5. **Model its own capabilities** — self-model with per-domain skill tracking
6. **Improve through introspection** — the recursive loop that makes intelligence self-correcting

This is the capstone: a brain that not only thinks, but thinks about its own thinking. The loop is closed.

---

*This phase transforms the brain from a pipeline into a self-improving system.*
