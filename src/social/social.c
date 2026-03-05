/*
 * social.c — Social Brain Implementation
 *
 * Rethink AI — Phase 14 / V14
 *
 * Theory of Mind, trust, mirror neurons, empathy, social reward.
 */

#include "social.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ──── Helper: cosine similarity ──── */
static float cosine_sim(const float *a, const float *b, int dim) {
    float dot = 0.0f, na = 0.0f, nb = 0.0f;
    for (int i = 0; i < dim; i++) {
        dot += a[i] * b[i];
        na  += a[i] * a[i];
        nb  += b[i] * b[i];
    }
    if (na < 1e-9f || nb < 1e-9f) return 0.0f;
    return dot / (sqrtf(na) * sqrtf(nb));
}

/* ──── Helper: clamp ──── */
static float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

/* ════════════════════════════════════════════
 *  Lifecycle
 * ════════════════════════════════════════════ */

void social_init(SocialSystem *ss) {
    memset(ss, 0, sizeof(SocialSystem));

    /* Default parameters — tuned to match neuroscience */
    ss->imitation_threshold   = 0.5f;    /* Moderate bar for imitation */
    ss->empathy_level         = 0.6f;    /* Moderate empathy (innate) */
    ss->helping_bonus         = 0.4f;    /* Reward for altruism */
    ss->being_helped_bonus    = 0.3f;    /* Reward for receiving help */
    ss->trust_build_rate      = 0.10f;   /* Slow to build trust */
    ss->trust_decay_rate      = 0.30f;   /* 3× faster to destroy trust */
    ss->familiarity_growth    = 0.05f;   /* Familiarity per interaction */
    ss->belief_decay          = 0.98f;   /* 2% decay per tick */
    ss->social_motivation     = 0.50f;   /* Moderate social drive */
}

/* ════════════════════════════════════════════
 *  Agent Management (TPJ creates a new slot)
 * ════════════════════════════════════════════ */

int social_add_agent(SocialSystem *ss, const char *name) {
    /* Check if already known */
    int existing = social_find_agent(ss, name);
    if (existing >= 0) return existing;

    /* Find empty slot */
    for (int i = 0; i < SOCIAL_MAX_AGENTS; i++) {
        if (!ss->agents[i].active) {
            AgentModel *a = &ss->agents[i];
            memset(a, 0, sizeof(AgentModel));
            strncpy(a->name, name, 31);
            a->name[31] = '\0';
            a->active = 1;
            a->trust = 0.5f;               /* Neutral trust — no bias */
            a->familiarity = 0.0f;          /* Complete stranger */
            a->prediction_accuracy = 0.5f;  /* Chance-level prediction */
            a->emotional_state = 0.0f;      /* Neutral emotion */
            ss->num_agents++;
            return i;
        }
    }
    return -1;  /* No room — social circle is full */
}

int social_find_agent(const SocialSystem *ss, const char *name) {
    for (int i = 0; i < SOCIAL_MAX_AGENTS; i++) {
        if (ss->agents[i].active &&
            strcmp(ss->agents[i].name, name) == 0)
            return i;
    }
    return -1;
}

void social_remove_agent(SocialSystem *ss, int agent_id) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return;
    if (!ss->agents[agent_id].active) return;
    ss->agents[agent_id].active = 0;
    ss->num_agents--;
}

/* ════════════════════════════════════════════
 *  Theory of Mind — Belief Tracking (TPJ)
 * ════════════════════════════════════════════ */

void social_update_belief(SocialSystem *ss, int agent_id,
                          const char *concept, float certainty) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return;
    AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return;

    /* Find existing belief slot */
    int slot = -1;
    for (int i = 0; i < a->num_beliefs; i++) {
        if (a->beliefs[i].active &&
            strcmp(a->beliefs[i].label, concept) == 0) {
            slot = i;
            break;
        }
    }

    if (slot >= 0) {
        /* Update existing — certainty drifts toward observed certainty */
        a->beliefs[slot].certainty +=
            0.2f * (certainty - a->beliefs[slot].certainty);
        a->beliefs[slot].certainty = clampf(a->beliefs[slot].certainty, 0.0f, 1.0f);
    } else if (a->num_beliefs < SOCIAL_MAX_BELIEFS) {
        /* Add new belief */
        slot = a->num_beliefs++;
        memset(&a->beliefs[slot], 0, sizeof(BeliefModel));
        strncpy(a->beliefs[slot].label, concept, 31);
        a->beliefs[slot].label[31] = '\0';
        a->beliefs[slot].certainty = clampf(certainty, 0.0f, 1.0f);
        a->beliefs[slot].active = 1;
    }
}

float social_query_belief(const SocialSystem *ss, int agent_id,
                          const char *concept) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return 0.0f;
    const AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return 0.0f;

    for (int i = 0; i < a->num_beliefs; i++) {
        if (a->beliefs[i].active &&
            strcmp(a->beliefs[i].label, concept) == 0)
            return a->beliefs[i].certainty;
    }
    return 0.0f;  /* We don't think they know this */
}

/* ════════════════════════════════════════════
 *  Mentalizing — Desires and Emotions (mPFC)
 * ════════════════════════════════════════════ */

void social_set_desire(SocialSystem *ss, int agent_id,
                       const float *features, int dim) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return;
    AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return;

    int d = dim < SOCIAL_FEATURE_DIM ? dim : SOCIAL_FEATURE_DIM;
    memcpy(a->desires, features, d * sizeof(float));
}

void social_set_emotion(SocialSystem *ss, int agent_id, float valence) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return;
    AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return;

    a->emotional_state = clampf(valence, -1.0f, 1.0f);
}

/* ════════════════════════════════════════════
 *  Predict Agent's Action (TPJ + mPFC)
 * ════════════════════════════════════════════ */

float social_predict_action(const SocialSystem *ss, int agent_id,
                            const float *action_features, int dim) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return 0.0f;
    const AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return 0.0f;

    /* How aligned is this action with their desires? */
    int d = dim < SOCIAL_FEATURE_DIM ? dim : SOCIAL_FEATURE_DIM;
    float desire_match = cosine_sim(a->desires, action_features, d);

    /* Modulate by our confidence in the model */
    float confidence = a->familiarity * a->prediction_accuracy;

    /* Combined: desire alignment × confidence */
    return clampf(desire_match * (0.3f + 0.7f * confidence), -1.0f, 1.0f);
}

/* ════════════════════════════════════════════
 *  Trust (vmPFC) — Bayesian-like updates
 * ════════════════════════════════════════════ */

void social_cooperate(SocialSystem *ss, int agent_id, float quality) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return;
    AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return;

    float old_trust = a->trust;
    quality = clampf(quality, 0.0f, 1.0f);

    /* Trust grows slowly — (1 - trust) makes it harder to reach 1.0 */
    a->trust += ss->trust_build_rate * (1.0f - a->trust) * quality;
    a->trust = clampf(a->trust, 0.0f, 1.0f);

    /* Update cooperation running average */
    a->cooperation_history = a->cooperation_history * 0.9f + quality * 0.1f;
    a->interaction_count++;

    /* Familiarity grows (diminishing returns) */
    a->familiarity += ss->familiarity_growth * (1.0f - a->familiarity);
    a->familiarity = clampf(a->familiarity, 0.0f, 1.0f);

    /* Record interaction */
    social_record_interaction(ss, a->name, 1, a->trust - old_trust);
}

void social_defect(SocialSystem *ss, int agent_id, float severity) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return;
    AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return;

    float old_trust = a->trust;
    severity = clampf(severity, 0.0f, 1.0f);

    /* Trust drops fast — multiplied by current trust (more to lose) */
    a->trust -= ss->trust_decay_rate * a->trust * severity;
    a->trust = clampf(a->trust, 0.0f, 1.0f);

    /* Cooperation history takes a big hit */
    a->cooperation_history *= 0.7f;
    a->interaction_count++;

    /* Familiarity still grows (we learn from betrayal too) */
    a->familiarity += ss->familiarity_growth * (1.0f - a->familiarity);
    a->familiarity = clampf(a->familiarity, 0.0f, 1.0f);

    /* Record interaction */
    social_record_interaction(ss, a->name, 0, a->trust - old_trust);
}

float social_trust(const SocialSystem *ss, int agent_id) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return 0.0f;
    const AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return 0.0f;
    return a->trust;
}

/* ════════════════════════════════════════════
 *  Mirror Neurons (Premotor F5 + IPL)
 * ════════════════════════════════════════════ */

void social_observe_action(SocialSystem *ss, const char *agent_name,
                           const float *action_features, int dim,
                           const char *action_name) {
    if (ss->mirror_count >= SOCIAL_MIRROR_BUFFER) return;

    MirrorActivation *m = &ss->mirror_buffer[ss->mirror_count];
    memset(m, 0, sizeof(MirrorActivation));

    /* Copy observed features */
    int d = dim < SOCIAL_FEATURE_DIM ? dim : SOCIAL_FEATURE_DIM;
    memcpy(m->features, action_features, d * sizeof(float));
    strncpy(m->agent_name, agent_name, 31);
    m->agent_name[31] = '\0';
    strncpy(m->action_name, action_name, 31);
    m->action_name[31] = '\0';
    m->active = 1;

    /* Compute activation: how novel/interesting is this action?
     * Higher activation for less familiar agents (novelty value) */
    int agent_id = social_find_agent(ss, agent_name);
    if (agent_id >= 0) {
        float fam = ss->agents[agent_id].familiarity;
        float trust = ss->agents[agent_id].trust;
        /* Activation = trust-weighted, familiar actions resonate more */
        m->activation = 0.3f + 0.4f * trust + 0.3f * fam;
    } else {
        /* Unknown agent — moderate activation (curiosity) */
        m->activation = 0.4f;
    }

    m->action_id = -1;  /* Will be resolved when checking imitation */
    ss->mirror_count++;
}

int social_should_imitate(const SocialSystem *ss) {
    for (int i = 0; i < ss->mirror_count; i++) {
        if (ss->mirror_buffer[i].active &&
            ss->mirror_buffer[i].activation >= ss->imitation_threshold)
            return 1;
    }
    return 0;
}

MirrorActivation social_get_strongest_mirror(const SocialSystem *ss) {
    MirrorActivation best;
    memset(&best, 0, sizeof(MirrorActivation));
    float max_act = -1.0f;

    for (int i = 0; i < ss->mirror_count; i++) {
        if (ss->mirror_buffer[i].active &&
            ss->mirror_buffer[i].activation > max_act) {
            max_act = ss->mirror_buffer[i].activation;
            best = ss->mirror_buffer[i];
        }
    }
    return best;
}

void social_clear_mirror(SocialSystem *ss) {
    ss->mirror_count = 0;
    memset(ss->mirror_buffer, 0, sizeof(ss->mirror_buffer));
}

/* ════════════════════════════════════════════
 *  Social Reward (VTA dopamine + anterior insula)
 * ════════════════════════════════════════════ */

float social_compute_reward(SocialSystem *ss, int agent_id,
                            int helped, int was_helped) {
    float reward = 0.0f;

    float trust = 0.5f;
    if (agent_id >= 0 && agent_id < SOCIAL_MAX_AGENTS &&
        ss->agents[agent_id].active) {
        trust = ss->agents[agent_id].trust;
    }

    /* Helping others — empathy-scaled (anterior insula) */
    if (helped)
        reward += ss->helping_bonus * ss->empathy_level;

    /* Being helped — need-based reward */
    if (was_helped)
        reward += ss->being_helped_bonus;

    /* Cooperation bonus — trust-scaled (vmPFC) */
    if (helped || was_helped)
        reward += 0.2f * trust;

    ss->social_reward = reward;
    return reward;
}

/* ════════════════════════════════════════════
 *  Empathy (Anterior Insula Simulation)
 * ════════════════════════════════════════════ */

float social_empathize(const SocialSystem *ss, int agent_id) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return 0.0f;
    const AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return 0.0f;

    /* Empathic response:
     *   their_emotion × empathy_level × (0.3 innate + 0.7 familiarity)
     * Even with strangers, 30% innate empathy (mirror neurons)
     * With close friends, full empathic response */
    return a->emotional_state * ss->empathy_level *
           (0.3f + 0.7f * a->familiarity);
}

/* ════════════════════════════════════════════
 *  Interaction Recording (Ring Buffer)
 * ════════════════════════════════════════════ */

void social_record_interaction(SocialSystem *ss, const char *agent_name,
                               int cooperative, float reward) {
    SocialInteraction *si = &ss->history[ss->history_idx];
    strncpy(si->agent_name, agent_name, 31);
    si->agent_name[31] = '\0';
    si->cooperative = cooperative;
    si->reward = reward;
    si->trust_delta = reward;  /* Overloaded: trust change stored in reward field from callers */
    si->tick = 0;              /* Would need external tick — set by caller if needed */

    ss->history_idx = (ss->history_idx + 1) % SOCIAL_HISTORY_SIZE;
    if (ss->history_count < SOCIAL_HISTORY_SIZE)
        ss->history_count++;
}

/* ════════════════════════════════════════════
 *  Time — Belief Decay (TPJ assumes others forget)
 * ════════════════════════════════════════════ */

void social_tick(SocialSystem *ss) {
    for (int i = 0; i < SOCIAL_MAX_AGENTS; i++) {
        if (!ss->agents[i].active) continue;
        AgentModel *a = &ss->agents[i];

        /* Decay beliefs — we assume others forget over time */
        for (int b = 0; b < a->num_beliefs; b++) {
            if (!a->beliefs[b].active) continue;
            a->beliefs[b].certainty *= ss->belief_decay;
            if (a->beliefs[b].certainty < 0.01f) {
                a->beliefs[b].active = 0;
            }
        }

        /* Familiarity decays very slowly without interaction */
        a->familiarity *= 0.999f;
    }
}

/* ════════════════════════════════════════════
 *  Query / Debug
 * ════════════════════════════════════════════ */

void social_print_agents(const SocialSystem *ss) {
    printf("\n  === Known Agents (%d/%d) ===\n", ss->num_agents, SOCIAL_MAX_AGENTS);
    if (ss->num_agents == 0) {
        printf("  No agents known. Use 'agent <name>' to add one.\n");
        return;
    }

    printf("  %-12s %-8s %-12s %-12s %-6s\n",
           "Name", "Trust", "Familiarity", "Cooperation", "Meets");
    printf("  %-12s %-8s %-12s %-12s %-6s\n",
           "----", "-----", "-----------", "-----------", "-----");

    for (int i = 0; i < SOCIAL_MAX_AGENTS; i++) {
        if (!ss->agents[i].active) continue;
        const AgentModel *a = &ss->agents[i];
        printf("  %-12s %-8.3f %-12.3f %-12.3f %-6d\n",
               a->name, a->trust, a->familiarity,
               a->cooperation_history, a->interaction_count);
    }
}

void social_print_agent(const SocialSystem *ss, int agent_id) {
    if (agent_id < 0 || agent_id >= SOCIAL_MAX_AGENTS) return;
    const AgentModel *a = &ss->agents[agent_id];
    if (!a->active) return;

    printf("\n  === Mental Model: %s ===\n", a->name);
    printf("  Trust:            %.3f", a->trust);
    if (a->trust > 0.7f) printf("  [HIGH — cooperate freely]");
    else if (a->trust < 0.3f) printf("  [LOW — guard information]");
    else printf("  [MEDIUM — cautious]");
    printf("\n");

    printf("  Familiarity:      %.3f", a->familiarity);
    if (a->familiarity > 0.7f) printf("  [close]");
    else if (a->familiarity < 0.2f) printf("  [stranger]");
    printf("\n");

    printf("  Emotional state:  %.3f", a->emotional_state);
    if (a->emotional_state > 0.3f) printf("  [positive]");
    else if (a->emotional_state < -0.3f) printf("  [negative]");
    else printf("  [neutral]");
    printf("\n");

    printf("  Prediction acc:   %.3f\n", a->prediction_accuracy);
    printf("  Cooperation avg:  %.3f\n", a->cooperation_history);
    printf("  Interactions:     %d\n", a->interaction_count);

    /* Print beliefs */
    int active_beliefs = 0;
    for (int i = 0; i < a->num_beliefs; i++) {
        if (a->beliefs[i].active) active_beliefs++;
    }

    printf("  Beliefs (%d):\n", active_beliefs);
    if (active_beliefs == 0) {
        printf("    (none — we don't know what they know)\n");
    } else {
        for (int i = 0; i < a->num_beliefs; i++) {
            if (!a->beliefs[i].active) continue;
            printf("    %-16s certainty=%.2f\n",
                   a->beliefs[i].label, a->beliefs[i].certainty);
        }
    }
}

void social_print_mirror(const SocialSystem *ss) {
    printf("\n  === Mirror Neuron Buffer (%d activations) ===\n", ss->mirror_count);
    if (ss->mirror_count == 0) {
        printf("  No observed actions. Use 'observe <agent> <action>' to watch someone.\n");
        return;
    }

    printf("  %-12s %-16s %-10s %-8s\n",
           "Agent", "Action", "Activation", "Imitate?");
    printf("  %-12s %-16s %-10s %-8s\n",
           "-----", "------", "----------", "--------");

    for (int i = 0; i < ss->mirror_count; i++) {
        if (!ss->mirror_buffer[i].active) continue;
        const MirrorActivation *m = &ss->mirror_buffer[i];
        printf("  %-12s %-16s %-10.3f %-8s\n",
               m->agent_name, m->action_name, m->activation,
               m->activation >= ss->imitation_threshold ? "YES" : "no");
    }

    printf("  Imitation threshold: %.2f\n", ss->imitation_threshold);
}
