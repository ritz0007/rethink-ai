# Phase 11 — Sensory Expansion: Mistakes & Pitfalls

## Mistakes I Made (or Almost Made)

### Design Mistakes
- [ ] Initially considered making each modality output different feature dimensions — broke plug-compatibility  
  **Fix:** Standardized all modalities to 32-dim output matching RETHINK_FEATURE_DIM

- [ ] Forgot that pain should never adapt — first draft had uniform adaptation for all channels  
  **Fix:** Added explicit `if (ch == TOUCH_PAIN)` override in tactile_adapt()

- [ ] Tried to do spatial binding (same location in space) across modalities but had no spatial encoding  
  **Fix:** Used cross-modal correlation as a proxy; true spatial binding deferred to future phase

### Implementation Pitfalls
- [ ] DFT in auditory_analyze_spectrum() is O(N×B) — not efficient for real-time  
  **Acceptable:** For our purposes (learning, not production) this is fine. Real brain uses basilar membrane resonance, not DFT.

- [ ] Multi-modal binding with only 1 active modality was returning empty percept  
  **Fix:** Added pass-through case: if only 1 modality active, copy features directly

- [ ] Ring buffer index wrapping: `(idx - 1 + LEN) % LEN` — easy to get the `+ LEN` wrong  
  **Lesson:** Always add buffer length before modulo to avoid negative indices

- [ ] Precision weights could drift to 0 if a modality keeps failing  
  **Fix:** Clamped precision to [0.1, 2.0] range in multimodal_update_precision()

### Integration Pitfalls
- [ ] Adding new modules to rethink_brain.h without updating the experience pipeline = dead code  
  **Fix:** Always wire init → process → extract → use in the same session

- [ ] New .c files in Makefile but forgot dependency rules  
  **Fix:** Added explicit `.o: .c .h` rules for all 3 new modules

## Patterns to Watch
- When adding new sensory modalities in future (proprioception, vestibular), follow the same pattern:
  1. Create `module.h` with init/process/extract_features
  2. Create `module.c` implementing the pipeline  
  3. Add to `multimodal.h` modality enum
  4. Wire into `rethink_brain.h/.c`
  5. Add to Makefile SRCS + dependencies
  6. Test build before anything else
