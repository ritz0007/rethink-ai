# V14 Milestone — Social Brain Verification

> Rethink AI — Phase 14, Guide 5 of 5

---

## Build Verification

### Compilation
- [ ] `social.h` compiles without errors
- [ ] `social.c` compiles without errors  
- [ ] `make` produces 23 object files → `librethink.a`
- [ ] `make test` builds and links `test_brain`
- [ ] `make chat` builds and links `chatbot`
- [ ] Only pre-existing harmless warnings (format-truncation, const-discard)

### Unit Tests (via chatbot)

#### Agent Management
- [ ] `agent alice` → creates agent model with neutral trust (0.5)
- [ ] `agent bob` → creates second agent model
- [ ] `agents` → lists both agents with trust, familiarity, beliefs

#### Theory of Mind
- [ ] `believe alice cat` → updates Alice's belief model (she knows about cats)
- [ ] `believe alice rain` → Alice also knows about rain
- [ ] `mind alice` → shows Alice's complete mental model
- [ ] `query alice cat` → returns certainty > 0 for cat

#### Trust
- [ ] `cooperate alice 0.8` → trust increases (slowly)
- [ ] `cooperate alice 0.8` → trust increases further
- [ ] `defect alice 0.5` → trust drops significantly (fast erosion)
- [ ] Trust asymmetry: one defection erases multiple cooperations

#### Mirror Neurons
- [ ] `observe alice reach_ball` → mirror neuron activation
- [ ] `mirror` → shows mirror buffer with activation levels

#### Social Reward
- [ ] `help alice` → social reward > 0 (helping bonus × empathy)
- [ ] `helped alice` → social reward > 0 (being helped bonus)

#### Empathy
- [ ] `emotion alice 0.8` → set Alice's emotional state to positive
- [ ] `empathize alice` → positive empathic response (scaled by familiarity + empathy)
- [ ] Empathy stronger after more interactions (higher familiarity)

#### Time Effects
- [ ] After idle/think periods, belief certainties decay
- [ ] Familiarity decays very slowly without interaction
- [ ] Trust is stable without new interactions

---

## Integration Tests

### With Motor System (V13)
- [ ] Mirror neuron observation maps to motor repertoire
- [ ] Social reward modulates motor learning
- [ ] Imitation creates new motor actions

### With Emotion System (V7)
- [ ] Empathy feeds through emotion system
- [ ] Trust defection triggers negative emotion

### With Attention (V12)
- [ ] Social agents can become attention goals
- [ ] High-trust interactions get attention priority

---

## Expected Output Example

```
brain> agent alice
  Added agent: 'alice' (trust=0.50, familiarity=0.00)

brain> believe alice cat
  Updated Alice's belief: 'cat' (certainty=0.80)

brain> cooperate alice 0.8
  Cooperated with Alice (quality=0.80)
  Trust: 0.50 → 0.54 (+0.04)

brain> defect alice 0.5
  Defection by Alice (severity=0.50)
  Trust: 0.54 → 0.46 (-0.08)

brain> mind alice
  === Mental Model: alice ===
  Trust:       0.46
  Familiarity: 0.10
  Emotion:     0.00
  Beliefs:
    cat (certainty=0.80)
  Interactions: 2

brain> help alice
  Helped Alice! Social reward: 0.24
```

---

## Verification Commands Summary

```bash
cd src
make clean && make test && make chat

# In chatbot:
agent alice
agent bob
agents
believe alice cat
believe alice rain
mind alice
cooperate alice 0.8
cooperate alice 0.8
defect alice 0.5
mind alice
observe alice reach_ball
mirror
emotion alice 0.7
empathize alice
help alice
helped bob
status
quit
```

---

## What This Proves

V14 demonstrates that our brain can:
1. **Model other minds** — separate mental models per agent
2. **Track trust** — with biologically realistic asymmetry
3. **Learn by observation** — mirror neurons connect to motor system
4. **Feel empathy** — simulate others' emotions internally
5. **Value cooperation** — social reward in the same currency as physical reward

The social brain is what makes intelligence *useful*. Without it, even perfect perception and reasoning are isolated. With it, knowledge can flow between agents.

---

*This phase bridges individual cognition to collective intelligence.*
