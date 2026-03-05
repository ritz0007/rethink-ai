# Phase 12 — Attention Mechanisms: Learnings

## What I Learned

### Brain Science
- [ ] The brain has TWO attention networks: dorsal (top-down, goal-directed) and ventral (bottom-up, stimulus-driven)
- [ ] Biased competition (Desimone & Duncan 1995) — stimuli compete, attention biases the winner
- [ ] Gain modulation is MULTIPLICATIVE, not additive — winner's signal is amplified, losers are suppressed
- [ ] Attention is a LIMITED resource — you can't attend to everything equally (capacity bottleneck)
- [ ] The priority map combines bottom-up salience + top-down relevance + emotional significance
- [ ] Lateral inhibition in attention = stimuli directly suppress each other (winner-take-most)
- [ ] The ventral network can OVERRIDE the dorsal network — an unexpected loud bang captures attention involuntarily
- [ ] Attention has temporal dynamics: goals have a TTL (time-to-live), focus fatigues over time
- [ ] Divisive normalization is the canonical neural computation — it's everywhere in the brain

### Implementation
- [ ] Cosine similarity between feature vectors and goal targets is a clean proxy for top-down relevance
- [ ] Bottom-up salience = deviation from average — things that stand out get attention
- [ ] Feature uniqueness (inverse occurrence frequency) adds a second salience signal
- [ ] The biased competition pipeline (salience → relevance → compete → modulate) maps cleanly to 4 functions
- [ ] Budget depletion + recovery models the "can't focus forever" phenomenon naturally
- [ ] Attention capture check = if (salience × emotion) > threshold — simple but effective
- [ ] Winner-take-most (not winner-take-all) via suppression_gain=0.3 — losers still contribute, just softly
- [ ] Gain modulation replaces the old single `attention_level` float with a full competition system

### Architecture
- [ ] The attention system plugs into the experience pipeline between perception and memory
- [ ] AttentionItem struct carries features + raw_activation + computed scores (salience, relevance, priority, gain)
- [ ] Up to 4 simultaneous goals with different types (SPATIAL/FEATURE/TEMPORAL/OBJECT)
- [ ] The attended_features[] output can be fed directly into downstream modules (classification, memory encoding)
- [ ] Stack-allocated AttentionSystem keeps memory management simple — no malloc

## Key Insight
> Attention isn't just "paying attention" — it's a COMPETITION. Every stimulus in the brain is
> fighting for processing resources. The winner gets amplified, the losers get suppressed. This
> is why you can't listen to two conversations at once. Our biased competition model makes this
> concrete: priority maps, lateral inhibition, gain modulation, limited budget.

## Surprise
- [ ] How much the single `attention_level` float was doing — replacing it with biased competition was a big upgrade
- [ ] That gain modulation is multiplicative, not additive — this has huge implications for signal processing
- [ ] The ventral attention capture mechanism is essentially an "interrupt" system — even a focused brain can be hijacked
- [ ] Budget depletion creates realistic fatigue effects without any explicit fatigue modeling

## What I'd Do Differently Next Time
- (fill in after reflection)
