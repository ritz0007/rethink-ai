# Nous — Project Status

> Last Updated: March 6, 2026

---

## Current Stage: V1 Release — Nous (νοῦς)

**Nous** is the V1 release of the Rethink Brain project — a brain-inspired AI system written entirely in raw C with 24 brain modules and zero external dependencies.

V1 upgrades the chatbot from command-only to a natural language understanding companion. Nous responds to greetings, farewells, feelings, opinions, compliments, small talk, and maps natural phrases ("this is a cat", "what do you remember", "how confident are you") to existing brain commands. Three-phase dispatch: conversation → natural language → exact commands. The model is named Nous (Greek for "mind"). README rewritten as product page. Training data guide created with links to 6 open datasets.

All 15 phases (V1-V15) plus the V1 release are complete. The brain compiles, links, and runs successfully. ~7,800 lines of C across 52 files.

---

## Phase Completion Status

| Phase | Title | Guides | Code | Status |
|-------|-------|--------|------|--------|
| 00 | Getting Started | 3 files | — | Done |
| 01 | The Neuron | 5 files | `neuron.c`, `network.c`, `synapse.h` | Done |
| 02 | Learning Without Backprop | 5 files | `hebbian.c`, `stdp.c` | Done |
| 03 | Seeing the World | 5 files | `retina.c`, `som.c` | Done |
| 04 | Memory | 7 files | `hopfield.c`, `decay.c` | Done |
| 05 | Learning Like a Child | 5 files | `prototype.c` | Done |
| 06 | Thinking Without Input | 6 files | `spontaneous.c`, `dream.c` | Done |
| 07 | The Modular Brain | 5 files | `brain.c`, `emotion.c` | Done |
| 08 | Understanding | 6 files | `predictor.c`, `causal.c` | Done |
| 09 | Communication | 4 files | `comm.c` | Done |
| 10 | The Rethink Brain | 5 files | `rethink_brain.c`, `test_brain.c` | Done |
| 11 | Sensory Expansion | 7 files | `auditory.c`, `tactile.c`, `multimodal.c` | Done |
| 12 | Attention Mechanisms | 5 files | `attention.c`, `chatbot.c` | Done |
| 13 | Motor System | 5 files | `motor.c` | Done |
| 14 | Social Brain | 5 files | `social.c` | Done |
| 15 | Metacognition | 5 files | `meta.c` | Done |

**Total: 87 guide files, 25 C source files, 25 header files, 1 Makefile, 1 chatbot (1,878 lines)**

---

## What's Been Built

### Brain Modules (all in `src/`)

| Module | File(s) | Brain Region | What It Does |
|--------|---------|-------------|-------------|
| LIF Neuron | `neurons/neuron.c` | Biological neuron | Leaky Integrate-and-Fire spiking |
| Synapse | `neurons/synapse.h` | Synaptic junction | Weighted + delayed connections |
| Network | `neurons/network.c` | Neural circuit | Spiking network with spike routing |
| Hebbian | `learning/hebbian.c` | Cortical plasticity | "Fire together, wire together" |
| STDP | `learning/stdp.c` | Synaptic timing | Spike-timing dependent plasticity |
| Prototype | `learning/prototype.c` | Inferotemporal cortex | Few-shot category learning |
| Retina | `perception/retina.c` | Primary visual cortex (V1) | Edge detection + lateral inhibition |
| SOM | `perception/som.c` | Cortical maps | Self-organizing topographic maps |
| Hopfield | `memory/hopfield.c` | Hippocampus | Associative content-addressable memory |
| Decay | `memory/decay.c` | Hippocampus | Ebbinghaus forgetting + consolidation |
| Spontaneous | `thinking/spontaneous.c` | Default Mode Network | Thinking without input, mental modes |
| Dream | `thinking/dream.c` | Sleep circuits | NREM replay + REM recombination |
| Brain Bus | `modular/brain.c` | Thalamus | Signal routing + attention gating |
| Emotion | `modular/emotion.c` | Amygdala | Fear/reward, emotional memory |
| Predictor | `understanding/predictor.c` | Prefrontal cortex | Hierarchical predictive coding |
| Causal | `understanding/causal.c` | Prefrontal + temporal | Causal reasoning + counterfactuals |
| Comm | `communication/comm.c` | Broca/Wernicke areas | Grounded symbol communication |
| Auditory | `perception/auditory.c` | Primary auditory cortex (A1) | Cochlea-inspired frequency decomposition, onset detection, spectral tracking |
| Tactile | `perception/tactile.c` | Somatosensory cortex (S1) | 8-channel touch with homunculus weighting, adaptation, pain fast-path |
| Multi-Modal | `perception/multimodal.c` | STS + Superior Colliculus | Bayesian precision-weighted binding, 3 binding rules, conflict detection |
| Attention | `modular/attention.c` | Dorsal/ventral attention networks | Biased competition, gain modulation, priority maps, limited capacity budget |
| Motor | `motor/motor.c` | Basal ganglia + cerebellum | GO/NO-GO action selection, forward model, RPE learning, sequence planning |
| Social | `social/social.c` | TPJ + mPFC + vmPFC + mirror neurons | Theory of Mind, trust, imitation, empathy, social reward |
| Meta | `meta/meta.c` | aPFC + ACC + dlPFC + AIC | Confidence monitoring, error detection, strategy selection, meta-learning, self-model |

### Integration
- `rethink_brain.c` — Connects all 24 modules into one unified brain
- `test_brain.c` — Full "Day in the Life" integration test
- `chatbot.c` — Interactive terminal chatbot front-end to the brain
- `Makefile` — `make` builds library, `make test` builds + links test, `make chat` builds chatbot

### Documentation
- **Guides/** — 85 markdown files across 16 phase folders + 2 user guides
- **Learnings/** — 16 template files (one per phase) for recording insights
- **Mistakes/** — 16 template files (one per phase) for recording errors
- **archive/** — Changelog with version history

---

## Verification Results (Last Run)

```
Learned 3 categories: cat, dog, bird
fluffy_thing -> 'cat' (confidence=1.00)
barking_thing -> 'dog' (confidence=1.00)
Why slipping? 'slipping' because: wet_ground(60%) <- rain(90%)
What if rain? 'rain' will cause: wet_ground(100%), slipping(97%), umbrella(99%)
Brain says: "cat dog"
Brain idled for 50 ticks
Slept for 10 cycles — dream insights generated
```

All modules functional. Zero errors. Warnings only (format-truncation, const-discard — harmless).
Attention: starts at 0.50, depletes to 0.41 after 2 experiences (budget depletion working).
Motor: exploration=0.30, confidence=0.00 (brand new brain, no motor experience yet).
Social: empathy=0.60, trust_build_rate=0.10, trust_decay_rate=0.30 (3× negativity bias).
Meta: confidence=0.50 (starting uncertain), 4 strategies, self-model at 0.50 across all domains.

---

## Future Roadmap — Uncompleted Tasks

### V11: Sensory Expansion
- [x] Design auditory processing module (`auditory.h/.c`)
- [x] Design tactile input module (`tactile.h/.c`)
- [x] Implement frequency analysis for sound patterns
- [x] Implement multi-modal binding (sight + sound + touch into one experience)
- [x] Write brain science guide: superior temporal sulcus, multi-sensory integration
- [x] Integrate new senses into rethink brain experience pipeline
- [x] Write milestone guide: multi-modal verification checklist

### V12: Attention Mechanisms
- [x] Replace single `attention_level` float with real attention system
- [x] Implement spotlight attention (focus one thing, suppress others)
- [x] Implement feature-based attention (attend to color across whole scene)
- [x] Implement temporal attention (attend to specific moment in sequence)
- [x] Add attention as limited resource with competition dynamics
- [x] Write brain science guide: dorsal/ventral attention networks, biased competition
- [x] Extend thalamus with competition dynamics and gain modulation
- [x] Write milestone test: attention switching under load
- [x] Build interactive chatbot front-end to the brain

### V13: Motor System
- [x] Design motor module (`motor.h/.c`)
- [x] Implement action planning (predict outcomes before executing)
- [x] Implement motor sequences (reach → grasp → pull chaining)
- [x] Implement reward-based action selection
- [x] Add embodied cognition (understanding through doing)
- [x] Write brain science guide: motor cortex, basal ganglia, cerebellum, mirror neurons
- [x] Write milestone test: learn action sequences from few examples
- [x] Add batch training from file (`train` command)
- [x] Add chatbot attention commands (`focus`, `attend`)
- [x] Add chatbot motor commands (`do`, `plan`, `actions`)
- [x] Create training data files and training guide

### V14: Social Brain
- [x] Design social module (`social.h/.c`)
- [x] Implement Theory of Mind (model what other agents know/want)
- [x] Implement imitation learning (learn by watching)
- [x] Add social reward signals (helping/being helped)
- [x] Build trust models through repeated interaction
- [x] Write brain science guide: mirror neurons, temporoparietal junction
- [x] Write milestone test: social brain verification checklist
- [x] Add 12 chatbot social commands (agent, agents, mind, believe, cooperate, defect, observe, mirror, emotion, empathize, help_agent, helped)

### V15: Metacognition
- [x] Design meta module (`meta.h/.c`)
- [x] Implement confidence monitoring (how sure am I?)
- [x] Implement strategy selection (which approach to use)
- [x] Add learning-to-learn (adjust learning rate based on performance)
- [x] Build self-model (internal representation of "self")
- [x] Write brain science guide: anterior prefrontal cortex, anterior insular cortex
- [x] Write milestone test: brain recognizes its own uncertainty
- [x] Add 11 chatbot metacognition commands (confident, errors, strategies, select, self, introspect, meta_lr, meta_perf, meta_err, meta_self)

### V1 Release: Nous
- [x] Natural Language Understanding — chatbot understands greetings, feelings, opinions, small talk
- [x] Three-phase dispatch: conversation → natural language mapping → exact commands
- [x] 10 conversation handlers (greeting, farewell, thanks, how-are-you, what-are-you, name, compliment, feeling, opinion, small-talk)
- [x] Natural language → command mapping ("this is a cat"→teach, "show me X"→show, "what if X"→whatif, etc.)
- [x] Unknown input handler with brain-state-aware responses
- [x] Prompt changed to "nous>" with Nous branding throughout
- [x] Code audit: all 52 files clean (no TODOs, no stubs, no dead code)
- [x] Model named "Nous" (Greek νοῦς = mind)
- [x] README rewritten as product page
- [x] Training data guide with 6 open dataset links (ConceptNet, WordNet, Wikipedia, NELL, DailyDialog, OMCS)
- [x] chatbot.c expanded from 1,279 to 1,878 lines

### Long-Term Research Directions
- [ ] Consciousness: Explore Integrated Information Theory (IIT), Global Workspace Theory
- [ ] Creativity: Analogy, bisociation, exploration, evaluation
- [ ] Developmental learning: Critical periods, curriculum, self-directed exploration
- [ ] Neuroplasticity: Synaptogenesis, pruning, neurogenesis, structural change
- [ ] Curiosity-driven learning: Actively seek novel/surprising experiences

---

## Per-Phase Checklist (Verification)

### Phase 00 — Getting Started
- [x] Understand the Rethink AI philosophy (brain-inspired, not statistical)
- [x] GCC + VS Code setup documented
- [x] Project folder structure defined with brain-to-code mapping

### Phase 01 — The Neuron
- [x] LIF spiking neuron with membrane potential, threshold, refractory period
- [x] Synapses with weights and propagation delays
- [x] Spiking neural network with spike routing and external input

### Phase 02 — Learning Without Backprop
- [x] Hebbian learning rule with Oja's normalization
- [x] STDP with exponential timing window
- [x] Documented 5 biological problems with backpropagation

### Phase 03 — Seeing the World
- [x] Retina with 4 edge kernels (H/V/DL/DR) and lateral inhibition
- [x] Self-Organizing Map with BMU finding and Gaussian neighborhood

### Phase 04 — Memory
- [x] Hopfield content-addressable memory (give a piece, get the whole)
- [x] Memory decay with Ebbinghaus forgetting curve
- [x] Recall strengthening (using a memory makes it stronger)
- [x] Importance tagging (important memories decay slower)
- [x] Consolidation (dream mode replays important memories)

### Phase 05 — Learning Like a Child
- [x] One-shot learning (categories from single examples)
- [x] Few-shot improvement (accuracy increases with more examples)
- [x] Novelty detection (unknowns flagged as unknown)
- [x] Instant new categories without retraining
- [x] Feature attention via contrastive learning

### Phase 06 — Thinking Without Input
- [x] Spontaneous neural activity (noise-driven thought)
- [x] Three mental modes (focused / creative / dreaming)
- [x] NREM replay for memory consolidation
- [x] REM recombination for creative insight
- [x] Insight discovery during sleep

### Phase 07 — The Modular Brain
- [x] Six specialized brain regions
- [x] Thalamus central routing with priority/surprise gating
- [x] Competitive attention (zero-sum)
- [x] Amygdala emotion system (fear/reward/surprise/familiar/disgust)
- [x] Fast danger detection pathway
- [x] Emotion-modulated memory encoding

### Phase 08 — Understanding
- [x] Hierarchical predictive coding (learn patterns, detect surprise)
- [x] Precision-weighted prediction errors
- [x] Forward causal reasoning (predict effects)
- [x] Backward causal reasoning (explain causes)
- [x] Counterfactual reasoning (imagine what-ifs)
- [x] Causal learning from temporal co-occurrence
- [x] Intervention testing (break causal links)

### Phase 09 — Communication
- [x] Grounded symbols (features, not just IDs)
- [x] Encode internal state → word sequence (speaking)
- [x] Decode word sequence → internal state (understanding)
- [x] Role-based sentence structure (agent/action/patient)
- [x] Emotional valence preserved in communication
- [x] Lossy round-trip (biologically accurate)

### Phase 10 — The Rethink Brain
- [x] All 17 modules connected into one RethinkBrain struct
- [x] Full experience pipeline (perceive → predict → emote → classify → remember)
- [x] Causal reasoning (explain why, predict what-if)
- [x] Communication (speak internal state, understand word sequences)
- [x] Idle thinking (spontaneous activity, thought capture)
- [x] Sleep cycle (NREM replay + REM + insight detection)
- [x] Integration test passes with all modules functional

### Phase 11 — Sensory Expansion
- [x] Auditory system: cochlea DFT into 16 frequency bands with sensitivity weighting
- [x] Auditory onset/offset detection (cochlear nucleus analog)
- [x] Spectral flux + centroid tracking (inferior colliculus analog)
- [x] 32-dimension auditory feature extraction
- [x] Tactile system: 8 channels with homunculus-weighted sensitivity
- [x] Sensory adaptation (habituation) with pain override (nociceptors never adapt)
- [x] Pain fast-path alert + temperature extreme detection
- [x] 32-dimension tactile feature extraction
- [x] Multi-modal binding: Bayesian precision-weighted feature fusion
- [x] Three binding rules: spatial (correlation), temporal (coherence), inverse effectiveness
- [x] Conflict detection (senses disagree)
- [x] Integrated into rethink_brain experience pipeline
- [x] Build compiles with zero errors (20 object files → librethink.a)
- [x] Full integration test passes

### Phase 12 — Attention Mechanisms
- [x] Biased competition: stimuli compete, attention biases the winner (Desimone & Duncan 1995)
- [x] Bottom-up salience: deviation from average + feature uniqueness
- [x] Top-down relevance: cosine similarity to goals × goal strength
- [x] Priority map: weighted sum of salience + relevance + emotional significance
- [x] Lateral inhibition: stimuli suppress each other, winner-take-most
- [x] Gain modulation: multiplicative amplification of winner, suppression of losers
- [x] Limited capacity budget: attention depletes with use, recovers at rest
- [x] Goal system: up to 4 simultaneous goals with type, strength, and TTL
- [x] Ventral attention capture: high salience+emotion can override focused attention
- [x] Replaced single attention_level float with full competition system
- [x] Interactive chatbot: terminal-based front-end with 15+ commands
- [x] Integrated into rethink_brain experience pipeline
- [x] Build compiles with zero errors (21 object files → librethink.a)
- [x] Full integration test passes

### Phase 13 — Motor System
- [x] Basal ganglia: GO/NO-GO competition via disinhibition (direct/indirect pathways)
- [x] Action evaluation: 40% net strength + 30% context relevance + 30% forward model prediction
- [x] Epsilon-greedy selection: exploration (random) vs exploitation (best)
- [x] SMA sequence planning: prepare → execute → complete (3-step decomposition)
- [x] Cerebellum forward model: per-action weights predict reward from state features
- [x] Motor execution with biological noise (80% expected + random noise)
- [x] Dopamine RPE learning: δ = actual - predicted, asymmetric GO/NOGO update
- [x] GO/NOGO weights clamped to [0.0, 2.0], confidence clamped to [0.0, 1.0]
- [x] Exploration decays as confidence grows (development model)
- [x] Action history ring buffer (64 entries)
- [x] Chatbot: train, focus, attend, do, plan, actions commands
- [x] 3 training data files: animals, weather, daily_life
- [x] Build compiles with zero errors (22 object files → librethink.a)
- [x] Full integration test passes

### Phase 14 — Social Brain
- [x] TPJ: separate belief model per agent (up to 8 agents, 16 beliefs each)
- [x] Belief tracking: certainty updated with observations, decayed over time
- [x] mPFC: desire feature vectors and emotional state per agent
- [x] vmPFC: trust score with Bayesian-like updates
- [x] Trust asymmetry: build rate=0.10, decay rate=0.30 (3× negativity bias)
- [x] Trust clamped to [0.0, 1.0], familiarity clamped to [0.0, 1.0]
- [x] Mirror neuron system: observe actions, compute activation, gate imitation by trust
- [x] Imitation learning: observed actions added to motor repertoire when above threshold
- [x] Existing motor actions strengthened by trust-weighted observation
- [x] Social reward: helping bonus × empathy + being-helped bonus + cooperation × trust
- [x] Empathy: their_emotion × empathy_level × (0.3 innate + 0.7 familiarity)
- [x] Familiarity grows with interaction (diminishing returns), decays slowly without contact
- [x] Interaction history ring buffer (64 entries)
- [x] 12 chatbot commands: agent, agents, mind, believe, cooperate, defect, observe, mirror, emotion, empathize, help_agent, helped
- [x] Auto-add agents on first interaction
- [x] Build compiles with zero errors (23 object files → librethink.a)
- [x] Full integration test passes

### Phase 15 — Metacognition
- [x] aPFC: multi-source confidence monitoring (consistency + prediction + memory)
- [x] Confidence calibration: ECE tracking + systematic bias correction
- [x] ACC: error magnitude tracking, streak detection, cognitive load
- [x] dlPFC: strategy repertoire with value-based softmax selection
- [x] Adaptive temperature: low when confident (exploit), high after errors (explore)
- [x] Strategy learning: success/effort ratio updated on every outcome
- [x] Meta-learning: adaptive learning rate = base × surprise × uncertainty
- [x] Learning trajectory: tracks performance trend (improving/declining/stable)
- [x] Self-model: per-domain skill estimates (perception, memory, reasoning, learning, social, motor)
- [x] Metacognitive accuracy: how well the self-model matches actual performance
- [x] Cognitive stamina: gradual fatigue with minimum 10% baseline
- [x] Curiosity: adapts to learning trajectory (improves → less curious, declining → more curious)
- [x] Introspection loop: full metacognitive cycle (assess → detect → adapt → model)
- [x] Performance history ring buffer (64 entries)
- [x] 11 chatbot commands: confident, errors, strategies, select, self, introspect, meta_lr, meta_perf, meta_err, meta_self
- [x] Build compiles with zero errors (24 object files → librethink.a)
- [x] Full integration test passes

---

## File Counts

| Category | Count |
|----------|-------|
| Guide files | 87 (incl. V0 reference + 2 user guides + training data guide) |
| Learnings templates | 16 |
| Mistakes templates | 16 |
| C source files (.c) | 25 (incl. chatbot) |
| C header files (.h) | 25 |
| Test file | 1 |
| Chatbot | 1 (1,878 lines, NLU-powered) |
| Training data files | 3 |
| Makefile | 1 |
| Archive | 3 (changelog + chatbot_v15 backup + README_v15 backup) |
| **Total** | **178** |

---

## Build & Run

```bash
cd src
make          # Builds librethink.a (static library)
make test     # Builds + links test_brain
./test_brain  # Runs full integration test
make chat     # Builds interactive chatbot
./chatbot     # Run the chatbot
make clean    # Removes all build artifacts
```

**Compiler**: GCC with `-std=c11 -O2 -Wall -Wextra -Wpedantic -lm`
**Dependencies**: None. Only C standard library + math.h.

---

*Nous V1 — a brain that thinks, not a model that predicts. Built with neurons, not matrices.*
