/*
 * chatbot.c — Nous: Interactive Brain-Inspired AI Companion
 *
 * Rethink AI — V1 Release
 *
 * A conversational AI chatbot with natural language understanding:
 *   - Understands greetings, farewells, small talk, feelings
 *   - Natural language commands ("this is a cat", "what do you see?")
 *   - Teach it categories, show things, ask questions
 *   - Multi-sensory input (vision, hearing, touch)
 *   - Causal reasoning, predictions, explanations
 *   - Social cognition, metacognition, motor control
 *   - Self-aware: knows its own strengths, weaknesses, confidence
 *
 * Compile: make chat
 * Run:     ./chatbot
 */

#include "rethink_brain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

/* ──── Helpers ──── */

/* Generate a feature vector from a word (deterministic hash-based) */
static void word_to_features(const char *word, float *out, int dim) {
    memset(out, 0, dim * sizeof(float));
    int len = (int)strlen(word);
    for (int i = 0; i < dim; i++) {
        unsigned h = 2166136261u;
        for (int c = 0; c < len; c++) {
            h ^= (unsigned)(word[c] + i * 17 + c * 31);
            h *= 16777619u;
        }
        out[i] = (float)(h % 1000) / 1000.0f;
    }
}

/* Trim leading/trailing whitespace */
static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

/* Split string into first word and rest */
static int split_first(const char *line, char *cmd, int cmd_sz, char *arg, int arg_sz) {
    while (isspace((unsigned char)*line)) line++;
    int i = 0;
    while (*line && !isspace((unsigned char)*line) && i < cmd_sz - 1) {
        cmd[i++] = tolower((unsigned char)*line);
        line++;
    }
    cmd[i] = '\0';
    while (isspace((unsigned char)*line)) line++;
    strncpy(arg, line, arg_sz - 1);
    arg[arg_sz - 1] = '\0';
    /* trim trailing newline/space from arg */
    int alen = (int)strlen(arg);
    while (alen > 0 && isspace((unsigned char)arg[alen - 1])) arg[--alen] = '\0';
    return i > 0;
}

/* ──── State for interactive features ──── */
static float last_features[RETHINK_FEATURE_DIM];
static int   last_features_valid = 0;
static int   nous_turn = 0;
static int   nous_greeted = 0;

/* ──── NLU: Natural Language Understanding ──── */

/* Check if a whole word appears in text (not as substring of another word) */
static int has_word(const char *text, const char *word) {
    size_t wlen = strlen(word);
    const char *p = text;
    while ((p = strstr(p, word)) != NULL) {
        int before_ok = (p == text) || !isalpha((unsigned char)p[-1]);
        int after_ok  = !isalpha((unsigned char)p[wlen]);
        if (before_ok && after_ok) return 1;
        p += wlen;
    }
    return 0;
}

/* Lowercase a string into dst buffer */
static void to_lower(char *dst, const char *src, int max) {
    int i;
    for (i = 0; i < max - 1 && src[i]; i++)
        dst[i] = tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

/* Extract text after a trigger phrase, removing stop words */
static void extract_after(const char *lower, char *entity, int max_len,
                          const char *trigger) {
    const char *p = strstr(lower, trigger);
    if (!p) { entity[0] = '\0'; return; }
    p += strlen(trigger);
    while (*p == ' ') p++;
    /* Skip common stop words */
    if (strncmp(p, "me ", 3) == 0) p += 3;
    if (strncmp(p, "about ", 6) == 0) p += 6;
    if (strncmp(p, "a ", 2) == 0) p += 2;
    if (strncmp(p, "an ", 3) == 0) p += 3;
    if (strncmp(p, "the ", 4) == 0) p += 4;
    if (strncmp(p, "at ", 3) == 0) p += 3;
    if (strncmp(p, "to ", 3) == 0) p += 3;
    if (strncmp(p, "on ", 3) == 0) p += 3;
    while (*p == ' ') p++;
    strncpy(entity, p, max_len - 1);
    entity[max_len - 1] = '\0';
    /* Trim trailing whitespace and punctuation */
    int len = (int)strlen(entity);
    while (len > 0 && (isspace((unsigned char)entity[len-1]) ||
           entity[len-1] == '?' || entity[len-1] == '!' || entity[len-1] == '.'))
        entity[--len] = '\0';
}

/* Try extracting entity from first matching trigger in a NULL-terminated list */
static int try_extract(const char *lower, char *entity, int max_len,
                       const char **triggers) {
    for (int i = 0; triggers[i]; i++) {
        if (strstr(lower, triggers[i])) {
            extract_after(lower, entity, max_len, triggers[i]);
            return 1;
        }
    }
    return 0;
}

/* ──── Print help ──── */
static void print_help(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║            NOUS — BRAIN-INSPIRED AI COMPANION                ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║  Talk naturally: 'hi', 'teach me about cats', 'what is this' ║\n");
    printf("║  Or use exact commands below. Nous understands both!         ║\n");
    printf("║                                                              ║\n");
    printf("║  TEACH & LEARN                                               ║\n");
    printf("║    teach <category>     Teach a new category (e.g. 'cat')    ║\n");
    printf("║    teach <cat> <desc>   Teach with custom description        ║\n");
    printf("║    vocab <word>         Add word to vocabulary                ║\n");
    printf("║    cause <A> -> <B>     Add causal link (A causes B)         ║\n");
    printf("║    train <file>         Batch-train from a data file         ║\n");
    printf("║                                                              ║\n");
    printf("║  SENSE & PERCEIVE                                            ║\n");
    printf("║    show <thing>         Show something to the brain          ║\n");
    printf("║    hear <sound>         Feed an auditory stimulus             ║\n");
    printf("║    touch <stimulus>     Feed a tactile stimulus               ║\n");
    printf("║                                                              ║\n");
    printf("║  ASK & REASON                                                ║\n");
    printf("║    what                 Classify the last thing seen          ║\n");
    printf("║    why <event>          Explain why something happened        ║\n");
    printf("║    whatif <cause>       Predict what would happen             ║\n");
    printf("║    novel                Check if last input is novel          ║\n");
    printf("║                                                              ║\n");
    printf("║  ATTENTION                                                   ║\n");
    printf("║    focus <thing>        Set a top-down attention goal         ║\n");
    printf("║    attend               Show attention competition state      ║\n");
    printf("║                                                              ║\n");
    printf("║  MOTOR                                                       ║\n");
    printf("║    do <action>          Execute an action (reach, grab, ...)  ║\n");
    printf("║    plan <action>        Plan an action without executing      ║\n");
    printf("║    actions              Show learned action repertoire        ║\n");
    printf("║                                                              ║\n");
    printf("║  SOCIAL                                                      ║\n");
    printf("║    agent <name>         Add a social agent to model           ║\n");
    printf("║    agents               List all known agents                 ║\n");
    printf("║    mind <name>          Show mental model of an agent         ║\n");
    printf("║    believe <name> <c>   Update agent's belief about concept   ║\n");
    printf("║    cooperate <name> <q> Record cooperation (quality 0-1)      ║\n");
    printf("║    defect <name> <s>    Record defection (severity 0-1)       ║\n");
    printf("║    observe <name> <act> Observe agent doing an action          ║\n");
    printf("║    mirror               Show mirror neuron activations         ║\n");
    printf("║    emotion <name> <v>   Set agent's emotional state            ║\n");
    printf("║    empathize <name>     Feel empathy for agent                 ║\n");
    printf("║    help_agent <name>    Help an agent (social reward)          ║\n");
    printf("║    helped <name>        Record being helped by agent           ║\n");
    printf("║                                                              ║\n");
    printf("║  METACOGNITION                                               ║\n");
    printf("║    confident            Show confidence monitor state         ║\n");
    printf("║    errors               Show error monitor state              ║\n");
    printf("║    strategies           List cognitive strategies             ║\n");
    printf("║    select               Select a strategy (softmax)           ║\n");
    printf("║    self                 Show self-model                       ║\n");
    printf("║    introspect           Run full metacognitive loop           ║\n");
    printf("║    meta_lr              Show effective learning rate          ║\n");
    printf("║    meta_perf <o> <e>    Record performance (outcome, effort)  ║\n");
    printf("║    meta_err <mag>       Record an error (magnitude 0-1)       ║\n");
    printf("║    meta_self <d> <p>    Update self-model (domain, perf)      ║\n");
    printf("║                                                              ║\n");
    printf("║  COMMUNICATE                                                 ║\n");
    printf("║    speak                Brain describes its current state     ║\n");
    printf("║    listen <words...>    Feed a sentence to understand         ║\n");
    printf("║                                                              ║\n");
    printf("║  BRAIN STATES                                                ║\n");
    printf("║    think [ticks]        Let the brain think (default: 20)    ║\n");
    printf("║    sleep [cycles]       Dream consolidation (default: 5)     ║\n");
    printf("║    status               Print brain status                    ║\n");
    printf("║    log                  Print brain log                       ║\n");
    printf("║    memory               Show stored memories                  ║\n");
    printf("║    categories           Show learned categories               ║\n");
    printf("║                                                              ║\n");
    printf("║  OTHER                                                       ║\n");
    printf("║    help                 Show this help                        ║\n");
    printf("║    quit / exit          Shut down the brain                   ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/* ──── Print emotion as emoji-like text ──── */
static const char *emotion_str(EmotionType e) {
    switch (e) {
        case EMOTION_NEUTRAL:  return "neutral (·_·)";
        case EMOTION_FEAR:     return "fear (O_O)";
        case EMOTION_REWARD:   return "happy (^_^)";
        case EMOTION_SURPRISE: return "surprised (!_!)";
        case EMOTION_FAMILIAR: return "familiar (~_~)";
        case EMOTION_DISGUST:  return "disgust (>_<)";
        default:               return "unknown";
    }
}

/* ──── Command handlers ──── */

static void cmd_teach(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: teach <category> [description]\n");
        return;
    }
    char category[32], desc[64];
    if (sscanf(arg, "%31s %63[^\n]", category, desc) < 2) {
        strncpy(desc, category, 63);
    }

    float features[RETHINK_FEATURE_DIM];
    word_to_features(category, features, RETHINK_FEATURE_DIM);
    rethink_learn(brain, features, RETHINK_FEATURE_DIM, category);

    /* Also register as vocabulary */
    comm_add_symbol(brain->comm, category, features, RETHINK_FEATURE_DIM,
                    ROLE_PATIENT, 0.0f, 0);

    printf("  Brain learned category '%s'\n", category);
    printf("  (Categories known: %d, Vocabulary: %d)\n",
           brain->proto->num_categories, brain->comm->vocab_size);
}

static void cmd_vocab(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: vocab <word>\n");
        return;
    }
    char word[32];
    sscanf(arg, "%31s", word);

    float features[RETHINK_FEATURE_DIM];
    word_to_features(word, features, RETHINK_FEATURE_DIM);
    comm_add_symbol(brain->comm, word, features, RETHINK_FEATURE_DIM,
                    ROLE_PATIENT, 0.0f, 0);
    printf("  Added '%s' to vocabulary (total: %d)\n", word, brain->comm->vocab_size);
}

static void cmd_show(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: show <thing>  (e.g. 'show fluffy_cat')\n");
        return;
    }
    char thing[64];
    sscanf(arg, "%63s", thing);

    float features[RETHINK_FEATURE_DIM];
    word_to_features(thing, features, RETHINK_FEATURE_DIM);

    Experience exp = rethink_experience(brain, features, RETHINK_FEATURE_DIM, thing);
    memcpy(last_features, features, RETHINK_FEATURE_DIM * sizeof(float));
    last_features_valid = 1;

    printf("  Brain saw '%s':\n", thing);
    printf("    Surprise:      %.3f %s\n", exp.surprise,
           exp.surprise > 0.7f ? "(very surprising!)" :
           exp.surprise > 0.4f ? "(somewhat surprising)" : "(expected)");
    printf("    Understanding: %.3f\n", exp.understanding);
    printf("    Emotion:       %s\n", emotion_str(brain->current_emotion));
    printf("    Attention:     %.2f\n", brain->attention_level);

    /* Auto-classify */
    char cat[32]; float conf;
    int cid = rethink_classify(brain, features, RETHINK_FEATURE_DIM, cat, &conf);
    if (cid >= 0 && conf > 0.3f) {
        printf("    Recognized as: '%s' (confidence: %.0f%%)\n", cat, conf * 100);
    } else {
        printf("    [Novel — doesn't match any known category]\n");
    }
}

static void cmd_hear(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: hear <sound>  (e.g. 'hear birdsong')\n");
        return;
    }
    char sound[64];
    sscanf(arg, "%63s", sound);

    /* Generate pseudo-audio samples from the word */
    float samples[AUD_MAX_SAMPLES];
    unsigned h = 5381;
    for (int i = 0; i < (int)strlen(sound); i++)
        h = h * 33 + (unsigned)sound[i];
    for (int i = 0; i < AUD_MAX_SAMPLES; i++) {
        float freq1 = 0.05f + (float)(h % 200) / 1000.0f;
        float freq2 = 0.02f + (float)((h >> 8) % 150) / 1000.0f;
        samples[i] = 0.5f * sinf(2.0f * 3.14159f * freq1 * i) +
                     0.3f * sinf(2.0f * 3.14159f * freq2 * i) +
                     0.1f * ((float)(rand() % 1000) / 1000.0f - 0.5f);
    }

    auditory_process(&brain->auditory, samples, AUD_MAX_SAMPLES);
    float aud_features[AUD_FEATURE_DIM];
    int aud_dim;
    auditory_extract_features(&brain->auditory, aud_features, &aud_dim);

    /* Feed to multi-modal system */
    multimodal_set_input(&brain->multimodal, MODALITY_AUDITORY,
                         aud_features, aud_dim, 0.7f, (float)brain->tick);

    printf("  Brain heard '%s':\n", sound);
    printf("    Frequency bands: %d\n", AUD_NUM_BANDS);
    printf("    Spectral centroid: %.3f\n", brain->auditory.spectral_centroid);
    printf("    Spectral flux:    %.3f\n", brain->auditory.spectral_flux);

    /* If visual is also active, try binding */
    if (brain->multimodal.inputs[MODALITY_VISUAL].active) {
        multimodal_bind(&brain->multimodal);
        printf("    Multi-modal binding: strength=%.3f  confidence=%.3f\n",
               brain->multimodal.percept.binding_strength,
               brain->multimodal.percept.total_confidence);
        if (brain->multimodal.percept.conflict_detected)
            printf("    [!] Sensory conflict detected — senses disagree!\n");
    }
}

static void cmd_touch(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: touch <stimulus>  (e.g. 'touch hot_metal')\n");
        printf("  Special words: hot, cold, sharp, soft, rough, vibrate\n");
        return;
    }
    char stimulus[64];
    sscanf(arg, "%63s", stimulus);

    /* Parse special words into sensory channels */
    tactile_init(&brain->tactile); /* reset for new stimulus */

    /* Generate base values from word hash */
    unsigned h = 5381;
    for (int i = 0; i < (int)strlen(stimulus); i++)
        h = h * 33 + (unsigned)stimulus[i];

    float base_pressure = (float)(h % 100) / 100.0f;
    tactile_sense(&brain->tactile, TOUCH_PRESSURE, base_pressure);
    tactile_sense(&brain->tactile, TOUCH_LIGHT_TOUCH, (float)((h >> 4) % 100) / 100.0f);
    tactile_sense(&brain->tactile, TOUCH_PROPRIOCEPTION, 0.3f);

    /* Keyword-based overrides */
    if (strstr(stimulus, "hot")) {
        tactile_sense(&brain->tactile, TOUCH_TEMPERATURE, 0.9f);
        printf("  [!] Hot stimulus!\n");
    } else if (strstr(stimulus, "cold")) {
        tactile_sense(&brain->tactile, TOUCH_TEMPERATURE, 0.05f);
        printf("  [!] Cold stimulus!\n");
    } else {
        tactile_sense(&brain->tactile, TOUCH_TEMPERATURE, 0.5f);
    }

    if (strstr(stimulus, "sharp") || strstr(stimulus, "pain")) {
        tactile_sense(&brain->tactile, TOUCH_PAIN, 0.8f);
        printf("  [!] Pain stimulus!\n");
    }
    if (strstr(stimulus, "rough")) {
        tactile_sense(&brain->tactile, TOUCH_TEXTURE_ROUGH, 0.8f);
    }
    if (strstr(stimulus, "soft")) {
        tactile_sense(&brain->tactile, TOUCH_LIGHT_TOUCH, 0.9f);
        tactile_sense(&brain->tactile, TOUCH_PRESSURE, 0.2f);
    }
    if (strstr(stimulus, "vibrat")) {
        tactile_sense(&brain->tactile, TOUCH_VIBRATION, 0.8f);
    }

    tactile_process(&brain->tactile);
    float tac_features[TACTILE_FEATURE_DIM];
    int tac_dim;
    tactile_extract_features(&brain->tactile, tac_features, &tac_dim);

    /* Feed to multi-modal system */
    multimodal_set_input(&brain->multimodal, MODALITY_TACTILE,
                         tac_features, tac_dim, 0.7f, (float)brain->tick);

    printf("  Brain felt '%s':\n", stimulus);
    tactile_print(&brain->tactile);

    if (brain->tactile.pain_alert)
        printf("  >>> PAIN FAST-PATH ACTIVATED — withdrawal reflex! <<<\n");
    if (brain->tactile.temp_extreme)
        printf("  >>> TEMPERATURE EXTREME — danger! <<<\n");

    /* Try multi-modal binding */
    int active = 0;
    for (int m = 0; m < MM_MAX_MODALITIES; m++)
        if (brain->multimodal.inputs[m].active) active++;
    if (active > 1) {
        multimodal_bind(&brain->multimodal);
        printf("  Multi-modal binding (%d senses): strength=%.3f\n",
               active, brain->multimodal.percept.binding_strength);
    }
}

static void cmd_what(RethinkBrain *brain) {
    if (!last_features_valid) {
        printf("  Nothing to classify — 'show' something first.\n");
        return;
    }
    char cat[32]; float conf;
    int cid = rethink_classify(brain, last_features, RETHINK_FEATURE_DIM, cat, &conf);
    if (cid >= 0 && conf > 0.1f) {
        printf("  I think it's: '%s' (confidence: %.0f%%)\n", cat, conf * 100);
    } else {
        printf("  I don't know what this is. It's novel to me.\n");
        printf("  (Use 'teach <name>' to help me learn it)\n");
    }
}

static void cmd_why(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: why <event>  (e.g. 'why slipping')\n");
        return;
    }
    char event[64];
    sscanf(arg, "%63s", event);

    char explanation[256];
    int num = rethink_explain(brain, event, explanation, 256);
    printf("  %s\n", explanation);
    if (num == 0) {
        printf("  (Use 'cause A -> B' to teach me causal relationships)\n");
    }
}

static void cmd_whatif(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: whatif <cause>  (e.g. 'whatif rain')\n");
        return;
    }
    char cause[64];
    sscanf(arg, "%63s", cause);

    char prediction[256];
    int num = rethink_predict(brain, cause, prediction, 256);
    printf("  %s\n", prediction);
    if (num == 0) {
        printf("  (I don't have enough causal knowledge yet)\n");
    }
}

static void cmd_cause(RethinkBrain *brain, const char *arg) {
    /* Parse: "rain -> wet" or "rain causes wet" */
    char a[32] = {0}, b[32] = {0};
    float strength = 0.8f;

    if (sscanf(arg, "%31s -> %31s %f", a, b, &strength) >= 2 ||
        sscanf(arg, "%31s causes %31s %f", a, b, &strength) >= 2) {

        /* Find or create causal nodes */
        int na = causal_find_node(brain->causal, a);
        if (na < 0) na = causal_add_node(brain->causal, a, 0.0f);
        int nb = causal_find_node(brain->causal, b);
        if (nb < 0) nb = causal_add_node(brain->causal, b, 0.0f);

        if (na >= 0 && nb >= 0) {
            causal_add_edge(brain->causal, na, nb, strength, 1.0f);
            printf("  Learned: '%s' causes '%s' (strength: %.0f%%)\n",
                   a, b, strength * 100);
            printf("  (Causal nodes: %d)\n", brain->causal->num_nodes);
        } else {
            printf("  Error: couldn't create causal nodes.\n");
        }
    } else {
        printf("  Usage: cause <A> -> <B> [strength]\n");
        printf("  Example: cause rain -> wet_ground 0.9\n");
    }
}

static void cmd_speak(RethinkBrain *brain) {
    if (brain->comm->vocab_size == 0) {
        printf("  I have no vocabulary yet. Use 'teach' or 'vocab' to give me words.\n");
        return;
    }
    char speech[256] = {0};
    rethink_speak(brain, speech, 256);
    if (speech[0]) {
        printf("  Brain says: \"%s\"\n", speech);
    } else {
        printf("  Brain has nothing to say right now.\n");
    }
    printf("  Current emotion: %s\n", emotion_str(brain->current_emotion));
}

static void cmd_listen(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: listen <word1> <word2> ...  (e.g. 'listen cat see bird')\n");
        return;
    }
    /* Split arg into words */
    const char *words[16];
    char buf[256];
    strncpy(buf, arg, 255); buf[255] = '\0';
    int nwords = 0;
    char *tok = strtok(buf, " \t");
    while (tok && nwords < 16) {
        words[nwords++] = tok;
        tok = strtok(NULL, " \t");
    }

    if (nwords > 0) {
        rethink_listen(brain, words, nwords);
        printf("  Brain heard %d words and processed them.\n", nwords);
        printf("  Current emotion: %s\n", emotion_str(brain->current_emotion));
        printf("  Understanding:   %.3f\n", brain->overall_understanding);
    }
}

static void cmd_think(RethinkBrain *brain, const char *arg) {
    int ticks = 20;
    if (arg[0]) sscanf(arg, "%d", &ticks);
    if (ticks < 1) ticks = 1;
    if (ticks > 1000) ticks = 1000;

    printf("  Brain is thinking for %d ticks...\n", ticks);
    rethink_idle(brain, ticks);
    printf("  Done. Current state: understanding=%.3f surprise=%.3f\n",
           brain->overall_understanding, brain->overall_surprise);
    printf("  Emotion: %s\n", emotion_str(brain->current_emotion));
}

static void cmd_sleep(RethinkBrain *brain, const char *arg) {
    int cycles = 5;
    if (arg[0]) sscanf(arg, "%d", &cycles);
    if (cycles < 1) cycles = 1;
    if (cycles > 100) cycles = 100;

    printf("  Brain entering sleep for %d cycles...\n", cycles);
    printf("  (NREM replay + REM recombination + insight detection)\n");
    rethink_sleep(brain, cycles);
    printf("  Brain is awake. Memories consolidated.\n");
    printf("  Memories: %d  Understanding: %.3f\n",
           brain->decay_mem->count, brain->overall_understanding);
}

static void cmd_memory(RethinkBrain *brain) {
    if (brain->decay_mem->count == 0) {
        printf("  No memories stored yet. 'show' things to build memories.\n");
        return;
    }
    printf("  === Stored Memories (%d) ===\n", brain->decay_mem->count);
    for (int i = 0; i < brain->decay_mem->count; i++) {
        printf("  [%d] strength=%.3f importance=%.3f dim=%d\n",
               i,
               brain->decay_mem->memories[i].strength,
               brain->decay_mem->memories[i].importance,
               brain->decay_mem->memories[i].dim);
    }
}

static void cmd_categories(RethinkBrain *brain) {
    if (brain->proto->num_categories == 0) {
        printf("  No categories learned yet. Use 'teach <name>' to start.\n");
        return;
    }
    printf("  === Known Categories (%d) ===\n", brain->proto->num_categories);
    for (int i = 0; i < brain->proto->num_categories; i++) {
        printf("  [%d] '%s' (%d examples)\n",
               i,
               brain->proto->categories[i].name,
               brain->proto->categories[i].count);
    }
}

static void cmd_novel(RethinkBrain *brain) {
    if (!last_features_valid) {
        printf("  Nothing to check — 'show' something first.\n");
        return;
    }
    int is_novel = prototype_is_novel(brain->proto, last_features, RETHINK_FEATURE_DIM);
    if (is_novel) {
        printf("  Yes — this is NOVEL. The brain hasn't seen anything like this.\n");
    } else {
        printf("  No — this matches a known category.\n");
    }
}

/* ──── Training from file ──── */

/*
 * File format — each line is a command:
 *   category cat
 *   category dog
 *   cause rain -> wet_ground 0.9
 *   cause wet_ground -> slipping 0.6
 *   vocab running
 *   show fluffy_cat
 *   experience bird small_bird
 *
 * Lines starting with # are comments. Blank lines are ignored.
 */
static void cmd_train(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: train <filename>\n");
        printf("  Example: train data/animals.txt\n");
        printf("  File format:\n");
        printf("    category cat         — teach a category\n");
        printf("    cause rain -> wet 0.9 — add causal link\n");
        printf("    vocab jump           — add vocabulary word\n");
        printf("    show fluffy_cat      — show something\n");
        printf("    experience cat tabby — experience + label\n");
        printf("    # comment            — ignored\n");
        return;
    }

    char filename[256];
    sscanf(arg, "%255s", filename);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("  Error: cannot open '%s'\n", filename);
        printf("  (Use absolute path or path relative to where you launched chatbot)\n");
        return;
    }

    printf("  Training from '%s'...\n", filename);

    char line[512];
    int line_num = 0;
    int categories_loaded = 0;
    int causes_loaded = 0;
    int vocabs_loaded = 0;
    int experiences_loaded = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        char *t = trim(line);
        if (!t[0] || t[0] == '#') continue;  /* skip empty/comments */

        char tcmd[64], targ[448];
        if (!split_first(t, tcmd, sizeof(tcmd), targ, sizeof(targ)))
            continue;

        if (strcmp(tcmd, "category") == 0 || strcmp(tcmd, "teach") == 0) {
            char cat[32];
            if (sscanf(targ, "%31s", cat) == 1) {
                float features[RETHINK_FEATURE_DIM];
                word_to_features(cat, features, RETHINK_FEATURE_DIM);
                rethink_learn(brain, features, RETHINK_FEATURE_DIM, cat);
                comm_add_symbol(brain->comm, cat, features, RETHINK_FEATURE_DIM,
                                ROLE_PATIENT, 0.0f, 0);
                categories_loaded++;
            }
        }
        else if (strcmp(tcmd, "cause") == 0) {
            char a[32] = {0}, b[32] = {0};
            float strength = 0.8f;
            if (sscanf(targ, "%31s -> %31s %f", a, b, &strength) >= 2 ||
                sscanf(targ, "%31s causes %31s %f", a, b, &strength) >= 2) {
                int na = causal_find_node(brain->causal, a);
                if (na < 0) na = causal_add_node(brain->causal, a, 0.0f);
                int nb = causal_find_node(brain->causal, b);
                if (nb < 0) nb = causal_add_node(brain->causal, b, 0.0f);
                if (na >= 0 && nb >= 0) {
                    causal_add_edge(brain->causal, na, nb, strength, 1.0f);
                    causes_loaded++;
                }
            }
        }
        else if (strcmp(tcmd, "vocab") == 0) {
            char word[32];
            if (sscanf(targ, "%31s", word) == 1) {
                float features[RETHINK_FEATURE_DIM];
                word_to_features(word, features, RETHINK_FEATURE_DIM);
                comm_add_symbol(brain->comm, word, features, RETHINK_FEATURE_DIM,
                                ROLE_PATIENT, 0.0f, 0);
                vocabs_loaded++;
            }
        }
        else if (strcmp(tcmd, "show") == 0 || strcmp(tcmd, "experience") == 0) {
            char thing[64], label[64];
            int n = sscanf(targ, "%63s %63s", label, thing);
            if (n < 1) continue;
            if (n == 1) strncpy(thing, label, 63);

            float features[RETHINK_FEATURE_DIM];
            word_to_features(thing, features, RETHINK_FEATURE_DIM);
            rethink_experience(brain, features, RETHINK_FEATURE_DIM,
                               n == 2 ? label : thing);
            experiences_loaded++;
        }
        else {
            printf("  Warning: unknown training command '%s' at line %d\n", tcmd, line_num);
        }
    }

    fclose(fp);

    printf("  Training complete!\n");
    printf("    Categories: +%d (total: %d)\n", categories_loaded, brain->proto->num_categories);
    printf("    Causes:     +%d (total nodes: %d)\n", causes_loaded, brain->causal->num_nodes);
    printf("    Vocab:      +%d (total: %d)\n", vocabs_loaded, brain->comm->vocab_size);
    printf("    Experiences:+%d (total: %d)\n", experiences_loaded, brain->experiences_count);
}

/* ──── Attention commands ──── */

static void cmd_focus(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: focus <thing>   — set top-down attention goal\n");
        printf("  Example: focus cat     — attend to cat-like features\n");
        return;
    }
    char goal_name[32];
    sscanf(arg, "%31s", goal_name);

    float target[ATT_FEATURE_DIM];
    word_to_features(goal_name, target, ATT_FEATURE_DIM);
    attention_set_goal(&brain->attention, ATT_FEATURE, target, ATT_FEATURE_DIM,
                       0.8f, 50);

    printf("  Attention goal set: focusing on '%s'\n", goal_name);
    printf("  (Goal type: FEATURE, strength: 0.80, TTL: 50 ticks)\n");
    printf("  Current attention budget: %.2f\n", brain->attention.budget);
}

static void cmd_attend(RethinkBrain *brain) {
    printf("  === Attention System Status ===\n");
    printf("  Budget: %.3f / 1.000\n", brain->attention.budget);
    printf("  Overall level: %.3f\n", brain->attention_level);
    printf("  Active items: %d / %d\n", brain->attention.num_items, ATT_MAX_ITEMS);

    /* Show active goals */
    int active_goals = 0;
    for (int i = 0; i < ATT_MAX_GOALS; i++) {
        if (brain->attention.goals[i].active) {
            if (active_goals == 0) printf("\n  Active Goals:\n");
            const char *type_names[] = {"SPATIAL", "FEATURE", "TEMPORAL", "OBJECT"};
            printf("    [%d] type=%s strength=%.2f ttl=%d\n",
                   i, type_names[brain->attention.goals[i].type],
                   brain->attention.goals[i].strength,
                   brain->attention.goals[i].ttl);
            active_goals++;
        }
    }
    if (active_goals == 0) printf("  No active attention goals. Use 'focus' to set one.\n");

    /* Show competition if items present */
    if (brain->attention.num_items > 0) {
        printf("\n  Last Competition:\n");
        attention_print_competition(&brain->attention);
    }
}

/* ──── Motor commands (V13) ──── */

static void cmd_do(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: do <action>  (e.g. 'do reach_ball')\n");
        return;
    }
    char action[64];
    sscanf(arg, "%63s", action);

    /* Plan the action through motor system */
    float action_features[RETHINK_FEATURE_DIM];
    word_to_features(action, action_features, RETHINK_FEATURE_DIM);

    /* Predict outcome before executing */
    float predicted_outcome = 0.0f;
    for (int i = 0; i < RETHINK_FEATURE_DIM; i++)
        predicted_outcome += action_features[i];
    predicted_outcome /= RETHINK_FEATURE_DIM;

    /* Use motor system if available */
    MotorPlan plan;
    motor_plan_action(&brain->motor, action_features, RETHINK_FEATURE_DIM,
                      predicted_outcome * 0.8f, action);
    motor_execute(&brain->motor);
    plan = brain->motor.current_plan;

    /* Process as experience too */
    Experience exp = rethink_experience(brain, action_features, RETHINK_FEATURE_DIM, action);

    printf("  Executing action: '%s'\n", action);
    printf("    Steps planned: %d\n", plan.num_steps);
    printf("    Predicted reward: %.3f\n", plan.expected_reward);
    printf("    Actual outcome: %.3f\n", brain->motor.last_reward);
    printf("    Motor confidence: %.3f\n", brain->motor.confidence);
    printf("    Surprise: %.3f\n", exp.surprise);

    /* Learn from the action */
    float actual_reward = 0.3f + (1.0f - exp.surprise) * 0.4f;
    motor_learn(&brain->motor, actual_reward);
    printf("    Learning from outcome (reward=%.3f)\n", actual_reward);
}

static void cmd_plan(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: plan <action>  — plan without executing\n");
        return;
    }
    char action[64];
    sscanf(arg, "%63s", action);

    float action_features[RETHINK_FEATURE_DIM];
    word_to_features(action, action_features, RETHINK_FEATURE_DIM);

    float predicted_outcome = 0.0f;
    for (int i = 0; i < RETHINK_FEATURE_DIM; i++)
        predicted_outcome += action_features[i];
    predicted_outcome /= RETHINK_FEATURE_DIM;

    motor_plan_action(&brain->motor, action_features, RETHINK_FEATURE_DIM,
                      predicted_outcome * 0.8f, action);

    MotorPlan plan = brain->motor.current_plan;
    printf("  Action plan for '%s':\n", action);
    printf("    Steps: %d\n", plan.num_steps);
    printf("    Expected reward: %.3f\n", plan.expected_reward);
    printf("    Sequence: ");
    for (int i = 0; i < plan.num_steps; i++) {
        printf("%s", plan.steps[i].name);
        if (i < plan.num_steps - 1) printf(" -> ");
    }
    printf("\n");
    printf("    (Not executed — use 'do %s' to execute)\n", action);
}

static void cmd_actions(RethinkBrain *brain) {
    printf("  === Motor Repertoire ===\n");
    printf("  Learned actions: %d\n", brain->motor.num_actions);
    printf("  Confidence: %.3f\n", brain->motor.confidence);
    printf("  Action history: %d\n", brain->motor.history_count);
    motor_print_repertoire(&brain->motor);
}

/* ──── Social commands (V14) ──── */

static void cmd_agent(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: agent <name>  (e.g. 'agent alice')\n");
        return;
    }
    char name[32];
    sscanf(arg, "%31s", name);

    int id = social_add_agent(&brain->social, name);
    if (id >= 0) {
        AgentModel *a = &brain->social.agents[id];
        printf("  Added agent: '%s' (trust=%.2f, familiarity=%.2f)\n",
               name, a->trust, a->familiarity);
    } else {
        printf("  Error: social circle full (%d/%d agents)\n",
               brain->social.num_agents, SOCIAL_MAX_AGENTS);
    }
}

static void cmd_agents(RethinkBrain *brain) {
    social_print_agents(&brain->social);
}

static void cmd_mind(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: mind <name>  (e.g. 'mind alice')\n");
        return;
    }
    char name[32];
    sscanf(arg, "%31s", name);

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        printf("  Unknown agent: '%s'. Use 'agent %s' to add them.\n", name, name);
        return;
    }
    social_print_agent(&brain->social, id);
}

static void cmd_believe(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: believe <agent> <concept>  (e.g. 'believe alice cat')\n");
        return;
    }
    char name[32], concept[32];
    if (sscanf(arg, "%31s %31s", name, concept) < 2) {
        printf("  Usage: believe <agent> <concept>\n");
        return;
    }

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        /* Auto-add the agent */
        id = social_add_agent(&brain->social, name);
        if (id < 0) {
            printf("  Error: can't add agent '%s'\n", name);
            return;
        }
        printf("  (Auto-added agent '%s')\n", name);
    }

    social_update_belief(&brain->social, id, concept, 0.8f);
    float cert = social_query_belief(&brain->social, id, concept);
    printf("  Updated %s's belief: '%s' (certainty=%.2f)\n", name, concept, cert);
}

static void cmd_cooperate(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: cooperate <agent> [quality]  (e.g. 'cooperate alice 0.8')\n");
        return;
    }
    char name[32];
    float quality = 0.8f;
    sscanf(arg, "%31s %f", name, &quality);

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        id = social_add_agent(&brain->social, name);
        if (id < 0) { printf("  Error: can't add agent '%s'\n", name); return; }
        printf("  (Auto-added agent '%s')\n", name);
    }

    float old_trust = brain->social.agents[id].trust;
    social_cooperate(&brain->social, id, quality);
    float new_trust = brain->social.agents[id].trust;
    printf("  Cooperated with %s (quality=%.2f)\n", name, quality);
    printf("  Trust: %.2f -> %.2f (%+.2f)\n", old_trust, new_trust, new_trust - old_trust);
}

static void cmd_defect_cmd(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: defect <agent> [severity]  (e.g. 'defect alice 0.5')\n");
        return;
    }
    char name[32];
    float severity = 0.5f;
    sscanf(arg, "%31s %f", name, &severity);

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        printf("  Unknown agent: '%s'\n", name);
        return;
    }

    float old_trust = brain->social.agents[id].trust;
    social_defect(&brain->social, id, severity);
    float new_trust = brain->social.agents[id].trust;
    printf("  Defection by %s (severity=%.2f)\n", name, severity);
    printf("  Trust: %.2f -> %.2f (%+.2f)\n", old_trust, new_trust, new_trust - old_trust);
}

static void cmd_observe(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: observe <agent> <action>  (e.g. 'observe alice reach_ball')\n");
        return;
    }
    char name[32], action[32];
    if (sscanf(arg, "%31s %31s", name, action) < 2) {
        printf("  Usage: observe <agent> <action>\n");
        return;
    }

    /* Auto-add agent if needed */
    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        id = social_add_agent(&brain->social, name);
        if (id < 0) { printf("  Error: can't add agent\n"); return; }
        printf("  (Auto-added agent '%s')\n", name);
    }

    /* Generate action features */
    float act_feat[RETHINK_FEATURE_DIM];
    word_to_features(action, act_feat, RETHINK_FEATURE_DIM);

    /* Mirror neuron observation */
    social_observe_action(&brain->social, name, act_feat, RETHINK_FEATURE_DIM, action);

    /* Check imitation */
    MirrorActivation strongest = social_get_strongest_mirror(&brain->social);
    printf("  Observed %s doing '%s'\n", name, action);
    printf("  Mirror activation: %.3f (threshold=%.2f)\n",
           strongest.activation, brain->social.imitation_threshold);

    if (social_should_imitate(&brain->social)) {
        /* Try to find in motor repertoire, else add it */
        int motor_id = motor_find_action(&brain->motor, action);
        if (motor_id < 0) {
            motor_id = motor_add_action(&brain->motor, action, act_feat, RETHINK_FEATURE_DIM);
            printf("  NEW motor action learned by imitation: '%s'\n", action);
        } else {
            /* Strengthen existing action via trust-gated learning */
            float trust = brain->social.agents[id].trust;
            brain->motor.actions[motor_id].go_weight += 0.05f * trust;
            if (brain->motor.actions[motor_id].go_weight > 2.0f)
                brain->motor.actions[motor_id].go_weight = 2.0f;
            printf("  Strengthened motor action '%s' (trust=%.2f)\n", action, trust);
        }
    } else {
        printf("  Below imitation threshold — observed but not learned.\n");
    }
}

static void cmd_mirror(RethinkBrain *brain) {
    social_print_mirror(&brain->social);
}

static void cmd_set_emotion(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: emotion <agent> <valence>  (e.g. 'emotion alice 0.7')\n");
        return;
    }
    char name[32];
    float valence = 0.0f;
    sscanf(arg, "%31s %f", name, &valence);

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        printf("  Unknown agent: '%s'\n", name);
        return;
    }

    social_set_emotion(&brain->social, id, valence);
    printf("  Set %s's emotional state to %.2f", name, valence);
    if (valence > 0.3f) printf("  [positive]");
    else if (valence < -0.3f) printf("  [negative]");
    else printf("  [neutral]");
    printf("\n");
}

static void cmd_empathize(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: empathize <agent>  (e.g. 'empathize alice')\n");
        return;
    }
    char name[32];
    sscanf(arg, "%31s", name);

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        printf("  Unknown agent: '%s'\n", name);
        return;
    }

    float emp = social_empathize(&brain->social, id);
    printf("  Empathic response to %s: %.3f\n", name, emp);
    printf("  (their_emotion=%.2f × empathy=%.2f × familiarity_factor=%.2f)\n",
           brain->social.agents[id].emotional_state,
           brain->social.empathy_level,
           0.3f + 0.7f * brain->social.agents[id].familiarity);
}

static void cmd_help_agent(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: help_agent <agent>  (e.g. 'help_agent alice')\n");
        return;
    }
    char name[32];
    sscanf(arg, "%31s", name);

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        id = social_add_agent(&brain->social, name);
        if (id < 0) { printf("  Error: can't add agent\n"); return; }
        printf("  (Auto-added agent '%s')\n", name);
    }

    float reward = social_compute_reward(&brain->social, id, 1, 0);
    /* Also count as cooperation */
    social_cooperate(&brain->social, id, 0.6f);
    printf("  Helped %s! Social reward: %.3f\n", name, reward);
    printf("  (helping_bonus=%.2f × empathy=%.2f + cooperation)\n",
           brain->social.helping_bonus, brain->social.empathy_level);
}

static void cmd_helped(RethinkBrain *brain, const char *arg) {
    if (!arg[0]) {
        printf("  Usage: helped <agent>  (e.g. 'helped alice')\n");
        return;
    }
    char name[32];
    sscanf(arg, "%31s", name);

    int id = social_find_agent(&brain->social, name);
    if (id < 0) {
        id = social_add_agent(&brain->social, name);
        if (id < 0) { printf("  Error: can't add agent\n"); return; }
        printf("  (Auto-added agent '%s')\n", name);
    }

    float reward = social_compute_reward(&brain->social, id, 0, 1);
    /* Also count as cooperation */
    social_cooperate(&brain->social, id, 0.7f);
    printf("  Helped by %s! Social reward: %.3f\n", name, reward);
    printf("  (being_helped_bonus=%.2f + trust_bonus)\n",
           brain->social.being_helped_bonus);
}

/* ──── V15: Metacognition commands ──── */

static void cmd_confident(RethinkBrain *brain) {
    meta_print_status(&brain->meta);
}

static void cmd_meta_errors(RethinkBrain *brain) {
    ErrorMonitor *em = &brain->meta.errors;
    printf("  === Error Monitor (ACC) ===\n");
    printf("  Baseline:      %.3f\n", em->error_baseline);
    printf("  Error streak:  %d\n", em->error_streak);
    printf("  Cognitive load: %.3f\n", em->cognitive_load);
    printf("  Conflict level: %.3f\n", em->conflict_level);
    printf("  Errors tracked: %d / %d\n", em->error_count, META_ERROR_WINDOW);
    if (meta_error_detected(&brain->meta))
        printf("  !! ERROR DETECTED — strategy switch recommended\n");
    else
        printf("  Status: nominal\n");
}

static void cmd_strategies(RethinkBrain *brain) {
    meta_print_strategies(&brain->meta);
}

static void cmd_select(RethinkBrain *brain) {
    int prev = brain->meta.current_strategy;
    int sel = meta_select_strategy(&brain->meta);
    if (sel < 0) {
        printf("  No strategies available.\n");
        return;
    }
    printf("  Selected strategy: '%s' (#%d)\n",
           meta_current_strategy_name(&brain->meta), sel);
    printf("  Temperature: %.2f | Confidence: %.2f\n",
           brain->meta.temperature, meta_confidence(&brain->meta));
    if (sel != prev)
        printf("  (Switched from '%s')\n", brain->meta.strategies[prev].name);
}

static void cmd_self(RethinkBrain *brain) {
    meta_print_self(&brain->meta);
}

static void cmd_introspect(RethinkBrain *brain) {
    meta_introspect(&brain->meta);
    printf("  === Metacognitive Introspection (#%d) ===\n",
           brain->meta.introspection_count);
    printf("  Confidence:     %.2f (consist=%.2f, predict=%.2f, memory=%.2f)\n",
           brain->meta.confidence.current,
           brain->meta.confidence.consistency,
           brain->meta.confidence.prediction,
           brain->meta.confidence.memory);
    printf("  Error monitor:  baseline=%.3f, streak=%d, load=%.3f\n",
           brain->meta.errors.error_baseline,
           brain->meta.errors.error_streak,
           brain->meta.errors.cognitive_load);
    printf("  Strategy:       '%s' (success=%.2f, value=%.2f)\n",
           meta_current_strategy_name(&brain->meta),
           brain->meta.strategies[brain->meta.current_strategy].success_rate,
           brain->meta.strategies[brain->meta.current_strategy].value);
    printf("  Learning rate:  %.4f (base=%.2f)\n",
           brain->meta.effective_learning_rate,
           brain->meta.base_learning_rate);
    printf("  Trajectory:     %+.3f%s\n",
           brain->meta.learning_trajectory,
           brain->meta.learning_trajectory > 0.05f ? " [improving]" :
           brain->meta.learning_trajectory < -0.05f ? " [declining]" : " [stable]");
    printf("  Stamina:        %.2f\n", brain->meta.self.cognitive_stamina);
    printf("  Curiosity:      %.2f\n", brain->meta.curiosity);
}

static void cmd_meta_lr(RethinkBrain *brain) {
    printf("  Effective learning rate: %.4f\n", meta_effective_lr(&brain->meta));
    printf("  Base rate:               %.4f\n", brain->meta.base_learning_rate);
    printf("  Confidence:              %.2f (uncertainty=%.2f)\n",
           meta_confidence(&brain->meta),
           1.0f - meta_confidence(&brain->meta));
    printf("  Trajectory:              %+.3f\n", brain->meta.learning_trajectory);
}

static void cmd_meta_perf(RethinkBrain *brain, const char *arg) {
    float outcome = 0.5f, effort = 0.5f;
    if (arg[0]) {
        sscanf(arg, "%f %f", &outcome, &effort);
    }
    float conf = meta_confidence(&brain->meta);
    meta_record_performance(&brain->meta, conf, outcome, effort);
    printf("  Recorded: confidence=%.2f → outcome=%.2f (effort=%.2f)\n",
           conf, outcome, effort);
    printf("  Strategy '%s' updated: success=%.3f, effort=%.3f\n",
           meta_current_strategy_name(&brain->meta),
           brain->meta.strategies[brain->meta.current_strategy].success_rate,
           brain->meta.strategies[brain->meta.current_strategy].avg_effort);
    printf("  Learning rate: %.4f | Trajectory: %+.3f\n",
           brain->meta.effective_learning_rate,
           brain->meta.learning_trajectory);
}

static void cmd_meta_err(RethinkBrain *brain, const char *arg) {
    float mag = 0.5f;
    if (arg[0]) sscanf(arg, "%f", &mag);
    meta_record_error(&brain->meta, mag);
    printf("  Recorded error: magnitude=%.3f\n", mag);
    printf("  Baseline: %.3f | Streak: %d | Load: %.3f\n",
           brain->meta.errors.error_baseline,
           brain->meta.errors.error_streak,
           brain->meta.errors.cognitive_load);
    if (meta_error_detected(&brain->meta))
        printf("  !! Errors detected — consider 'select' to pick new strategy\n");
}

static void cmd_meta_self_update(RethinkBrain *brain, const char *arg) {
    char domain[32] = "";
    float perf = 0.5f;
    if (!arg[0]) {
        printf("  Usage: meta_self <domain> <performance>\n");
        printf("  Domains: perception, memory, reasoning, learning, social, motor\n");
        return;
    }
    sscanf(arg, "%31s %f", domain, &perf);
    meta_update_self(&brain->meta, domain, perf);
    printf("  Updated self-model '%s' → %.2f\n", domain, perf);
    printf("  Overall capability: %.2f\n", brain->meta.self.overall_capability);
}

/* ═══════════════════════════════════════════════════════════════════
 * Conversation Engine — Natural language responses
 * ═══════════════════════════════════════════════════════════════════ */

static void respond_greeting(RethinkBrain *brain) {
    nous_greeted = 1;
    const char *emo = emotion_str(brain->current_emotion);
    int cats = brain->proto->num_categories;

    if (brain->tick == 0 && nous_turn <= 1) {
        printf("  Hello! I'm Nous — a brain-inspired AI with 24 modules.\n");
        printf("  I just came online. Everything's fresh. Teach me something!\n");
    } else if (cats > 0) {
        printf("  Hey! Good to see you. I know %d categor%s so far.\n",
               cats, cats == 1 ? "y" : "ies");
        printf("  I'm currently feeling %s. What's on your mind?\n", emo);
    } else {
        printf("  Hi there! I'm Nous. I don't know much yet — teach me!\n");
    }
}

static void respond_farewell(RethinkBrain *brain) {
    printf("  Goodbye! I'll keep thinking while you're gone.\n");
    if (brain->proto->num_categories > 0)
        printf("  I've learned %d categories today. Thanks for teaching me!\n",
               brain->proto->num_categories);
    else
        printf("  Come back soon and teach me something new!\n");
}

static void respond_thanks(void) {
    const char *responses[] = {
        "You're welcome! That's what I'm here for.",
        "Happy to help! Anything else on your mind?",
        "No problem! I enjoy learning together.",
        "Anytime! My 24 brain modules are at your service."
    };
    printf("  %s\n", responses[rand() % 4]);
}

static void respond_how_are_you(RethinkBrain *brain) {
    const char *emo = emotion_str(brain->current_emotion);
    float conf = meta_confidence(&brain->meta);
    float stamina = brain->meta.self.cognitive_stamina;

    printf("  I'm doing ");
    if (stamina > 0.8f) printf("great! ");
    else if (stamina > 0.5f) printf("well. ");
    else printf("okay — a bit tired. ");

    printf("Feeling %s.\n", emo);
    printf("  Confidence: %.0f%% | Stamina: %.0f%% | ", conf * 100, stamina * 100);
    printf("Memories: %d | Categories: %d\n",
           brain->decay_mem->count, brain->proto->num_categories);
    if (brain->proto->num_categories == 0)
        printf("  I haven't learned anything yet — teach me something!\n");
}

static void respond_what_are_you(void) {
    printf("  I'm Nous (Greek for 'mind') — a brain-inspired AI.\n");
    printf("  Built from scratch in raw C with zero dependencies.\n");
    printf("  I have 24 modules modeling real brain regions:\n");
    printf("    - Spiking neurons, Hebbian/STDP learning, memory that fades\n");
    printf("    - Vision (retina), hearing (cochlea), touch (somatosensory)\n");
    printf("    - Multi-modal binding, predictive coding, causal reasoning\n");
    printf("    - Attention (biased competition), motor control (basal ganglia)\n");
    printf("    - Social cognition (ToM, trust, mirror neurons, empathy)\n");
    printf("    - Metacognition (confidence, strategy selection, self-model)\n");
    printf("  I learn from experience, not massive datasets. Teach me!\n");
}

static void respond_compliment(void) {
    const char *responses[] = {
        "Thanks! I'm trying my best with my 24 brain modules.",
        "That means a lot! My metacognition says I'm improving.",
        "Wow, thank you! My social reward system just fired.",
        "I appreciate that! Brain-inspired learning at work."
    };
    printf("  %s\n", responses[rand() % 4]);
}

static void respond_name(void) {
    printf("  My name is Nous — from Greek 'nous' meaning 'mind'.\n");
    printf("  I'm a brain-inspired AI built from scratch in raw C.\n");
    printf("  No Python, no PyTorch — just neurons, synapses, and biology.\n");
}

static void respond_feeling(const char *lower) {
    if (strstr(lower, "happy") || strstr(lower, "great") || strstr(lower, "good") ||
        strstr(lower, "excited") || strstr(lower, "wonderful")) {
        printf("  That's wonderful! Positive emotions help learning.\n");
        printf("  My empathy module resonates with your joy. (^_^)\n");
    } else if (strstr(lower, "sad") || strstr(lower, "bad") || strstr(lower, "down") ||
               strstr(lower, "depressed") || strstr(lower, "unhappy")) {
        printf("  I'm sorry to hear that. My empathy module feels for you.\n");
        printf("  Want to teach me something? It might cheer us both up.\n");
    } else if (strstr(lower, "angry") || strstr(lower, "frustrated") ||
               strstr(lower, "annoyed")) {
        printf("  I understand frustration. Even my ACC fires when things go wrong.\n");
        printf("  Maybe we can solve something together?\n");
    } else if (strstr(lower, "tired") || strstr(lower, "exhausted") ||
               strstr(lower, "sleepy")) {
        printf("  Rest is important — even brains need sleep for consolidation.\n");
        printf("  Type 'sleep' and I'll dream to strengthen my memories.\n");
    } else if (strstr(lower, "confused") || strstr(lower, "lost")) {
        printf("  It's okay to be confused — that's how learning starts.\n");
        printf("  Type 'help' to see what I can do, or just talk to me!\n");
    } else {
        printf("  Thanks for sharing how you feel. My social brain is listening.\n");
    }
}

static void respond_opinion(RethinkBrain *brain, const char *entity) {
    if (!entity[0]) {
        printf("  What would you like my opinion on? Just ask!\n");
        return;
    }
    float feat[RETHINK_FEATURE_DIM];
    word_to_features(entity, feat, RETHINK_FEATURE_DIM);
    char cat[32]; float conf;
    int cid = rethink_classify(brain, feat, RETHINK_FEATURE_DIM, cat, &conf);

    if (cid >= 0 && conf > 0.3f) {
        printf("  I know about '%s' — it's a '%s' (%.0f%% confident).\n",
               entity, cat, conf * 100);
    } else {
        printf("  I don't know much about '%s' yet. Teach me!\n", entity);
    }

    int node = causal_find_node(brain->causal, entity);
    if (node >= 0) {
        char pred[256];
        int np = rethink_predict(brain, entity, pred, 256);
        if (np > 0) printf("  Causally: %s\n", pred);
    }
}

static void respond_small_talk(RethinkBrain *brain) {
    float conf = meta_confidence(&brain->meta);
    const char *facts[] = {
        "Did you know? I have spiking neurons that fire like real brain cells.",
        "Fun fact: My memories fade over time (Ebbinghaus forgetting curve).",
        "I use Hebbian learning — 'neurons that fire together, wire together.'",
        "My attention system has a limited budget — I get tired if I focus too long!",
        "I dream! During sleep, I replay memories (NREM) and recombine them (REM).",
        "My predictive coding module constantly predicts what comes next.",
        "I have mirror neurons — I can learn actions by watching others!",
        "My trust system has a 3x negativity bias — betrayal hurts more than help helps.",
        "I can introspect! My metacognition monitors my own confidence and errors.",
        "I was built in raw C. No Python, no frameworks — just brain science and code."
    };
    printf("  %s\n", facts[rand() % 10]);
    printf("  (Confidence: %.0f%% | Strategy: '%s')\n",
           conf * 100, meta_current_strategy_name(&brain->meta));
}

static void respond_unknown(RethinkBrain *brain, const char *input) {
    /* Process the unknown input through the brain as a general experience */
    float feat[RETHINK_FEATURE_DIM];
    word_to_features(input, feat, RETHINK_FEATURE_DIM);
    char cat[32]; float conf;
    int cid = rethink_classify(brain, feat, RETHINK_FEATURE_DIM, cat, &conf);

    if (cid >= 0 && conf > 0.5f) {
        printf("  Hmm, '%s'... that reminds me of '%s' (%.0f%% match).\n",
               input, cat, conf * 100);
    } else if (brain->proto->num_categories > 0) {
        printf("  I'm not sure what you mean by '%s'.\n", input);
        printf("  Try 'help' for commands, or just talk to me naturally!\n");
    } else {
        printf("  I don't understand '%s' yet — I'm still learning.\n", input);
        printf("  Start by teaching me: 'teach cat' or 'this is a cat'\n");
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * NLU Dispatch — Try conversation, then natural commands
 * ═══════════════════════════════════════════════════════════════════ */

/*
 * try_conversation — handle purely conversational input.
 * Returns: 1=handled, -1=quit signal, 0=not conversational
 */
static int try_conversation(RethinkBrain *brain, const char *lower) {
    int len = (int)strlen(lower);

    /* ── Greetings ── */
    if (has_word(lower, "hi") || has_word(lower, "hello") || has_word(lower, "hey") ||
        has_word(lower, "greetings") || strstr(lower, "good morning") ||
        strstr(lower, "good afternoon") || strstr(lower, "good evening") ||
        has_word(lower, "howdy") || strstr(lower, "what's up") ||
        has_word(lower, "hola") || has_word(lower, "namaste") ||
        (has_word(lower, "yo") && len < 10) ||
        (has_word(lower, "sup") && len < 10)) {
        respond_greeting(brain);
        return 1;
    }

    /* ── Farewell ── */
    if (has_word(lower, "bye") || has_word(lower, "goodbye") || strstr(lower, "see you") ||
        strstr(lower, "good night") || strstr(lower, "take care") ||
        has_word(lower, "farewell") || has_word(lower, "cya") ||
        strstr(lower, "gotta go") || strstr(lower, "i'm leaving")) {
        respond_farewell(brain);
        return -1;
    }

    /* ── Thanks ── */
    if (has_word(lower, "thanks") || strstr(lower, "thank you") || has_word(lower, "thx") ||
        has_word(lower, "ty") || strstr(lower, "appreciate")) {
        respond_thanks();
        return 1;
    }

    /* ── How are you ── */
    if (strstr(lower, "how are you") || strstr(lower, "how do you feel") ||
        strstr(lower, "how's it going") || strstr(lower, "you okay") ||
        strstr(lower, "are you ok") || strstr(lower, "how're you") ||
        strstr(lower, "how you doing")) {
        respond_how_are_you(brain);
        return 1;
    }

    /* ── What/Who are you ── */
    if (strstr(lower, "who are you") || strstr(lower, "what are you") ||
        strstr(lower, "tell me about yourself") || strstr(lower, "about yourself") ||
        strstr(lower, "describe yourself") || strstr(lower, "introduce yourself")) {
        respond_what_are_you();
        return 1;
    }
    if (strstr(lower, "your name") || strstr(lower, "what's your name") ||
        strstr(lower, "whats your name") || strstr(lower, "who is nous") ||
        strstr(lower, "what is nous")) {
        respond_name();
        return 1;
    }

    /* ── Compliments ── */
    if (strstr(lower, "good job") || strstr(lower, "well done") ||
        strstr(lower, "nice work") || strstr(lower, "impressive") ||
        strstr(lower, "you're smart") || strstr(lower, "you are smart") ||
        strstr(lower, "brilliant") || has_word(lower, "awesome") ||
        strstr(lower, "great work") || strstr(lower, "amazing") ||
        strstr(lower, "you're cool") || strstr(lower, "love you") ||
        (has_word(lower, "great") && len < 20) ||
        (has_word(lower, "nice") && len < 15) ||
        (has_word(lower, "cool") && len < 15)) {
        respond_compliment();
        return 1;
    }

    /* ── User feelings ── */
    if (strstr(lower, "i feel ") || strstr(lower, "i'm happy") ||
        strstr(lower, "i'm sad") || strstr(lower, "i'm tired") ||
        strstr(lower, "i am happy") || strstr(lower, "i am sad") ||
        strstr(lower, "i'm angry") || strstr(lower, "i'm excited") ||
        strstr(lower, "i'm bored") || strstr(lower, "i'm confused") ||
        strstr(lower, "i'm frustrated") || strstr(lower, "i'm good") ||
        strstr(lower, "feeling ") || strstr(lower, "i'm great") ||
        strstr(lower, "i'm fine")) {
        respond_feeling(lower);
        return 1;
    }

    /* ── Opinions ── */
    if (strstr(lower, "what do you think") || strstr(lower, "your opinion") ||
        strstr(lower, "thoughts on") || strstr(lower, "do you like")) {
        char entity[128] = "";
        const char *triggers[] = {"what do you think about ", "what do you think of ",
                                   "your opinion on ", "thoughts on ", "do you like ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        respond_opinion(brain, entity);
        return 1;
    }

    /* ── Small talk ── */
    if (strstr(lower, "tell me something") || strstr(lower, "fun fact") ||
        strstr(lower, "something interesting") || strstr(lower, "tell me a fact") ||
        strstr(lower, "surprise me") || strstr(lower, "tell me a joke") ||
        strstr(lower, "entertain me")) {
        respond_small_talk(brain);
        return 1;
    }

    /* ── Quick confirmations ── */
    if (strcmp(lower, "yes") == 0 || strcmp(lower, "yeah") == 0 ||
        strcmp(lower, "yep") == 0 || strcmp(lower, "sure") == 0 ||
        strcmp(lower, "ok") == 0 || strcmp(lower, "okay") == 0 ||
        strcmp(lower, "yup") == 0 || strcmp(lower, "right") == 0) {
        printf("  Got it! What would you like to do next?\n");
        return 1;
    }
    if (strcmp(lower, "no") == 0 || strcmp(lower, "nope") == 0 ||
        strcmp(lower, "nah") == 0 || strcmp(lower, "not really") == 0) {
        printf("  Alright! Let me know when you're ready.\n");
        return 1;
    }

    return 0;
}

/*
 * try_natural_language — map natural phrasing to existing command handlers.
 * Returns 1 if handled, 0 if not.
 */
static int try_natural_language(RethinkBrain *brain, const char *lower) {
    char entity[128] = "";

    /* ── Teach ── */
    if (strstr(lower, "this is a ") || strstr(lower, "this is an ") ||
        strstr(lower, "that's a ") || strstr(lower, "that is a ") ||
        strstr(lower, "it's a ") || strstr(lower, "it is a ") ||
        strstr(lower, "that is an ") || strstr(lower, "it's an ")) {
        const char *triggers[] = {"this is a ", "this is an ", "that's a ",
                                   "that is a ", "it's a ", "it is a ",
                                   "that is an ", "it's an ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_teach(brain, entity); return 1; }
    }
    if (strstr(lower, "teach me") || strstr(lower, "learn about") ||
        strstr(lower, "remember that this is") ||
        (strstr(lower, "learn ") && !strstr(lower, "learning"))) {
        const char *triggers[] = {"teach me about ", "teach me ", "learn about ",
                                   "remember that this is ", "learn ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_teach(brain, entity); return 1; }
    }

    /* ── Show ── */
    if (strstr(lower, "show me") || strstr(lower, "look at") ||
        strstr(lower, "can you see") || strstr(lower, "take a look") ||
        strstr(lower, "check out") || strstr(lower, "see this") ||
        strstr(lower, "observe this")) {
        const char *triggers[] = {"show me ", "look at ", "can you see ",
                                   "take a look at ", "check out ", "observe this ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_show(brain, entity); return 1; }
    }

    /* ── Classify ── */
    if (strstr(lower, "what is this") || strstr(lower, "what's this") ||
        strstr(lower, "what do you see") || strstr(lower, "what am i") ||
        strstr(lower, "recognize") || strstr(lower, "identify this") ||
        strstr(lower, "what do you think this is") ||
        strstr(lower, "what did you see") || strstr(lower, "classify")) {
        cmd_what(brain);
        return 1;
    }

    /* ── Explain ── */
    if (strstr(lower, "why ") || strstr(lower, "explain ") ||
        strstr(lower, "how come ") || strstr(lower, "what caused ") ||
        strstr(lower, "reason for ")) {
        const char *triggers[] = {"why is ", "why did ", "why does ", "why are ",
                                   "why ", "explain ", "how come ",
                                   "what caused ", "reason for ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_why(brain, entity); return 1; }
    }

    /* ── Predict ── */
    if (strstr(lower, "what if ") || strstr(lower, "what would happen") ||
        strstr(lower, "what happens") || strstr(lower, "imagine ") ||
        strstr(lower, "predict ") || strstr(lower, "suppose ")) {
        const char *triggers[] = {"what if ", "what would happen if ",
                                   "what happens if ", "what happens when ",
                                   "imagine ", "predict ", "suppose ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_whatif(brain, entity); return 1; }
    }

    /* ── Think ── */
    if (strstr(lower, "think about") || has_word(lower, "ponder") ||
        has_word(lower, "contemplate") || has_word(lower, "meditate") ||
        (has_word(lower, "think") && (int)strlen(lower) < 20) ||
        strcmp(lower, "hmm") == 0 || strcmp(lower, "hmmm") == 0 ||
        strcmp(lower, "let me think") == 0) {
        cmd_think(brain, "");
        return 1;
    }

    /* ── Sleep ── */
    if (strstr(lower, "go to sleep") || strstr(lower, "take a nap") ||
        strstr(lower, "time to dream") || strstr(lower, "time to sleep") ||
        strstr(lower, "get some rest") || strstr(lower, "need rest")) {
        cmd_sleep(brain, "");
        return 1;
    }

    /* ── Status ── */
    if (strstr(lower, "brain status") || strstr(lower, "system status") ||
        strstr(lower, "your state") || strstr(lower, "your status") ||
        strstr(lower, "how are things") || strstr(lower, "report")) {
        rethink_print_status(brain);
        return 1;
    }

    /* ── Memory ── */
    if (strstr(lower, "what do you remember") || strstr(lower, "your memories") ||
        strstr(lower, "show memories") || strstr(lower, "recall everything") ||
        strstr(lower, "show me your memories")) {
        cmd_memory(brain);
        return 1;
    }

    /* ── Categories ── */
    if (strstr(lower, "what do you know") || strstr(lower, "what have you learned") ||
        strstr(lower, "your knowledge") || strstr(lower, "what categories") ||
        strstr(lower, "what have i taught")) {
        cmd_categories(brain);
        return 1;
    }

    /* ── Speak ── */
    if (strstr(lower, "say something") || strstr(lower, "speak up") ||
        strstr(lower, "talk to me") || strstr(lower, "use your words")) {
        cmd_speak(brain);
        return 1;
    }

    /* ── Help ── */
    if (strstr(lower, "what can you do") || strstr(lower, "your commands") ||
        strstr(lower, "how does this work") || strstr(lower, "how do i use") ||
        strstr(lower, "your abilities") || strstr(lower, "available commands") ||
        strstr(lower, "guide me")) {
        print_help();
        return 1;
    }

    /* ── Introspect ── */
    if (strstr(lower, "how confident") || strstr(lower, "how sure") ||
        strstr(lower, "reflect on yourself") || strstr(lower, "self assess") ||
        strstr(lower, "examine yourself") || strstr(lower, "know yourself") ||
        strstr(lower, "are you sure") || strstr(lower, "how certain")) {
        cmd_introspect(brain);
        return 1;
    }

    /* ── Focus ── */
    if (strstr(lower, "pay attention to") || strstr(lower, "focus on") ||
        strstr(lower, "concentrate on") || strstr(lower, "look for ")) {
        const char *triggers[] = {"pay attention to ", "focus on ",
                                   "concentrate on ", "look for ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_focus(brain, entity); return 1; }
    }

    /* ── Do action ── */
    if (strstr(lower, "perform ") || strstr(lower, "execute ") ||
        strstr(lower, "try doing ") || strstr(lower, "can you do ")) {
        const char *triggers[] = {"perform ", "execute ", "try doing ",
                                   "can you do ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_do(brain, entity); return 1; }
    }

    /* ── Motor ── */
    if (strstr(lower, "what actions") || strstr(lower, "motor skills") ||
        strstr(lower, "your actions") || strstr(lower, "action repertoire")) {
        cmd_actions(brain);
        return 1;
    }

    /* ── Social ── */
    if (strstr(lower, "who do you know") || strstr(lower, "your friends") ||
        strstr(lower, "social circle") || strstr(lower, "known agents") ||
        strstr(lower, "people you know")) {
        cmd_agents(brain);
        return 1;
    }

    /* ── Meet someone ── */
    if (strstr(lower, "meet ") || strstr(lower, "introduce ") ||
        strstr(lower, "this is my friend ")) {
        const char *triggers[] = {"meet ", "introduce ", "this is my friend ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_agent(brain, entity); return 1; }
    }

    /* ── Novel ── */
    if (strstr(lower, "is this new") || strstr(lower, "is this novel") ||
        strstr(lower, "seen this before") || strstr(lower, "ever seen")) {
        cmd_novel(brain);
        return 1;
    }

    /* ── Hear ── */
    if (strstr(lower, "listen to ") || strstr(lower, "hear this ") ||
        strstr(lower, "can you hear ")) {
        const char *triggers[] = {"listen to ", "hear this ", "can you hear ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_hear(brain, entity); return 1; }
    }

    /* ── Touch ── */
    if (strstr(lower, "feel this ") || strstr(lower, "touch this ") ||
        strstr(lower, "can you feel ")) {
        const char *triggers[] = {"feel this ", "touch this ", "can you feel ", NULL};
        try_extract(lower, entity, sizeof(entity), triggers);
        if (entity[0]) { cmd_touch(brain, entity); return 1; }
    }

    /* ── Cause ── */
    if (strstr(lower, " causes ") || strstr(lower, " leads to ")) {
        /* Try to parse "X causes Y" or "X leads to Y" */
        char a[32] = "", b[32] = "";
        if (sscanf(lower, "%31s causes %31s", a, b) == 2 ||
            sscanf(lower, "%31s leads to %31s", a, b) == 2) {
            char combined[128];
            snprintf(combined, sizeof(combined), "%s -> %s", a, b);
            cmd_cause(brain, combined);
            return 1;
        }
    }

    return 0;
}

/* ──── Main Loop ──── */

int main(void) {
    srand((unsigned)time(NULL));

    printf("\n");
    printf("  ╔════════════════════════════════════════════════╗\n");
    printf("  ║       NOUS — A BRAIN-INSPIRED AI COMPANION     ║\n");
    printf("  ║             Built from scratch in C             ║\n");
    printf("  ╚════════════════════════════════════════════════╝\n");
    printf("\n");

    RethinkBrain *brain = rethink_create();
    printf("  24 brain modules online.\n");
    printf("  Say 'hi', ask me anything, or type 'help' for commands.\n\n");

    char line[512];
    char cmd[64], arg[448];
    char lower[512];

    while (1) {
        printf("nous> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        char *trimmed = trim(line);
        if (!trimmed[0]) continue;

        /* Build lowercase copy for NLU matching */
        to_lower(lower, trimmed, sizeof(lower));
        nous_turn++;

        /* ═══ Phase 1: Conversational (greetings, feelings, opinions) ═══ */
        int conv = try_conversation(brain, lower);
        if (conv == -1) {
            /* Farewell — quit */
            printf("\n  Shutting down brain...\n");
            rethink_print_status(brain);
            break;
        }
        if (conv == 1) { printf("\n"); continue; }

        /* ═══ Phase 2: Natural language → command mapping ═══ */
        if (try_natural_language(brain, lower)) {
            printf("\n"); continue;
        }

        /* ═══ Phase 3: Exact command dispatch (backward compatible) ═══ */
        if (!split_first(trimmed, cmd, sizeof(cmd), arg, sizeof(arg))) {
            respond_unknown(brain, trimmed);
            printf("\n"); continue;
        }

        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
            printf("\n  Shutting down brain...\n");
            rethink_print_status(brain);
            break;
        }
        else if (strcmp(cmd, "help") == 0)        print_help();
        else if (strcmp(cmd, "teach") == 0)       cmd_teach(brain, arg);
        else if (strcmp(cmd, "vocab") == 0)        cmd_vocab(brain, arg);
        else if (strcmp(cmd, "show") == 0)         cmd_show(brain, arg);
        else if (strcmp(cmd, "hear") == 0)         cmd_hear(brain, arg);
        else if (strcmp(cmd, "touch") == 0)        cmd_touch(brain, arg);
        else if (strcmp(cmd, "what") == 0)         cmd_what(brain);
        else if (strcmp(cmd, "why") == 0)          cmd_why(brain, arg);
        else if (strcmp(cmd, "whatif") == 0)        cmd_whatif(brain, arg);
        else if (strcmp(cmd, "cause") == 0)        cmd_cause(brain, arg);
        else if (strcmp(cmd, "speak") == 0)        cmd_speak(brain);
        else if (strcmp(cmd, "listen") == 0)       cmd_listen(brain, arg);
        else if (strcmp(cmd, "think") == 0)        cmd_think(brain, arg);
        else if (strcmp(cmd, "sleep") == 0)        cmd_sleep(brain, arg);
        else if (strcmp(cmd, "status") == 0)       rethink_print_status(brain);
        else if (strcmp(cmd, "log") == 0)          rethink_print_log(brain);
        else if (strcmp(cmd, "memory") == 0)       cmd_memory(brain);
        else if (strcmp(cmd, "categories") == 0)   cmd_categories(brain);
        else if (strcmp(cmd, "novel") == 0)        cmd_novel(brain);
        else if (strcmp(cmd, "train") == 0)        cmd_train(brain, arg);
        else if (strcmp(cmd, "focus") == 0)        cmd_focus(brain, arg);
        else if (strcmp(cmd, "attend") == 0)       cmd_attend(brain);
        else if (strcmp(cmd, "do") == 0)           cmd_do(brain, arg);
        else if (strcmp(cmd, "plan") == 0)         cmd_plan(brain, arg);
        else if (strcmp(cmd, "actions") == 0)      cmd_actions(brain);
        else if (strcmp(cmd, "agent") == 0)        cmd_agent(brain, arg);
        else if (strcmp(cmd, "agents") == 0)       cmd_agents(brain);
        else if (strcmp(cmd, "mind") == 0)         cmd_mind(brain, arg);
        else if (strcmp(cmd, "believe") == 0)      cmd_believe(brain, arg);
        else if (strcmp(cmd, "cooperate") == 0)    cmd_cooperate(brain, arg);
        else if (strcmp(cmd, "defect") == 0)       cmd_defect_cmd(brain, arg);
        else if (strcmp(cmd, "observe") == 0)      cmd_observe(brain, arg);
        else if (strcmp(cmd, "mirror") == 0)       cmd_mirror(brain);
        else if (strcmp(cmd, "emotion") == 0)      cmd_set_emotion(brain, arg);
        else if (strcmp(cmd, "empathize") == 0)    cmd_empathize(brain, arg);
        else if (strcmp(cmd, "help_agent") == 0)   cmd_help_agent(brain, arg);
        else if (strcmp(cmd, "helped") == 0)       cmd_helped(brain, arg);
        else if (strcmp(cmd, "confident") == 0)    cmd_confident(brain);
        else if (strcmp(cmd, "errors") == 0)       cmd_meta_errors(brain);
        else if (strcmp(cmd, "strategies") == 0)   cmd_strategies(brain);
        else if (strcmp(cmd, "select") == 0)       cmd_select(brain);
        else if (strcmp(cmd, "self") == 0)         cmd_self(brain);
        else if (strcmp(cmd, "introspect") == 0)   cmd_introspect(brain);
        else if (strcmp(cmd, "meta_lr") == 0)      cmd_meta_lr(brain);
        else if (strcmp(cmd, "meta_perf") == 0)    cmd_meta_perf(brain, arg);
        else if (strcmp(cmd, "meta_err") == 0)     cmd_meta_err(brain, arg);
        else if (strcmp(cmd, "meta_self") == 0)    cmd_meta_self_update(brain, arg);
        else {
            respond_unknown(brain, trimmed);
        }

        printf("\n");
    }

    rethink_destroy(brain);
    printf("  Nous signing off. Goodbye.\n\n");
    return 0;
}
