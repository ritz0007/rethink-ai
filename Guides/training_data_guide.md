# Training Data Guide for Nous

> How to feed knowledge into Nous using open datasets from the internet.

---

## Nous Training Format

Nous learns through its `train <file>` command, which reads a text file line-by-line and executes each line as a command. The format is simple:

```
# Lines starting with # are comments
# Blank lines are ignored

# Teach categories (concept names)
teach cat
teach dog
teach bird
teach car
teach tree

# Add vocabulary words
vocab sunshine
vocab thunder
vocab gentle
vocab fierce

# Add causal relationships (A causes B)
cause rain -> wet_ground
cause wet_ground -> slipping
cause fire -> smoke
cause smoke -> alarm

# Show things (creates sensory experiences + memories)
show fluffy_kitten
show barking_dog
show chirping_bird

# Listen to sounds
hear thunder
hear birdsong
hear engine

# Touch stimuli
touch soft_fur
touch rough_bark
touch cold_ice
```

---

## Open Datasets You Can Use

### 1. ConceptNet — Commonsense Knowledge

**URL**: https://conceptnet.io  
**Download**: https://github.com/commonsense/conceptnet5/wiki/Downloads  
**What it is**: 21 million edges of commonsense knowledge (IsA, HasA, UsedFor, Causes, etc.)

**How to convert for Nous**:

```bash
# Download the CSV
wget https://s3.amazonaws.com/conceptnet/downloads/2019/edges/conceptnet-assertions-5.7.0.csv.gz
gunzip conceptnet-assertions-5.7.0.csv.gz

# Extract English "IsA" relations → teach commands
grep '/r/IsA/' conceptnet-assertions-5.7.0.csv | \
  grep '/c/en/' | \
  awk -F'\t' '{
    split($3, a, "/"); split($4, b, "/");
    if (length(a[4]) < 20 && length(b[4]) < 20)
      print "teach " b[4]
  }' | sort -u | head -500 > nous_concepts.txt

# Extract "Causes" relations → cause commands
grep '/r/Causes/' conceptnet-assertions-5.7.0.csv | \
  grep '/c/en/' | \
  awk -F'\t' '{
    split($3, a, "/"); split($4, b, "/");
    if (length(a[4]) < 20 && length(b[4]) < 20)
      print "cause " a[4] " -> " b[4]
  }' | sort -u | head -200 > nous_causes.txt
```

### 2. WordNet — Lexical Database

**URL**: https://wordnet.princeton.edu  
**What it is**: 155k+ words organized into synonym sets with definitions and relationships.

**How to convert for Nous**:

```bash
# If you have WordNet data files (data.noun, data.adj, etc.)
# Extract noun categories
grep -oP '^\d+ \d+ \w \d+ \K\w+' data.noun | \
  head -300 | \
  sed 's/^/teach /' > nous_wordnet.txt

# Or use the NLTK WordNet corpus (if Python is available for conversion only)
python3 -c "
from nltk.corpus import wordnet as wn
for s in list(wn.all_synsets('n'))[:500]:
    name = s.lemmas()[0].name().replace('_',' ')
    if len(name) < 20:
        print(f'teach {name}')
" > nous_wordnet.txt
```

### 3. Simple English Wikipedia

**URL**: https://simple.wikipedia.org/wiki/Main_Page  
**Dump**: https://dumps.wikimedia.org/simplewiki/latest/  
**What it is**: Wikipedia written in simple English — great for extracting clean concepts.

**How to convert**:

```bash
# Download the titles dump
wget https://dumps.wikimedia.org/simplewiki/latest/simplewiki-latest-all-titles-in-ns0.gz
gunzip simplewiki-latest-all-titles-in-ns0.gz

# Convert to teach commands (single-word concepts work best)
grep -E '^[a-z]+$' simplewiki-latest-all-titles-in-ns0 | \
  head -500 | \
  sed 's/^/teach /' > nous_wiki.txt
```

### 4. NELL — Never-Ending Language Learner

**URL**: http://rtw.ml.cmu.edu/rtw/  
**What it is**: CMU's project that continuously reads the web and extracts structured facts.

**How to convert**:

```bash
# Download NELL beliefs
wget http://rtw.ml.cmu.edu/rtw/resources/nell_beliefs.csv

# Extract high-confidence "generalizations" (categoryOf relations)
grep 'generalizations' nell_beliefs.csv | \
  awk -F'\t' '{if ($5 > 0.9) print "teach " $2}' | \
  sed 's/concept://; s/_/ /g' | \
  head -300 > nous_nell.txt
```

### 5. DailyDialog — Conversational Data

**URL**: http://yanran.li/dailydialog  
**What it is**: 13,118 multi-turn dialogues about daily life topics.

**How it helps Nous**: While Nous doesn't learn conversation patterns from files (its NLU is rule-based), DailyDialog topics can be used to build Nous's knowledge base:

```bash
# Extract topic words from dialogues
# Topics: 1=ordinary, 2=school, 3=culture, 4=attitude, 5=emotion,
#         6=relationship, 7=tourism, 8=health, 9=work, 10=politics
cat dialogues_topic.txt | \
  tr ' ' '\n' | \
  tr '[:upper:]' '[:lower:]' | \
  grep -E '^[a-z]{3,15}$' | \
  sort | uniq -c | sort -rn | \
  head -200 | awk '{print "vocab " $2}' > nous_dialog_vocab.txt
```

### 6. Open Mind Common Sense

**URL**: https://github.com/commonsense/conceptnet5  
**What it is**: The crowd-sourced foundation of ConceptNet — raw human knowledge contributions.

Same conversion as ConceptNet above.

---

## Recommended Training Pipeline

For a well-rounded Nous brain, train in this order:

```bash
# Step 1: Core concepts (what things ARE)
nous> train nous_concepts.txt

# Step 2: Causal knowledge (what causes what)
nous> train nous_causes.txt

# Step 3: Vocabulary expansion
nous> train nous_vocab.txt

# Step 4: Sensory experiences
nous> train nous_experiences.txt

# Step 5: Let it consolidate
nous> think 100
nous> sleep 20

# Step 6: Check what it learned
nous> categories
nous> memory
nous> introspect
```

---

## Creating Your Own Training Data

The simplest approach — just write facts you know:

```
# my_knowledge.txt

# Animals
teach cat
teach dog
teach bird
teach fish
teach horse

# Causes
cause rain -> puddles
cause puddles -> splashing
cause cold -> shivering
cause running -> sweating
cause studying -> knowledge

# Things I want it to see
show sunset
show mountain
show ocean

# Sounds
hear music
hear laughter
hear wind

# Vocabulary
vocab beautiful
vocab dangerous
vocab mysterious
```

Then: `nous> train my_knowledge.txt`

---

## Tips

1. **Start small.** 50-100 concepts is plenty for a brain that just came online.
2. **Teach categories before causes.** Nous needs to know what things are before understanding relationships.
3. **Use `show` after `teach`.** Teaching creates the category; showing creates memories and experiences.
4. **Let it sleep.** After heavy training, `sleep 10-20` consolidates memories.
5. **Check understanding.** Use `introspect`, `categories`, and `memory` to verify learning.
6. **Single words work best.** Nous's prototype learner works on word features — keep category names to 1-2 words.

---

*The best training data for Nous isn't the biggest dataset — it's the most meaningful experiences.*
