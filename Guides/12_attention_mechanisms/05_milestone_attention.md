# Phase 12 — Milestone: Attention Mechanisms Verification

## Checklist

### Attention System Core
- [ ] `attention_init()` sets sensible defaults (budget=1.0, gains, weights)
- [ ] `attention_add_item()` registers items for competition (up to 16)
- [ ] `attention_set_goal()` registers top-down bias targets (up to 4)
- [ ] `attention_process()` runs full pipeline each tick
- [ ] `attention_clear_items()` and `attention_clear_goals()` reset properly

### Biased Competition
- [ ] Multiple items with equal activation → winner is somewhat random (tie-breaking)
- [ ] Adding a top-down goal biases competition toward matching item
- [ ] Item with highest raw activation wins when no goals are set
- [ ] Lateral inhibition suppresses weaker items proportionally

### Gain Modulation
- [ ] Winner item gets gain > 1.0 (amplified)
- [ ] Loser items get gain < 1.0 (suppressed)
- [ ] `attended_features` output reflects gain-modulated winner
- [ ] Gain scales with remaining budget (low budget → weaker amplification)

### Bottom-Up Capture
- [ ] Very high salience item captures attention away from current winner
- [ ] High emotional boost item captures attention (fear/threat override)
- [ ] Below-threshold salience does NOT capture (filter works)

### Limited Capacity (Budget)
- [ ] Budget depletes when multiple items compete
- [ ] Budget recovers slowly over ticks of low demand
- [ ] Low budget degrades attention quality (lower winner gain)
- [ ] Budget never drops below 0.1 (minimum processing)

### Goal Management
- [ ] Goals with TTL expire after set number of ticks
- [ ] Multiple simultaneous goals bias toward whichever matches best
- [ ] Clearing goals removes top-down bias, leaves only bottom-up

### Integration
- [ ] AttentionSystem added to RethinkBrain struct
- [ ] `rethink_experience()` feeds items to attention before processing
- [ ] `attention_level()` replaces the old `attention_level` float
- [ ] Chatbot can set attention goals interactively
- [ ] Build compiles with zero errors
- [ ] All existing tests still pass

### Brain Science Accuracy
- [ ] Biased competition matches Desimone & Duncan (1995) model
- [ ] Gain modulation matches thalamic relay behavior
- [ ] Bottom-up capture matches ventral attention network properties
- [ ] Limited capacity matches attentional blink / change blindness findings
- [ ] Priority = salience + relevance + emotion matches priority map theory
