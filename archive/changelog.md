# Nous — Changelog

> Nothing is ever deleted. Every version lives here. Every decision is tracked.

---

## Version Log

| Version | Date | Phase | What Changed | Notes |
|---------|------|-------|-------------|-------|
| V0 | 2026-03-05 | Pre-project | Original C++ guide archived | The starting point — a standard AI framework guide that inspired rethink AI, but follows algorithms instead of brain science |
| V1 | 2026-03-05 | Phase 0-1 | Project structure created, The Neuron phase written | First real neurons — spiking, leaking, firing based on thresholds, not weighted sums |
| V11 | 2026-03-05 | Phase 11 | Sensory Expansion — auditory, tactile, multi-modal binding | Three new perception modules: cochlea-inspired hearing, somatosensory touch with adaptation, Bayesian multi-modal fusion (STS/SC). 7 guide files, 6 source files, integrated into rethink_brain |
| V12 | 2026-03-05 | Phase 12 | Attention Mechanisms — biased competition, gain modulation | Replaced single attention float with full biased competition system (Desimone & Duncan 1995). Dorsal/ventral attention networks, priority maps, lateral inhibition, gain modulation, limited capacity budget. Also added interactive chatbot (`chatbot.c`). 5 guide files, 2 source files, integrated into rethink_brain |
| V13 | 2026-03-05 | Phase 13 | Motor System — basal ganglia action selection, cerebellar forward model | GO/NO-GO competition (disinhibition), Reward Prediction Error (dopamine), SMA sequence planning, cerebellar forward model, exploration→exploitation transition. Added batch training from file, attention commands, motor commands to chatbot. 5 guide files, 2 source files, 3 training data files, 2 user guides, integrated into rethink_brain |
| V14 | 2026-03-05 | Phase 14 | Social Brain — Theory of Mind, trust, mirror neurons, empathy | TPJ belief tracking per agent, vmPFC Bayesian trust (3× negativity bias), mirror neuron imitation learning connected to motor system, anterior insula empathy via simulation, VTA social reward (helping/helped/cooperation). 5 guide files, 2 source files, 12 chatbot commands (agent, agents, mind, believe, cooperate, defect, observe, mirror, emotion, empathize, help_agent, helped), integrated into rethink_brain |
| V15 | 2026-03-05 | Phase 15 | Metacognition — confidence, strategy selection, self-model | aPFC multi-source confidence monitoring (consistency + prediction + memory) with calibration. ACC error/conflict detection with streak tracking. dlPFC value-based strategy selection (softmax with adaptive temperature). Meta-learning adaptive learning rate (surprise × uncertainty). Self-model with per-domain skill estimates and metacognitive accuracy. 5 guide files, 2 source files, 11 chatbot commands (confident, errors, strategies, select, self, introspect, meta_lr, meta_perf, meta_err, meta_self), integrated into rethink_brain |
| **NOUS V1** | **2026-03-06** | **V1 Release** | **Natural language chatbot, model naming, product release** | **Chatbot rewritten with NLU: 3-phase dispatch (conversation → natural language → commands), 10 conversation handlers, natural phrase mapping to 50+ commands, unknown-input handler. Model named "Nous" (νοῦς = mind). README rewritten as product page. Training data guide with 6 dataset links. Code audit: 52 files clean. chatbot.c expanded 1,279 → 1,878 lines.** |

---

## Archive Index

| File | Original Location | Archived Date | Reason |
|------|------------------|---------------|--------|
| `build_ai_from_scratch_complete_guide.md` | `Guides/` | 2026-03-05 | Kept in Guides/ as V0 reference. Standard ML framework guide (C++, backprop-based). Rethink AI takes a fundamentally different approach. |

---

## Versioning Rules

1. **Never delete** — move to `archive/` with a note in this changelog
2. **Tag versions** when a phase milestone is complete (V1, V2, etc.)
3. **Each version** = a working state where something new actually WORKS
4. **Mistakes are progress** — they go in `Mistakes/`, not in the trash
5. **When rewriting a guide**: keep old version in `archive/versions/`, create new one in `Guides/`

---

## Philosophy Version History

| Version | Core Idea |
|---------|-----------|
| V0 | "Build AI from standard ML algorithms in C++" |
| V1 | "Build AI that works like a physical brain — neurons that fire, memories that fade, understanding that emerges — in raw C" |
| V11 | "The brain doesn't see, hear, and touch separately — it fuses. Precision-weighted Bayesian binding, cochlear frequency decomposition, somatosensory adaptation with pain override." |
| V12 | "Attention is competition. Every stimulus fights for limited processing resources. Winners get amplified, losers get suppressed. Biased competition with gain modulation, priority maps, and budget depletion." |
| V13 | "The brain doesn't just think — it ACTS. Actions compete via GO/NO-GO pathways, outcomes drive dopamine learning, and the cerebellum predicts before executing. Exploration fades as confidence grows." |
| V14 | "Intelligence is social. The brain models other minds (TPJ), tracks trust through asymmetric Bayesian updates (vmPFC), learns by watching (mirror neurons), and feels what others feel (anterior insula). Social reward uses the same dopamine currency as physical reward — cooperation is literally rewarding." |
| V15 | "The brain thinks about its own thinking. Confidence monitoring (aPFC), error detection (ACC), strategy selection (dlPFC), adaptive learning rate (meta-learning), and a self-model that tracks its own capabilities. The recursive loop that makes intelligence self-correcting." |
| **NOUS V1** | **"The brain doesn't just compute — it converses. Nous understands 'hi', 'what are you', 'how confident are you'. Three-phase NLU dispatch turns natural language into brain operations. Named after the Greek word for mind. V1: a brain that talks back."** |
