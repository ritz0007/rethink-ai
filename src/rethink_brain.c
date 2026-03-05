/*
 * rethink_brain.c — The Complete Rethink Brain Implementation
 *
 * Rethink AI — Phase 11 / V11
 */

#include "rethink_brain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ──── Internal logging ──── */
static void brain_log(RethinkBrain *rb, const char *msg) {
    if (rb->log_count < RETHINK_LOG_SIZE) {
        snprintf(rb->log[rb->log_count], 128, "[t=%04d] %s", rb->tick, msg);
        rb->log_count++;
    }
}

/* ──── Create / Destroy ──── */

RethinkBrain *rethink_create(void) {
    RethinkBrain *rb = (RethinkBrain *)calloc(1, sizeof(RethinkBrain));
    
    rb->state = BRAIN_STATE_IDLE;
    rb->tick = 0;
    rb->attention_level = 0.5f;
    
    /* Initialize modules */
    retina_init(&rb->retina, 16);  /* 16x16 retina */
    auditory_init(&rb->auditory);  /* V11: cochlea */
    tactile_init(&rb->tactile);    /* V11: somatosensory */
    multimodal_init(&rb->multimodal); /* V11: multi-modal binding */
    
    rb->memory = hopfield_create(RETHINK_FEATURE_DIM);
    rb->decay_mem = decay_create();
    rb->proto = prototype_create(RETHINK_FEATURE_DIM);
    rb->spontaneous = spontaneous_create(32);
    rb->dream = dream_create(rb->decay_mem);
    rb->amygdala = amygdala_create();
    
    int pred_layers[] = {RETHINK_FEATURE_DIM, 16, 8};
    rb->predictor = predictive_create(pred_layers, 3, 0.03f);
    
    rb->causal = causal_create();
    rb->comm = comm_create(RETHINK_FEATURE_DIM);
    
    brain_log(rb, "Rethink Brain initialized — all modules online (V11: sensory expansion)");
    return rb;
}

void rethink_destroy(RethinkBrain *rb) {
    hopfield_destroy(rb->memory);
    decay_destroy(rb->decay_mem);
    prototype_destroy(rb->proto);
    spontaneous_destroy(rb->spontaneous);
    dream_destroy(rb->dream);
    amygdala_destroy(rb->amygdala);
    predictive_destroy(rb->predictor);
    causal_destroy(rb->causal);
    comm_destroy(rb->comm);
    free(rb);
}

/* ──── Core: Experience ──── */

Experience rethink_experience(RethinkBrain *rb, const float *input, int dim,
                               const char *label) {
    rb->state = BRAIN_STATE_PERCEIVING;
    rb->tick++;
    
    Experience exp;
    memset(&exp, 0, sizeof(Experience));
    exp.timestamp = rb->tick;
    if (label) strncpy(exp.description, label, 63);
    
    /* Copy features (truncate/pad to RETHINK_FEATURE_DIM) */
    int copy_dim = dim < RETHINK_FEATURE_DIM ? dim : RETHINK_FEATURE_DIM;
    memcpy(exp.features, input, copy_dim * sizeof(float));
    memcpy(rb->current_features, input, copy_dim * sizeof(float));
    exp.dim = copy_dim;
    
    /* Step 0 (V11): Multi-modal binding — feed visual features into binding
       Auditory and tactile are fed separately via their own sense calls;
       here we always push the visual input if present. */
    multimodal_set_input(&rb->multimodal, MODALITY_VISUAL,
                         exp.features, copy_dim, 0.8f, (float)rb->tick);
    multimodal_bind(&rb->multimodal);
    
    /* If multi-modal binding produced a stronger percept, use it */
    if (rb->multimodal.percept.num_active > 1 &&
        rb->multimodal.percept.binding_strength > 0.3f) {
        int bd = rb->multimodal.percept.feature_dim;
        if (bd > RETHINK_FEATURE_DIM) bd = RETHINK_FEATURE_DIM;
        memcpy(exp.features, rb->multimodal.percept.features, bd * sizeof(float));
        memcpy(rb->current_features, rb->multimodal.percept.features, bd * sizeof(float));
        copy_dim = bd;
        brain_log(rb, "Multi-modal binding active — using fused percept");
    }
    
    /* Step 1: Predictive coding — was this expected? */
    predictive_process(rb->predictor, input, copy_dim);
    exp.surprise = predictive_surprise(rb->predictor);
    exp.understanding = predictive_understanding(rb->predictor);
    
    /* Step 2: Emotional evaluation */
    float pattern[RETHINK_FEATURE_DIM];
    memcpy(pattern, input, copy_dim * sizeof(float));
    EmotionalResponse emo = amygdala_evaluate(rb->amygdala, pattern, copy_dim);
    rb->current_emotion = emo.emotion;
    exp.emotional_valence = emo.valence;
    
    /* Step 3: Attention — more surprising/emotional = more attention */
    rb->attention_level = fminf(1.0f, 
        0.3f + exp.surprise * 0.4f + fabsf(exp.emotional_valence) * 0.3f);
    
    /* Step 4: Classify using prototypes */
    char cat_name[32];
    float confidence;
    int cat_id = prototype_classify(rb->proto, input, copy_dim, cat_name, &confidence);
    exp.category = cat_id;
    
    /* Step 5: Memory — store if important enough */
    float importance = exp.surprise * 0.4f + fabsf(exp.emotional_valence) * 0.3f + 
                       rb->attention_level * 0.3f;
    if (importance > 0.3f) {
        decay_store(rb->decay_mem, pattern, copy_dim, importance);
        
        char log_msg[128];
        snprintf(log_msg, 128, "Stored: '%s' (imp=%.2f, surp=%.2f, emo=%.2f)",
                 label ? label : "unknown", importance, exp.surprise, exp.emotional_valence);
        brain_log(rb, log_msg);
    }
    
    /* Step 6: Update running averages */
    float alpha = 0.1f;
    rb->overall_surprise = rb->overall_surprise * (1 - alpha) + exp.surprise * alpha;
    rb->overall_understanding = rb->overall_understanding * (1 - alpha) + 
                                exp.understanding * alpha;
    
    rb->experiences_count++;
    rb->state = BRAIN_STATE_IDLE;
    return exp;
}

/* ──── Core: Learn ──── */

void rethink_learn(RethinkBrain *rb, const float *features, int dim,
                   const char *category) {
    rb->state = BRAIN_STATE_LEARNING;
    rb->tick++;
    
    int copy_dim = dim < RETHINK_FEATURE_DIM ? dim : RETHINK_FEATURE_DIM;
    prototype_learn(rb->proto, features, copy_dim, category);
    
    /* Also store emotional association */
    amygdala_learn(rb->amygdala, features, copy_dim, EMOTION_FAMILIAR, 0.0f);
    
    char log_msg[128];
    snprintf(log_msg, 128, "Learned category: '%s'", category);
    brain_log(rb, log_msg);
    
    rb->state = BRAIN_STATE_IDLE;
}

/* ──── Core: Classify ──── */

int rethink_classify(RethinkBrain *rb, const float *features, int dim,
                     char *out_category, float *out_confidence) {
    int copy_dim = dim < RETHINK_FEATURE_DIM ? dim : RETHINK_FEATURE_DIM;
    return prototype_classify(rb->proto, features, copy_dim, out_category, out_confidence);
}

/* ──── Core: Explain ──── */

int rethink_explain(RethinkBrain *rb, const char *event,
                    char *explanation, int max_len) {
    int node = causal_find_node(rb->causal, event);
    if (node < 0) {
        snprintf(explanation, max_len, "I don't know about '%s'", event);
        return 0;
    }
    
    int chain[CAUSAL_MAX_CHAIN];
    float strengths[CAUSAL_MAX_CHAIN];
    int len = causal_explain(rb->causal, node, chain, strengths, 5);
    
    if (len == 0) {
        snprintf(explanation, max_len, "'%s' has no known causes", event);
        return 0;
    }
    
    /* Build explanation string */
    int pos = snprintf(explanation, max_len, "'%s' because: ", event);
    for (int i = 0; i < len && pos < max_len - 40; i++) {
        pos += snprintf(explanation + pos, max_len - pos, "%s(%.0f%%)",
                        rb->causal->nodes[chain[i]].name, 
                        strengths[i] * 100);
        if (i < len - 1) pos += snprintf(explanation + pos, max_len - pos, " <- ");
    }
    
    return len;
}

/* ──── Core: Predict ──── */

int rethink_predict(RethinkBrain *rb, const char *cause,
                    char *prediction, int max_len) {
    int node = causal_find_node(rb->causal, cause);
    if (node < 0) {
        snprintf(prediction, max_len, "I don't know about '%s'", cause);
        return 0;
    }
    
    causal_reset(rb->causal);
    causal_predict(rb->causal, node);
    
    int pos = snprintf(prediction, max_len, "'%s' will cause: ", cause);
    int count = 0;
    for (int i = 0; i < rb->causal->num_nodes && pos < max_len - 40; i++) {
        if (i != node && rb->causal->nodes[i].activation > 0.1f) {
            if (count > 0) pos += snprintf(prediction + pos, max_len - pos, ", ");
            pos += snprintf(prediction + pos, max_len - pos, "%s(%.0f%%)",
                           rb->causal->nodes[i].name,
                           rb->causal->nodes[i].activation * 100);
            count++;
        }
    }
    
    return count;
}

/* ──── Core: Speak ──── */

void rethink_speak(RethinkBrain *rb, char *output, int max_len) {
    rb->state = BRAIN_STATE_COMMUNICATING;
    
    InternalState istate;
    memset(&istate, 0, sizeof(InternalState));
    int dim = RETHINK_FEATURE_DIM < COMM_MAX_FEATURES ? RETHINK_FEATURE_DIM : COMM_MAX_FEATURES;
    memcpy(istate.features, rb->current_features, dim * sizeof(float));
    istate.feature_dim = dim;
    istate.emotional_valence = (rb->current_emotion == EMOTION_REWARD) ? 0.5f :
                                (rb->current_emotion == EMOTION_FEAR) ? -0.7f : 0.0f;
    
    Message msg = comm_encode(rb->comm, &istate);
    
    int pos = 0;
    for (int i = 0; i < msg.length && pos < max_len - 32; i++) {
        pos += snprintf(output + pos, max_len - pos, "%s",
                       rb->comm->vocabulary[msg.words[i].symbol_id].label);
        if (i < msg.length - 1) pos += snprintf(output + pos, max_len - pos, " ");
    }
    
    rb->state = BRAIN_STATE_IDLE;
}

/* ──── Core: Listen ──── */

void rethink_listen(RethinkBrain *rb, const char **words, int num_words) {
    rb->state = BRAIN_STATE_COMMUNICATING;
    rb->tick++;
    
    InternalState understood = comm_understand(rb->comm, words, num_words);
    
    /* Feed the understood state through the experience pipeline */
    rethink_experience(rb, understood.features, understood.feature_dim, "heard_message");
    
    brain_log(rb, "Processed incoming message");
    rb->state = BRAIN_STATE_IDLE;
}

/* ──── Core: Idle ──── */

void rethink_idle(RethinkBrain *rb, int ticks) {
    rb->state = BRAIN_STATE_THINKING;
    
    for (int t = 0; t < ticks; t++) {
        rb->tick++;
        spontaneous_tick(rb->spontaneous);
        
        /* Check if spontaneous activity produced a thought */
        Thought thought;
        if (spontaneous_capture_thought(rb->spontaneous, &thought)) {
            char log_msg[128];
            snprintf(log_msg, 128, "Spontaneous thought (strength=%.2f)", thought.strength);
            brain_log(rb, log_msg);
        }
    }
    
    rb->state = BRAIN_STATE_IDLE;
}

/* ──── Core: Sleep ──── */

void rethink_sleep(RethinkBrain *rb, int cycles) {
    rb->state = BRAIN_STATE_DREAMING;
    brain_log(rb, "Entering sleep...");
    
    for (int c = 0; c < cycles; c++) {
        rb->tick++;
        dream_nrem_replay(rb->dream);
        dream_rem_recombine(rb->dream);
        
        /* Check for insights */
        Insight insight;
        if (dream_check_insight(rb->dream, &insight)) {
            char log_msg[128];
            snprintf(log_msg, 128, "Dream insight! novelty=%.2f stability=%.2f",
                     insight.novelty, insight.stability);
            brain_log(rb, log_msg);
        }
    }
    
    brain_log(rb, "Waking up — memories consolidated");
    rb->state = BRAIN_STATE_IDLE;
}

/* ──── Core: Tick ──── */

void rethink_tick(RethinkBrain *rb) {
    rb->tick++;
    /* Decay all memories slightly */
    decay_tick(rb->decay_mem, 0.1f);
}

/* ──── Query ──── */

float rethink_understanding(const RethinkBrain *rb) { return rb->overall_understanding; }
float rethink_surprise(const RethinkBrain *rb) { return rb->overall_surprise; }
EmotionType rethink_emotion(const RethinkBrain *rb) { return rb->current_emotion; }
BrainState rethink_state(const RethinkBrain *rb) { return rb->state; }

/* ──── Print ──── */

void rethink_print_status(const RethinkBrain *rb) {
    const char *state_names[] = {"IDLE", "PERCEIVING", "LEARNING", 
                                  "THINKING", "DREAMING", "COMMUNICATING"};
    const char *emo_names[] = {"neutral", "fear", "reward", "surprise", 
                                "familiar", "disgust"};
    
    printf("\n======================================\n");
    printf("       RETHINK BRAIN STATUS\n");
    printf("======================================\n");
    printf(" State:         %-18s\n", state_names[rb->state]);
    printf(" Tick:          %-18d\n", rb->tick);
    printf(" Experiences:   %-18d\n", rb->experiences_count);
    printf(" Attention:     %-18.2f\n", rb->attention_level);
    printf(" Understanding: %-18.3f\n", rb->overall_understanding);
    printf(" Surprise:      %-18.3f\n", rb->overall_surprise);
    printf(" Emotion:       %-18s\n", emo_names[rb->current_emotion]);
    printf(" Memories:      %-18d\n", rb->decay_mem->count);
    printf(" Categories:    %-18d\n", rb->proto->num_categories);
    printf(" Causal nodes:  %-18d\n", rb->causal->num_nodes);
    printf(" Vocabulary:    %-18d\n", rb->comm->vocab_size);
    printf("======================================\n");
}

void rethink_print_log(const RethinkBrain *rb) {
    printf("\n-- Brain Log (%d entries) --\n", rb->log_count);
    int start = rb->log_count > 20 ? rb->log_count - 20 : 0;
    for (int i = start; i < rb->log_count; i++) {
        printf("  %s\n", rb->log[i]);
    }
    printf("-- End Log --\n");
}
