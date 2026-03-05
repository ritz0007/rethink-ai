# Training the Rethink Brain — Guide to Batch Learning with Data Files

> How to train the brain from files, build your own datasets, and use online databases

---

## Overview

The Rethink Brain learns through its `train` command, which reads a plain text file line by line and executes training directives. This is how you can batch-load:

- **Categories** — what the brain can classify (animals, objects, emotions, etc.)
- **Causal knowledge** — cause-effect relationships (rain → wet → slipping)
- **Vocabulary** — words the brain can use to communicate
- **Experiences** — things the brain "sees" and processes through its full pipeline

---

## Training File Format

Each line is a directive. Comments start with `#`. Blank lines are ignored.

```
# This is a comment

# Teach categories
category cat
category dog
category bird
category fish

# Add causal relationships
cause rain -> wet_ground 0.9
cause wet_ground -> slipping 0.6
cause fire -> smoke 0.95
cause smoke -> alarm 0.8

# Add vocabulary words
vocab running
vocab jumping
vocab eating

# Show experiences (runs through full brain pipeline)
show fluffy_cat
show barking_dog

# Experience with specific label: experience <label> <thing>
experience cat tabby_kitten
experience dog golden_retriever
```

---

## Using the Training Command

```
brain> train data/animals.txt
  Training from 'data/animals.txt'...
  Training complete!
    Categories: +10 (total: 10)
    Causes:     +8 (total nodes: 12)
    Vocab:      +5 (total: 15)
    Experiences:+6 (total: 6)
```

---

## Sample Training Files

### 1. Animals (`data/animals.txt`)

```
# Animals — Basic categories and knowledge
# Teaches the brain about common animals

# Categories
category cat
category dog
category bird
category fish
category horse
category snake
category rabbit
category elephant

# Animal relationships
cause cat -> purring 0.8
cause cat -> hunting 0.6
cause dog -> barking 0.9
cause dog -> loyalty 0.85
cause bird -> flying 0.7
cause bird -> singing 0.8
cause fish -> swimming 0.95
cause snake -> venom 0.4
cause horse -> galloping 0.8
cause elephant -> trumpeting 0.75

# Vocabulary
vocab furry
vocab scaly
vocab feathered
vocab large
vocab small
vocab fast
vocab slow
vocab wild
vocab domestic

# Experiences
experience cat fluffy_kitten
experience cat orange_tabby
experience dog golden_retriever
experience dog small_poodle
experience bird red_robin
experience bird blue_jay
experience fish goldfish
experience horse brown_stallion
```

### 2. Weather & Nature (`data/weather.txt`)

```
# Weather — Causal chains
# Teaches the brain about weather cause-effect

# Categories
category sunny
category rainy
category cloudy
category stormy
category snowy

# Causal chains
cause sun -> warmth 0.9
cause warmth -> evaporation 0.7
cause evaporation -> clouds 0.8
cause clouds -> rain 0.6
cause clouds -> shade 0.9
cause rain -> wet_ground 0.95
cause wet_ground -> slipping 0.5
cause wet_ground -> puddles 0.8
cause rain -> rainbow 0.3
cause rain -> umbrella 0.7
cause cold -> frost 0.7
cause cold -> snow 0.5
cause snow -> snowman 0.6
cause storm -> thunder 0.9
cause storm -> lightning 0.85
cause storm -> power_outage 0.3
cause wind -> kite_flying 0.5
cause wind -> fallen_trees 0.2
```

### 3. Daily Life (`data/daily_life.txt`)

```
# Daily life — Actions and consequences
# Teaches the brain about human activities

# Categories
category food
category drink
category tool
category vehicle
category furniture

# Causal knowledge
cause hunger -> eating 0.9
cause eating -> satisfaction 0.8
cause eating -> energy 0.7
cause thirst -> drinking 0.9
cause exercise -> tiredness 0.7
cause exercise -> health 0.8
cause sleep -> rest 0.9
cause rest -> energy 0.8
cause study -> knowledge 0.7
cause study -> tiredness 0.5
cause work -> money 0.8
cause money -> shopping 0.6

# Vocabulary
vocab eat
vocab drink
vocab run
vocab sleep
vocab think
vocab read
vocab write
vocab build
```

---

## How to Use Online Databases

The Rethink Brain uses 32-dimensional feature vectors derived from word hashing — not raw images or text embeddings. To train from online databases, you **convert** the data to training file format.

### Step 1: Choose a Database

| Database | URL | What It Has | Best For |
|----------|-----|-------------|----------|
| WordNet | `wordnet.princeton.edu` | Word relationships, hypernyms, synonyms | Categories, vocabulary |
| ConceptNet | `conceptnet.io` | Common sense: "cat HasA fur", "rain CausesDesire umbrella" | Causal knowledge |
| Wikidata | `wikidata.org` | Structured knowledge: "cat instanceOf animal" | Categories + relationships |
| DBpedia | `dbpedia.org` | Wikipedia structured data | Everything |
| Open Images | `storage.googleapis.com/openimages` | Image labels + descriptions | Visual categories |

### Step 2: Convert to Training Format

#### Example: ConceptNet → Training File

ConceptNet has an API that returns common-sense knowledge:

```bash
# Fetch data about "cat" from ConceptNet (HTTPS API)
curl -s "https://api.conceptnet.io/c/en/cat?limit=20" | python3 -c "
import json, sys
data = json.load(sys.stdin)
for edge in data.get('edges', []):
    rel = edge['rel']['label']
    start = edge['start']['label'].lower().replace(' ', '_')
    end = edge['end']['label'].lower().replace(' ', '_')
    weight = min(edge.get('weight', 1.0) / 5.0, 1.0)
    
    if rel == 'IsA':
        print(f'category {end}')
    elif rel in ('Causes', 'HasSubevent'):
        print(f'cause {start} -> {end} {weight:.1f}')
    elif rel in ('HasProperty', 'HasA'):
        print(f'vocab {end}')
    elif rel == 'UsedFor':
        print(f'cause {start} -> {end} {weight:.1f}')
" > data/conceptnet_cat.txt
```

This produces something like:

```
category animal
category pet
category feline
cause cat -> purring 0.6
cause cat -> hunting 0.4
vocab furry
vocab whiskers
vocab independent
```

#### Example: WordNet → Training File

```bash
# Use NLTK (Python) to extract WordNet hypernyms
python3 -c "
from nltk.corpus import wordnet as wn
animals = ['cat', 'dog', 'bird', 'fish', 'horse', 'snake', 'bear', 'wolf']
for a in animals:
    print(f'category {a}')
    syns = wn.synsets(a, pos=wn.NOUN)
    if syns:
        for lemma in syns[0].lemma_names():
            if lemma != a:
                print(f'vocab {lemma}')
        for hyp in syns[0].hypernyms():
            for lemma in hyp.lemma_names():
                print(f'cause {a} -> {lemma.lower()} 0.7')
" > data/wordnet_animals.txt
```

#### Example: Custom CSV → Training File

If you have a CSV file (from Kaggle, etc.):

```bash
# Convert a CSV with columns: name,type,habitat,diet
python3 -c "
import csv
with open('animals.csv') as f:
    for row in csv.DictReader(f):
        name = row['name'].lower().replace(' ', '_')
        animal_type = row['type'].lower()
        habitat = row['habitat'].lower().replace(' ', '_')
        diet = row['diet'].lower()
        print(f'category {name}')
        print(f'cause {name} -> {habitat} 0.8')
        print(f'cause {name} -> {diet} 0.7')
        print(f'vocab {animal_type}')
" > data/from_csv.txt
```

### Step 3: Load Into the Brain

```bash
./chatbot
brain> train data/conceptnet_cat.txt
brain> train data/wordnet_animals.txt
brain> train data/weather.txt
```

You can load multiple files in sequence — knowledge accumulates!

---

## Creating a Master Training Script

```bash
#!/bin/bash
# train_all.sh — Load all training data into the brain

echo "Starting batch training session..."
echo "
train data/animals.txt
train data/weather.txt
train data/daily_life.txt
sleep 10
status
quit
" | ./chatbot
```

Or pipe commands directly:

```bash
echo "train data/animals.txt
status
quit" | ./chatbot
```

---

## Advanced: Building a Knowledge Scraper

Here's a complete Python script that scrapes ConceptNet for multiple topics:

```python
#!/usr/bin/env python3
"""
scrape_conceptnet.py — Build training files from ConceptNet

Usage:
    python3 scrape_conceptnet.py cat dog bird fish > data/animals_conceptnet.txt
"""

import sys
import json
import urllib.request
import time

def fetch_concept(word, limit=30):
    url = f"https://api.conceptnet.io/c/en/{word}?limit={limit}"
    try:
        with urllib.request.urlopen(url) as response:
            return json.loads(response.read())
    except Exception as e:
        print(f"# Error fetching {word}: {e}", file=sys.stderr)
        return None

def concept_to_training(word, data):
    lines = []
    lines.append(f"# Knowledge about: {word}")
    lines.append(f"category {word}")
    
    for edge in data.get('edges', []):
        rel = edge['rel']['label']
        start_lang = edge['start'].get('language', 'en')
        end_lang = edge['end'].get('language', 'en')
        if start_lang != 'en' or end_lang != 'en':
            continue
        
        start = edge['start']['label'].lower().replace(' ', '_')
        end = edge['end']['label'].lower().replace(' ', '_')
        weight = min(edge.get('weight', 1.0) / 5.0, 0.95)
        weight = max(weight, 0.1)
        
        if rel == 'IsA':
            lines.append(f"category {end}")
        elif rel in ('Causes', 'HasSubevent', 'UsedFor', 'ReceivesAction'):
            lines.append(f"cause {start} -> {end} {weight:.2f}")
        elif rel in ('HasProperty', 'HasA', 'PartOf', 'MadeOf'):
            lines.append(f"vocab {end}")
        elif rel == 'CapableOf':
            lines.append(f"cause {start} -> {end} {weight:.2f}")
        elif rel == 'AtLocation':
            lines.append(f"cause {start} -> {end} {weight:.2f}")
    
    lines.append(f"experience {word} {word}")
    lines.append("")
    return lines

if __name__ == "__main__":
    topics = sys.argv[1:] if len(sys.argv) > 1 else ["cat", "dog", "bird"]
    
    print("# Auto-generated training data from ConceptNet")
    print(f"# Topics: {', '.join(topics)}")
    print()
    
    for topic in topics:
        data = fetch_concept(topic)
        if data:
            for line in concept_to_training(topic, data):
                print(line)
        time.sleep(0.5)  # Be polite to the API
```

Run it:

```bash
python3 scrape_conceptnet.py cat dog bird fish horse > data/animals_conceptnet.txt
./chatbot
brain> train data/animals_conceptnet.txt
```

---

## Recommended Training Order

1. **Basic categories first** — `category cat`, `category dog` (gives the brain a foundation)
2. **Causal knowledge second** — `cause rain -> wet` (builds reasoning capacity)
3. **Vocabulary third** — `vocab running` (enables communication)
4. **Experiences last** — `show fluffy_cat` (triggers full brain pipeline with classification, memory, emotion)
5. **Sleep after batch loading** — `sleep 20` (consolidates everything via NREM/REM)

---

## Limits to Know

| Resource | Maximum | Notes |
|----------|---------|-------|
| Categories | 32 | Prototype learner limit (PROTO_MAX_CATEGORIES) |
| Causal nodes | 64 | Causal network limit (CAUSAL_MAX_NODES) |
| Causal edges | 256 | (CAUSAL_MAX_EDGES) |
| Vocabulary | 128 | Communication engine limit (COMM_MAX_SYMBOLS) |
| Memories | 512 | Decay memory limit |
| Feature dimensions | 32 | All vectors are 32-dim |

If you hit a limit, the brain will silently ignore new entries. In future phases, these limits can be increased.
