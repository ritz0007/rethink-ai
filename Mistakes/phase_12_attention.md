# Phase 12 — Attention Mechanisms: Mistakes & Pitfalls

## Mistakes I Made (or Almost Made)

### Design Mistakes
- [ ] Initially considered a simple "spotlight" model (attend to one thing, ignore everything else)  
  **Fix:** Implemented biased competition — it's winner-take-MOST, losers still get some processing (suppression_gain=0.3)

- [ ] Forgot to handle the case when no items are submitted for competition  
  **Fix:** Early return in `attention_compete()` if `sys->num_items == 0`

- [ ] First draft had no temporal dynamics for goals — they would persist forever  
  **Fix:** Added TTL (time-to-live) to goals, decremented in `attention_modulate()`, auto-deactivated at 0

### Implementation Pitfalls
- [ ] Cosine similarity can return NaN if both vectors are zero  
  **Fix:** Guard clause: if `norm_a == 0 || norm_b == 0` return 0.0

- [ ] Budget depletion without recovery would eventually starve the system  
  **Fix:** Budget recovers by 0.01 per cycle, so a resting brain regains attention capacity

- [ ] Lateral inhibition with too high a strength makes ALL losers go to zero  
  **Fix:** Tuned `inhibition_strength=0.15` — strong enough to differentiate, weak enough to preserve signals

- [ ] Feature uniqueness in salience computation assumed items > 1; division by zero with single item  
  **Fix:** Only compute uniqueness when `sys->num_items > 1`, otherwise it's 0.0

### Integration Pitfalls
- [ ] The old `attention_level` was used in multiple places in the experience pipeline  
  **Fix:** Replaced the computation of `rb->attention_level` to use `attention_process()` output, kept the float as a summary

- [ ] Adding attention module but forgetting to call `attention_init()` in `rethink_create()`  
  **Fix:** Added init call immediately after adding the struct member

- [ ] New .c file in Makefile SRCS but initially missed adding the dependency rule  
  **Fix:** Added `modular/attention.o: modular/attention.c modular/attention.h` rule

## Patterns to Watch
- When extending attention in future phases:
  1. The priority map weights (salience_weight, relevance_weight, emotion_weight) are tunable — different tasks need different balances
  2. Budget depletion rate (0.01 per winner per cycle) may need tuning with more complex scenarios
  3. The capture_threshold (0.7) determines how easily the ventral network overrides focus — lower = more distractible
  4. MAX_ITEMS=16 and MAX_GOALS=4 are arbitrary limits — increase if needed for richer scenes
  5. Gain modulation currently affects features directly — could also modulate learning rates in future
