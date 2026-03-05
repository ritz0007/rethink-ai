/*
 * chatbot.c — Interactive Chat with the Rethink Brain
 *
 * Rethink AI — V15
 *
 * A terminal-based chatbot that lets you interact with the brain:
 *   - Teach it categories ("teach cat")
 *   - Show it things ("show fluffy_ball")
 *   - Ask what something is ("what is this?")
 *   - Ask why ("why slipping?") 
 *   - Ask what-if ("whatif rain?")
 *   - Feed it sounds and touch
 *   - Let it think and dream
 *   - Build causal knowledge
 *   - Talk to it
 *   - Train from file ("train data/animals.txt")
 *   - Set attention goals ("focus cat")
 *   - Execute motor actions ("do reach_grab")
 *   - Model social agents ("agent alice")
 *   - Metacognition ("introspect", "confident", "strategies")
 *
 * Compile: make chatbot
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

/* ──── Print help ──── */
static void print_help(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              RETHINK BRAIN — INTERACTIVE CHAT               ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
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

/* ──── Main Loop ──── */

int main(void) {
    srand((unsigned)time(NULL));

    printf("\n");
    printf("  ╔════════════════════════════════════════╗\n");
    printf("  ║     RETHINK BRAIN — COMING ONLINE      ║\n");
    printf("  ╚════════════════════════════════════════╝\n");
    printf("\n");

    RethinkBrain *brain = rethink_create();
    printf("  Brain created. 24 modules initialized.\n");
    printf("  Type 'help' for commands, 'quit' to exit.\n\n");

    char line[512];
    char cmd[64], arg[448];

    while (1) {
        printf("brain> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        char *trimmed = trim(line);
        if (!trimmed[0]) continue;

        if (!split_first(trimmed, cmd, sizeof(cmd), arg, sizeof(arg)))
            continue;

        /* ── Dispatch ── */
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
            printf("  Unknown command: '%s'. Type 'help' for available commands.\n", cmd);
        }

        printf("\n");
    }

    rethink_destroy(brain);
    printf("  Brain destroyed. Goodbye.\n\n");
    return 0;
}
