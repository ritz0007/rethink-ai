# Phase 13 — Motor System: Milestone Verification

> Checklist to confirm the motor system works correctly

---

## Core Functionality

- [ ] `motor_init()` — System initializes with exploration=0.3, learning_rate=0.1, confidence=0.0
- [ ] `motor_add_action()` — Can add actions with names, features, neutral GO/NOGO weights
- [ ] `motor_find_action()` — Finds actions by name, returns -1 for unknown
- [ ] `motor_evaluate_action()` — Returns a score combining GO-NOGO + context + prediction
- [ ] `motor_select_action()` — Epsilon-greedy: random with probability ε, best otherwise

## Action Selection (Basal Ganglia)

- [ ] GO vs NO-GO competition: net = go_weight - nogo_weight
- [ ] Context relevance via cosine similarity to current state
- [ ] Forward model prediction contributes to action value
- [ ] Combined score: 40% net + 30% relevance + 30% prediction

## Sequence Planning (SMA)

- [ ] `motor_plan_action()` — Creates 3-step plan: prepare → execute → complete
- [ ] Each step has expected_effect based on goal_reward
- [ ] Plan tracks current_step and completion status
- [ ] Unknown actions are automatically added to repertoire

## Execution (M1 + Cerebellum)

- [ ] `motor_execute()` — Runs all steps in sequence
- [ ] Forward model predicts outcome before each step
- [ ] Actual effect = 80% of expected + noise (simulates imprecision)
- [ ] Total actual_reward is sum of step actual_effects
- [ ] Execution recorded in history ring buffer
- [ ] execution_count incremented for the action

## Learning (Dopamine + Cerebellum)

- [ ] Reward Prediction Error: RPE = actual - predicted_average
- [ ] RPE > 0: GO strengthened, NO-GO weakened (approach)
- [ ] RPE < 0: GO weakened, NO-GO strengthened (avoid)
- [ ] GO/NOGO weights clamped to [0.0, 2.0]
- [ ] Forward model updated by: weight += lr × RPE × features
- [ ] Reward average updated by exponential moving average (α=0.1)
- [ ] Confidence grows as |RPE| shrinks
- [ ] Exploration rate decays as confidence grows (min 5%)

## Integration

- [ ] `MotorSystem motor` added to `RethinkBrain` struct
- [ ] `motor_init(&rb->motor)` called in `rethink_create()`
- [ ] Chatbot `do` command: plan → execute → learn
- [ ] Chatbot `plan` command: plan without executing
- [ ] Chatbot `actions` command: print repertoire
- [ ] Makefile updated with motor.c in SRCS
- [ ] Build compiles with zero errors

## Brain Science Accuracy

- [ ] GO/NOGO competition models basal ganglia direct/indirect pathways
- [ ] RPE models dopamine signal from SNc
- [ ] Forward model models cerebellar prediction
- [ ] Sequence planning models SMA function
- [ ] Exploration → exploitation transition models development
- [ ] Action repertoire models procedural memory
