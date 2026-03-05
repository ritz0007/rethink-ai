# Phase 14 — Social Brain: Mistakes & Pitfalls

> Rethink AI — V14

---

## Mistakes Made

### 1. Initial Trust Too High or Too Low
**Problem**: First attempt set initial trust to 0.7 (optimistic). This meant even strangers were trusted almost fully, making defection have outsized impact.
**Fix**: Set initial trust to 0.5 (neutral). The brain must learn to trust through observed cooperation, matching the real TPJ/vmPFC development arc.

### 2. Empathy Without Familiarity Scaling
**Problem**: Early empathy was just `their_emotion × empathy_level`. This meant we felt maximum empathy for strangers we'd never interacted with.
**Fix**: Added familiarity modulation: `(0.3 innate + 0.7 × familiarity)`. The 30% innate floor ensures basic empathy even for strangers (mirror neurons fire automatically), while deep empathy requires relationship.

### 3. Mirror Neurons Firing Too Eagerly
**Problem**: Every observation matched above threshold, causing the brain to learn every observed action immediately.
**Fix**: Activation scaled by trust × familiarity. Low-trust strangers have lower mirror activation. Threshold of 0.5 filters out weak observations.

### 4. Forgetting to Decay Beliefs
**Problem**: Once we recorded "Alice knows about cats", that belief persisted forever. But real people forget.
**Fix**: Added `belief_decay = 0.98` per tick. Certainties decay 2% per tick, and beliefs with certainty < 0.01 are garbage-collected. We assume others forget at rates similar to our own memory decay (Phase 4).

### 5. Social Reward Not Feeding Back to Motor
**Problem**: Social reward was computed but existed in isolation — it didn't affect the motor system's RPE learning.
**Fix**: In the chatbot, `help_agent` and `helped` also call `social_cooperate()`, which updates trust. Social reward conceptually feeds the same dopamine circuit as motor reward, creating unified reward learning.

---

## Things to Watch

| Issue | Risk | Mitigation |
|-------|------|------------|
| Agent slot exhaustion | 8 agents max — realistic but limiting | Could implement LRU eviction in future |
| Belief tracking granularity | Only concept names, not propositions | Sufficient for first-order ToM |
| No deception detection | Brain can't detect when someone lies | Would need prediction error on social expectations |
| No second-order ToM | Can't model "what Alice thinks Bob knows" | Recursive ToM is V14+, extremely rare even in humans |
| Mirror neuron buffer not auto-clearing | Old observations persist until manual clear | Could auto-clear on tick or capacity |

---

## What I'd Do Differently Next Time

1. **Add prediction tracking from the start**: Record what we predicted an agent would do, then compare to what they actually did. This is how the real TPJ calibrates its models.
2. **Implement social learning rate adaptation**: Familiar agents should have faster belief updates (we're more confident in our model of close friends).
3. **Connect social reward to the main experience pipeline**: Social interactions should generate full `Experience` records, complete with emotional valence and surprise.
