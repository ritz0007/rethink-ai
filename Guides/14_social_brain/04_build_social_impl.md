# Building the Social Brain — Implementation

> Rethink AI — Phase 14, Guide 4 of 5

---

## Implementation Walkthrough

### 1. Initialization — A Newborn Social Brain

```c
void social_init(SocialSystem *ss) {
    memset(ss, 0, sizeof(SocialSystem));
    ss->imitation_threshold = 0.5f;
    ss->empathy_level = 0.6f;         // Moderate empathy
    ss->helping_bonus = 0.4f;
    ss->being_helped_bonus = 0.3f;
    ss->trust_build_rate = 0.1f;      // Slow to build
    ss->trust_decay_rate = 0.3f;      // Fast to destroy (3x)
    ss->familiarity_growth = 0.05f;
    ss->belief_decay = 0.98f;         // Slow decay (2% per tick)
    ss->social_motivation = 0.5f;     // Moderate social drive
}
```

**Brain connection**: A newborn has moderate empathy (innate), no agent models yet (no social experience), and neutral social motivation.

The trust asymmetry ($\beta = 0.3 > \alpha = 0.1$) is critical: evolution wired us to detect betrayal fast because the cost of trusting a predator is death, while the cost of not trusting a friend is merely a missed opportunity.

---

### 2. Agent Management — Building Mental Models

```c
int social_add_agent(SocialSystem *ss, const char *name) {
    // Check if already known
    int existing = social_find_agent(ss, name);
    if (existing >= 0) return existing;

    // Find empty slot
    for (int i = 0; i < SOCIAL_MAX_AGENTS; i++) {
        if (!ss->agents[i].active) {
            AgentModel *a = &ss->agents[i];
            memset(a, 0, sizeof(AgentModel));
            strncpy(a->name, name, 31);
            a->active = 1;
            a->trust = 0.5f;              // Neutral trust
            a->familiarity = 0.0f;        // Stranger
            a->prediction_accuracy = 0.5f; // Chance level
            ss->num_agents++;
            return i;
        }
    }
    return -1;  // No room
}
```

**Brain connection**: Every new person starts as a blank slate with neutral trust. The TPJ creates a fresh mental model slot. Over time, beliefs, desires, and trust will diverge between agents as we learn about each one individually.

---

### 3. Theory of Mind — Tracking Beliefs

```c
void social_update_belief(SocialSystem *ss, int agent_id,
                          const char *concept, float certainty) {
    AgentModel *a = &ss->agents[agent_id];

    // Find existing belief or empty slot
    int slot = -1;
    for (int i = 0; i < a->num_beliefs; i++) {
        if (a->beliefs[i].active &&
            strcmp(a->beliefs[i].label, concept) == 0) {
            slot = i;
            break;
        }
    }

    if (slot >= 0) {
        // Update existing: certainty grows toward 1
        a->beliefs[slot].certainty +=
            0.2f * (certainty - a->beliefs[slot].certainty);
    } else if (a->num_beliefs < SOCIAL_MAX_BELIEFS) {
        // Add new belief
        slot = a->num_beliefs++;
        strncpy(a->beliefs[slot].label, concept, 31);
        a->beliefs[slot].certainty = certainty;
        a->beliefs[slot].active = 1;
    }
}
```

**Brain connection**: The TPJ tracks each concept separately. When we observe someone encountering "rain", we update our model: "they now know about rain". The certainty grows with repeated observation and decays over time (we assume they forget).

```c
float social_query_belief(const SocialSystem *ss, int agent_id,
                          const char *concept) {
    const AgentModel *a = &ss->agents[agent_id];
    for (int i = 0; i < a->num_beliefs; i++) {
        if (a->beliefs[i].active &&
            strcmp(a->beliefs[i].label, concept) == 0)
            return a->beliefs[i].certainty;
    }
    return 0.0f;  // We don't think they know this
}
```

---

### 4. Trust — The Bayesian Dance

```c
void social_cooperate(SocialSystem *ss, int agent_id, float quality) {
    AgentModel *a = &ss->agents[agent_id];
    float old_trust = a->trust;

    // Trust grows slowly
    a->trust += ss->trust_build_rate * (1.0f - a->trust) * quality;
    if (a->trust > 1.0f) a->trust = 1.0f;

    // Update cooperation history
    a->cooperation_history = a->cooperation_history * 0.9f + quality * 0.1f;
    a->interaction_count++;
    a->familiarity += ss->familiarity_growth * (1.0f - a->familiarity);

    // Record
    social_record_interaction(ss, a->name, 1,
                              a->trust - old_trust);
}

void social_defect(SocialSystem *ss, int agent_id, float severity) {
    AgentModel *a = &ss->agents[agent_id];
    float old_trust = a->trust;

    // Trust drops fast
    a->trust -= ss->trust_decay_rate * a->trust * severity;
    if (a->trust < 0.0f) a->trust = 0.0f;

    // Cooperation history takes a hit
    a->cooperation_history = a->cooperation_history * 0.7f;
    a->interaction_count++;
    a->familiarity += ss->familiarity_growth * (1.0f - a->familiarity);

    social_record_interaction(ss, a->name, 0,
                              a->trust - old_trust);
}
```

The asymmetry is deliberate: `trust_decay_rate = 0.3` vs `trust_build_rate = 0.1`. One betrayal erases many cooperative acts. This matches experimental data: it takes 5-7 cooperative actions to recover from one defection.

---

### 5. Mirror Neurons — Learning by Watching

```c
void social_observe_action(SocialSystem *ss, const char *agent_name,
                           const float *action_features, int dim,
                           const char *action_name) {
    if (ss->mirror_count >= SOCIAL_MIRROR_BUFFER) return;

    MirrorActivation *m = &ss->mirror_buffer[ss->mirror_count];
    memset(m, 0, sizeof(MirrorActivation));

    // Copy observed features
    int d = dim < SOCIAL_FEATURE_DIM ? dim : SOCIAL_FEATURE_DIM;
    memcpy(m->features, action_features, d * sizeof(float));
    strncpy(m->agent_name, agent_name, 31);
    m->active = 1;

    // The mirror activation will be matched against our motor repertoire
    // in social_should_imitate() when the motor system is consulted
    m->action_id = -1;  // Will be resolved when checking imitation
    m->activation = 0.0f;

    ss->mirror_count++;
}
```

**Brain connection**: The mirror neuron system fires automatically when we observe actions. But imitation only happens when activation exceeds threshold AND the agent is trusted enough.

```c
int social_should_imitate(const SocialSystem *ss) {
    for (int i = 0; i < ss->mirror_count; i++) {
        if (ss->mirror_buffer[i].active &&
            ss->mirror_buffer[i].activation >= ss->imitation_threshold)
            return 1;
    }
    return 0;
}
```

---

### 6. Social Reward — The Currency of Cooperation

```c
float social_compute_reward(SocialSystem *ss, int agent_id,
                            int helped, int was_helped) {
    const AgentModel *a = &ss->agents[agent_id];
    float reward = 0.0f;

    // Helping others (empathy-scaled)
    if (helped)
        reward += ss->helping_bonus * ss->empathy_level;

    // Being helped (need-scaled)
    if (was_helped)
        reward += ss->being_helped_bonus;

    // Cooperation bonus (trust-scaled)
    if (helped || was_helped)
        reward += 0.2f * a->trust;

    ss->social_reward = reward;
    return reward;
}
```

**Brain connection**: Social reward feeds into the same dopamine circuit as motor reward (V13). This means the brain values social cooperation alongside physical outcomes — exactly as in real brains where VTA dopamine neurons fire for both food and social contact.

---

### 7. Empathy — Feeling What They Feel

```c
float social_empathize(const SocialSystem *ss, int agent_id) {
    const AgentModel *a = &ss->agents[agent_id];
    // Empathic response = their emotion × empathy level × familiarity
    return a->emotional_state * ss->empathy_level *
           (0.3f + 0.7f * a->familiarity);
}
```

Even with strangers, we feel 30% empathy (innate mirror response). With close friends (familiarity → 1.0), we feel the full empathic response scaled by our empathy level.

---

### 8. Time Passage — Belief Decay

```c
void social_tick(SocialSystem *ss) {
    for (int i = 0; i < SOCIAL_MAX_AGENTS; i++) {
        if (!ss->agents[i].active) continue;
        AgentModel *a = &ss->agents[i];

        // Decay beliefs (we assume others forget)
        for (int b = 0; b < a->num_beliefs; b++) {
            if (!a->beliefs[b].active) continue;
            a->beliefs[b].certainty *= ss->belief_decay;
            if (a->beliefs[b].certainty < 0.01f) {
                a->beliefs[b].active = 0;
            }
        }

        // Familiarity decays slowly without interaction
        a->familiarity *= 0.999f;
    }
}
```

---

## Integration Points

| V14 Function | Connects To | Purpose |
|---|---|---|
| `social_observe_action()` | `motor_find_action()` | Match observation to motor repertoire |
| `social_compute_reward()` | Motor RPE learning | Social reward enters dopamine signal |
| `social_empathize()` | `amygdala_evaluate()` | Empathic emotion processing |
| `social_predict_action()` | Feature matching | Predict what agent will do next |

---

*Next: [05_milestone_social.md](05_milestone_social.md) — Verification checklist*
