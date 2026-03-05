# Building the Social Brain — Header Design

> Rethink AI — Phase 14, Guide 3 of 5

---

## File Layout

```
src/
  social/
    social.h    ← Structures + API
    social.c    ← Implementation
```

---

## Header Design: `social.h`

### Constants

```c
#define SOCIAL_MAX_AGENTS       8       /* Max agents we can model simultaneously */
#define SOCIAL_MAX_BELIEFS      16      /* Max beliefs tracked per agent */
#define SOCIAL_FEATURE_DIM      32      /* Feature dimension (matches RETHINK_FEATURE_DIM) */
#define SOCIAL_MIRROR_BUFFER    16      /* Mirror neuron activation slots */
#define SOCIAL_HISTORY_SIZE     64      /* Interaction history ring buffer */
```

Why these limits?
- **8 agents**: Matches Dunbar's innermost circle (~5 intimate contacts)
- **16 beliefs per agent**: Enough for meaningful ToM without explosion
- **64 history**: Covers recent social memory

---

### Core Structures

#### BeliefModel — What we think they know

```c
typedef struct {
    char  label[32];      /* What concept (e.g. "cat", "rain") */
    float certainty;      /* How sure we are they know this [0-1] */
    int   active;         /* Is this belief slot in use? */
} BeliefModel;
```

**Brain region**: TPJ — tracks each belief independently
**Biological analog**: Each concept in someone's mental model is a separate track

#### AgentModel — Our full model of another mind

```c
typedef struct {
    char  name[32];                           /* Agent identifier */
    int   active;                             /* Slot in use? */

    /* Theory of Mind (TPJ) */
    BeliefModel beliefs[SOCIAL_MAX_BELIEFS];
    int   num_beliefs;
    float desires[SOCIAL_FEATURE_DIM];        /* What they seem to want */
    float emotional_state;                    /* Their emotional valence [-1,+1] */

    /* Trust (vmPFC) */
    float trust;                              /* 0=distrust, 1=full trust */
    float cooperation_history;                /* Running avg of cooperation */
    int   interaction_count;

    /* Familiarity */
    float familiarity;                        /* How well we know them [0-1] */

    /* Prediction accuracy */
    float prediction_accuracy;                /* How well we predict them [0-1] */
} AgentModel;
```

**Brain regions**: TPJ (beliefs), mPFC (desires/emotions), vmPFC (trust)

#### MirrorActivation — Mirror neuron firing

```c
typedef struct {
    int   action_id;         /* Which motor action this maps to (-1 if novel) */
    float activation;        /* Mirror neuron firing strength [0-1] */
    float features[SOCIAL_FEATURE_DIM]; /* Observed action features */
    char  agent_name[32];    /* Who performed the action */
    int   active;            /* Slot in use? */
} MirrorActivation;
```

**Brain region**: Premotor cortex F5 + inferior parietal lobule

#### SocialInteraction — Record of a social event

```c
typedef struct {
    char  agent_name[32];
    int   cooperative;       /* 1=cooperative, 0=defective */
    float reward;            /* Reward from this interaction */
    float trust_delta;       /* How trust changed */
    int   tick;              /* When it happened */
} SocialInteraction;
```

---

### The Social System

```c
typedef struct {
    /* Agent models (TPJ + mPFC + vmPFC) */
    AgentModel agents[SOCIAL_MAX_AGENTS];
    int num_agents;

    /* Mirror neuron system (premotor) */
    MirrorActivation mirror_buffer[SOCIAL_MIRROR_BUFFER];
    int mirror_count;
    float imitation_threshold;     /* Min activation to trigger imitation */

    /* Social reward parameters */
    float empathy_level;           /* 0=low, 1=high empathy (anterior insula) */
    float social_reward;           /* Current social reward signal */
    float helping_bonus;           /* Reward for helping */
    float being_helped_bonus;      /* Reward for being helped */

    /* Interaction history (ring buffer) */
    SocialInteraction history[SOCIAL_HISTORY_SIZE];
    int history_count;
    int history_idx;

    /* Parameters */
    float trust_build_rate;        /* Alpha: how fast trust grows */
    float trust_decay_rate;        /* Beta: how fast trust erodes (> build) */
    float familiarity_growth;      /* How fast familiarity grows */
    float belief_decay;            /* How fast we assume others forget */
    float social_motivation;       /* How much we seek interaction */
} SocialSystem;
```

---

### API Design

```c
/* ──── Lifecycle ──── */
void social_init(SocialSystem *ss);

/* ──── Agent Management (TPJ) ──── */
int  social_add_agent(SocialSystem *ss, const char *name);
int  social_find_agent(const SocialSystem *ss, const char *name);
void social_remove_agent(SocialSystem *ss, int agent_id);

/* ──── Theory of Mind ──── */
void social_update_belief(SocialSystem *ss, int agent_id,
                          const char *concept, float certainty);
float social_query_belief(const SocialSystem *ss, int agent_id,
                          const char *concept);
void social_set_desire(SocialSystem *ss, int agent_id,
                       const float *features, int dim);
void social_set_emotion(SocialSystem *ss, int agent_id, float valence);
float social_predict_action(const SocialSystem *ss, int agent_id,
                            const float *action_features, int dim);

/* ──── Trust (vmPFC) ──── */
void  social_cooperate(SocialSystem *ss, int agent_id, float quality);
void  social_defect(SocialSystem *ss, int agent_id, float severity);
float social_trust(const SocialSystem *ss, int agent_id);

/* ──── Mirror Neurons (Premotor) ──── */
void social_observe_action(SocialSystem *ss, const char *agent_name,
                           const float *action_features, int dim,
                           const char *action_name);
int  social_should_imitate(const SocialSystem *ss);
MirrorActivation social_get_strongest_mirror(const SocialSystem *ss);

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
```

---

## Connection Diagram

```
  ┌─────────────────────────────────────────────────────────┐
  │                    RETHINK BRAIN                        │
  ├──────────────────────────┬──────────────────────────────┤
  │    Existing Modules      │      V14: Social             │
  │                          │                              │
  │  ┌──── Emotion ──────┐  │  ┌──── SocialSystem ──────┐  │
  │  │ amygdala_evaluate  │◄─┼─►│ social_empathize()     │  │
  │  └───────────────────┘  │  │ (maps their emotion     │  │
  │                         │  │  through our amygdala)   │  │
  │  ┌──── Motor ────────┐  │  │                          │  │
  │  │ motor_add_action   │◄─┼─►│ Mirror neuron imitation │  │
  │  │ motor_find_action  │  │  │ (learn by watching)     │  │
  │  └───────────────────┘  │  │                          │  │
  │                         │  │                          │  │
  │  ┌──── Attention ────┐  │  │ social_compute_reward()  │  │
  │  │ attention_set_goal │◄─┼─►│ (social reward into     │  │
  │  └───────────────────┘  │  │  attention priority)     │  │
  │                         │  └──────────────────────────┘  │
  └──────────────────────────┴──────────────────────────────┘
```

---

*Next: [04_build_social_impl.md](04_build_social_impl.md) — Full implementation walkthrough*
