# Phase 14 — Social Brain: Learnings

> Rethink AI — V14

---

## What I Learned

### 1. Trust Asymmetry Is Fundamental
Trust builds slowly (α=0.10) but erodes fast (β=0.30). One defection at severity=0.5 erased two full cooperations. This 3× asymmetry matches the neuroscience — the amygdala reacts to betrayal far faster than the vmPFC builds trust. Evolutionary logic: the cost of trusting a predator is death, while missing a friendship is merely a lost opportunity.

### 2. Theory of Mind = Separate Models Per Agent
The TPJ maintains independent mental models for each known agent. Alice's beliefs are tracked separately from Bob's. This sounds obvious, but it's the foundation of social cognition — you can't have false belief reasoning without separating "what I know" from "what they know."

### 3. Mirror Neurons Bridge Perception and Action
The key insight: when we observe someone perform an action, we literally simulate it in our own motor cortex. This means V14 (social) naturally connects to V13 (motor) — observation triggers motor action learning through trust-gated imitation. The activation threshold prevents us from mindlessly copying everything.

### 4. Empathy Is Computation, Not Magic
Empathy = their_emotion × empathy_level × (0.3 innate + 0.7 familiarity). Even with strangers, 30% innate empathic response (mirror neurons fire automatically). With close friends, the full response. This formula captures the observation that empathy is stronger for in-group members.

### 5. Social Reward Uses Same Currency as Physical Reward
Helping others generates dopamine-equivalent reward, just like successful motor actions in V13. The brain doesn't distinguish "I found food" from "I helped a friend" at the reward circuit level — both feed into VTA dopamine. This is why cooperation evolved: it literally feels good.

### 6. Familiarity Is the Master Modulator
Everything improves with familiarity: belief tracking precision, empathy strength, trust stability, imitation gating. This matches human experience — we predict close friends far better than strangers.

---

## Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| 8 agent slots | Matches Dunbar's innermost circle (~5 intimate contacts) |
| 16 beliefs per agent | Rich enough for meaningful ToM, bounded for memory |
| Auto-add agents on first interaction | Reduces friction, matches how we actually meet people |
| Belief decay per tick | We assume others forget, matching our own memory decay |
| Trust clamped [0,1] | Prevents runaway trust or distrust |
| Mirror buffer cleared manually | Allows examining observation history before clearance |

---

## Brain Regions → Code Mapping

| Brain Region | Code Component | Function |
|---|---|---|
| TPJ | `AgentModel.beliefs[]` | Track what each agent knows |
| mPFC | `AgentModel.desires[]`, `emotional_state` | Model wants and feelings |
| vmPFC | `AgentModel.trust`, `cooperation_history` | Trust computation |
| Premotor F5 + IPL | `MirrorActivation` buffer | Mirror neuron firing |
| Anterior Insula | `social_empathize()` | Empathy via simulation |
| VTA | `social_compute_reward()` | Social reward signal |
