# Phase 13 — Motor System: Mistakes & Pitfalls

## Mistakes I Made (or Almost Made)

### Design Mistakes
- [ ] Initially tried to model individual muscles/joints — way too complex for this phase  
  **Fix:** Abstract to action feature vectors — same 32-dim pattern as perception modules

- [ ] First design had no forward model — actions were fire-and-forget  
  **Fix:** Added per-action forward_model[] array that predicts outcome from state

- [ ] Forgot that the basal ganglia's default state is INHIBITION, not activation  
  **Fix:** Designed GO/NOGO so that when both are 0.5 (neutral), net=0 → no strong action selection

### Implementation Pitfalls
- [ ] RPE update was symmetric initially — positive and negative RPE had same magnitude effect  
  **Fix:** Made it asymmetric: positive RPE affects GO×1.0 and NOGO×0.5, negative is reversed. This matches the D1/D2 receptor difference.

- [ ] Forward model could diverge to infinity with large features  
  **Fix:** Features are normalized by word_to_features(), and forward model weights stay bounded via small learning rate

- [ ] Exploration rate could go negative if confidence exceeded 1.0  
  **Fix:** Clamped confidence to [0,1] and exploration to minimum 0.05

- [ ] Motor execution without plan → crash  
  **Fix:** Early return guard: `if (plan->num_steps == 0) return;`

### Integration Pitfalls
- [ ] Adding motor to rethink_brain.h but forgetting to include motor.h  
  **Fix:** Added `#include "motor/motor.h"` in the include block

- [ ] motor.c references stdlib.h rand() which could be undefined  
  **Fix:** Always include `<stdlib.h>` explicitly in motor.c

- [ ] Chatbot motor commands reference brain->motor but motor wasn't in RethinkBrain struct  
  **Fix:** Added `MotorSystem motor;` to RethinkBrain typedef

## Patterns to Watch
- When extending the motor system:
  1. Variable-length sequences (not just 3 steps) — needs dynamic planning
  2. Sequence chunking (merge adjacent actions) — needs temporal adjacency detection
  3. Motor cortex population coding — needs multiple neurons voting on direction
  4. Mirror neurons (learn from observing) — needs link between perception and motor
  5. GO/NOGO weight range [0,2] may need tuning for complex action spaces
