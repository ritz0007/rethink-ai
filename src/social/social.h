/*
 * social.h — Social Brain (Theory of Mind, Trust, Imitation)
 *
 * Rethink AI — Phase 14 / V14
 *
 * Brain regions modeled:
 *   - Temporoparietal Junction (TPJ): Theory of Mind — belief tracking
 *   - Medial Prefrontal Cortex (mPFC): Mentalizing — desire/emotion modeling
 *   - Mirror Neuron System (F5 + IPL): Action observation → imitation
 *   - Superior Temporal Sulcus (STS): Social perception
 *   - Anterior Insula + ACC: Empathy via simulation
 *   - Ventromedial PFC (vmPFC): Trust computation, social reward
 *
 * Key principles:
 *   - Separate mental model per known agent (TPJ)
 *   - Trust is slow to build, fast to destroy (negativity bias)
 *   - Mirror neurons map observed actions to motor repertoire
 *   - Empathy = simulate others' emotions through own circuits
 *   - Social reward feeds same dopamine pathway as motor reward
 *   - Familiarity modulates all social computations
 */

#ifndef SOCIAL_H
#define SOCIAL_H

#include <string.h>
#include <math.h>

#define SOCIAL_MAX_AGENTS       8
#define SOCIAL_MAX_BELIEFS      16
#define SOCIAL_FEATURE_DIM      32
#define SOCIAL_MIRROR_BUFFER    16
#define SOCIAL_HISTORY_SIZE     64

/* ──── Belief Model (TPJ) ──── */
typedef struct {
    char  label[32];        /* Concept name (e.g. "cat", "rain") */
    float certainty;        /* How sure we are they know this [0-1] */
    int   active;           /* Slot in use? */
} BeliefModel;

/* ──── Agent Model (TPJ + mPFC + vmPFC) ──── */
typedef struct {
    char  name[32];                             /* Agent identifier */
    int   active;                               /* Slot in use? */

    /* Theory of Mind (TPJ) — what they know */
    BeliefModel beliefs[SOCIAL_MAX_BELIEFS];
    int   num_beliefs;

    /* Mentalizing (mPFC) — what they want / feel */
    float desires[SOCIAL_FEATURE_DIM];          /* Desire feature vector */
    float emotional_state;                      /* Their valence [-1,+1] */

    /* Trust (vmPFC) */
    float trust;                                /* 0=distrust, 1=full trust */
    float cooperation_history;                  /* Running avg of cooperation */
    int   interaction_count;

    /* Familiarity */
    float familiarity;                          /* How well we know them [0-1] */

    /* Prediction accuracy */
    float prediction_accuracy;                  /* How well we predict them [0-1] */
} AgentModel;

/* ──── Mirror Neuron Activation (Premotor F5 + IPL) ──── */
typedef struct {
    int   action_id;                            /* Motor repertoire match (-1 = novel) */
    float activation;                           /* Firing strength [0-1] */
    float features[SOCIAL_FEATURE_DIM];         /* Observed action features */
    char  agent_name[32];                       /* Who performed it */
    char  action_name[32];                      /* What they did */
    int   active;                               /* Slot in use? */
} MirrorActivation;

/* ──── Social Interaction Record ──── */
typedef struct {
    char  agent_name[32];
    int   cooperative;                          /* 1=cooperative, 0=defective */
    float reward;                               /* Reward from interaction */
    float trust_delta;                          /* How trust changed */
    int   tick;                                 /* When it happened */
} SocialInteraction;

/* ──── The Social System ──── */
typedef struct {
    /* Agent models (TPJ + mPFC + vmPFC) */
    AgentModel agents[SOCIAL_MAX_AGENTS];
    int num_agents;

    /* Mirror neuron system (premotor) */
    MirrorActivation mirror_buffer[SOCIAL_MIRROR_BUFFER];
    int mirror_count;
    float imitation_threshold;                  /* Min activation to imitate */

    /* Social reward parameters (VTA / anterior insula) */
    float empathy_level;                        /* 0=low, 1=high (insula) */
    float social_reward;                        /* Current social reward signal */
    float helping_bonus;                        /* Reward for helping */
    float being_helped_bonus;                   /* Reward for being helped */

    /* Interaction history (ring buffer) */
    SocialInteraction history[SOCIAL_HISTORY_SIZE];
    int history_count;
    int history_idx;

    /* Parameters */
    float trust_build_rate;                     /* Alpha: slow build */
    float trust_decay_rate;                     /* Beta: fast erosion (> alpha) */
    float familiarity_growth;                   /* Per-interaction familiarity gain */
    float belief_decay;                         /* Per-tick certainty multiplier */
    float social_motivation;                    /* How much we seek interaction */
} SocialSystem;

/* ──── Lifecycle ──── */
void social_init(SocialSystem *ss);

/* ──── Agent Management (TPJ) ──── */
int  social_add_agent(SocialSystem *ss, const char *name);
int  social_find_agent(const SocialSystem *ss, const char *name);
void social_remove_agent(SocialSystem *ss, int agent_id);

/* ──── Theory of Mind ──── */
void  social_update_belief(SocialSystem *ss, int agent_id,
                           const char *concept, float certainty);
float social_query_belief(const SocialSystem *ss, int agent_id,
                          const char *concept);
void  social_set_desire(SocialSystem *ss, int agent_id,
                        const float *features, int dim);
void  social_set_emotion(SocialSystem *ss, int agent_id, float valence);
float social_predict_action(const SocialSystem *ss, int agent_id,
                            const float *action_features, int dim);

/* ──── Trust (vmPFC) ──── */
void  social_cooperate(SocialSystem *ss, int agent_id, float quality);
void  social_defect(SocialSystem *ss, int agent_id, float severity);
float social_trust(const SocialSystem *ss, int agent_id);

/* ──── Mirror Neurons (Premotor) ──── */
void  social_observe_action(SocialSystem *ss, const char *agent_name,
                            const float *action_features, int dim,
                            const char *action_name);
int   social_should_imitate(const SocialSystem *ss);
MirrorActivation social_get_strongest_mirror(const SocialSystem *ss);
void  social_clear_mirror(SocialSystem *ss);

/* ──── Social Reward ──── */
float social_compute_reward(SocialSystem *ss, int agent_id,
                            int helped, int was_helped);
float social_empathize(const SocialSystem *ss, int agent_id);

/* ──── Interaction Recording ──── */
void social_record_interaction(SocialSystem *ss, const char *agent_name,
                               int cooperative, float reward);

/* ──── Time (decay) ──── */
void social_tick(SocialSystem *ss);

/* ──── Query / Debug ──── */
void social_print_agents(const SocialSystem *ss);
void social_print_agent(const SocialSystem *ss, int agent_id);
void social_print_mirror(const SocialSystem *ss);

#endif /* SOCIAL_H */
