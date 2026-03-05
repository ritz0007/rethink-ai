# Phase 13 — Motor System: Learnings

## What I Learned

### Brain Science
- [ ] The basal ganglia works by DISINHIBITION — it removes inhibition rather than exciting directly
- [ ] GPi/SNr tonically inhibits thalamus by default — nothing happens unless something unlocks it
- [ ] Direct pathway (D1, GO) vs indirect pathway (D2, NO-GO) compete for every action
- [ ] Dopamine encodes Reward Prediction Error (RPE): δ = actual - predicted
- [ ] Three dopamine signals: burst (better than expected), baseline (as expected), dip (worse than expected)
- [ ] The cerebellum contains ~80% of all brain neurons — dedicated to motor precision and timing
- [ ] The cerebellum maintains a forward model: predicts sensory consequences of actions
- [ ] Motor sequences are chunked (SMA) — sub-steps merge into automatic sequences through practice
- [ ] Premotor cortex selects actions based on context — same stimulus can trigger different actions
- [ ] Exploration → exploitation transition models infant development

### Implementation
- [ ] GO/NOGO competition is just subtraction: net = go_weight - nogo_weight
- [ ] Action evaluation combines 3 signals: net strength (40%) + context relevance (30%) + prediction (30%)
- [ ] Epsilon-greedy selection balances exploration and exploitation naturally
- [ ] Sequence planning with 3 steps (prepare/execute/complete) mimics real motor decomposition
- [ ] Reward prediction error update rule is asymmetric: positive and negative RPE affect GO/NOGO differently
- [ ] Confidence grows as |RPE| shrinks — natural measure of motor expertise
- [ ] Exploration rate decays with confidence — models infant-to-adult transition automatically
- [ ] Forward model learning is just gradient descent on prediction error × features

### Architecture
- [ ] MotorSystem fits as a stack-allocated member of RethinkBrain — no malloc needed
- [ ] Action repertoire (max 32 actions) is stored as array of MotorAction structs
- [ ] History ring buffer (64 entries) enables temporal credit assignment
- [ ] The motor system naturally connects to perception (context), emotion (reward), and attention (selection)
- [ ] Chatbot commands (do, plan, actions) provide interactive access to the motor system

## Key Insight
> The brain doesn't just pick actions — it PREDICTS their outcomes, executes, compares reality
> to prediction, and updates. This is the sensorimotor loop. Our motor system implements this
> complete loop: forward model → execute → RPE → learn. After enough cycles, the brain
> automatically shifts from exploration to exploitation.

## Surprise
- [ ] How the asymmetric RPE update naturally makes rewarded actions dominant over time
- [ ] That confidence and exploration are inversely linked without any explicit programming
- [ ] The simplicity of the basal ganglia model — just GO minus NOGO — yet it produces complex selection behavior
- [ ] Motor noise in execution is biologically realistic AND makes the system more robust

## What I'd Do Differently Next Time
- (fill in after reflection)
