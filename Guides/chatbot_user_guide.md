# Rethink Brain Chatbot — User Guide & Demo

> How to interact with the brain through the terminal chatbot

---

## Quick Start

```bash
cd src
make chat       # Build the chatbot
./chatbot       # Launch it
```

You'll see:

```
  ╔════════════════════════════════════════╗
  ║     RETHINK BRAIN — COMING ONLINE      ║
  ╚════════════════════════════════════════╝

  Brain created. 21 modules initialized.
  Type 'help' for commands, 'quit' to exit.

brain> 
```

---

## Demo Session — Full Walkthrough

Copy-paste these commands one by one to see the brain in action:

### 1. Teach It Categories

```
brain> teach cat
  Brain learned category 'cat'
  (Categories known: 1, Vocabulary: 1)

brain> teach dog
  Brain learned category 'dog'
  (Categories known: 2, Vocabulary: 2)

brain> teach bird
  Brain learned category 'bird'
  (Categories known: 3, Vocabulary: 3)
```

**What happens**: The brain creates prototype representations for each category. Each word is hashed into a 32-dimensional feature vector. The brain can now classify things into cat, dog, or bird.

### 2. Show Things & Watch Classification

```
brain> show fluffy_cat
  Brain saw 'fluffy_cat':
    Surprise:      0.754 (very surprising!)
    Understanding: 0.570
    Emotion:       neutral (·_·)
    Attention:     0.49
    Recognized as: 'cat' (confidence: 85%)
```

```
brain> show barking_dog
  Brain saw 'barking_dog':
    Surprise:      0.314 (expected)
    Understanding: 0.761
    Emotion:       familiar (~_~)
    Attention:     0.47
    Recognized as: 'dog' (confidence: 91%)
```

```
brain> show alien_blob
  Brain saw 'alien_blob':
    Surprise:      0.852 (very surprising!)
    Understanding: 0.320
    Emotion:       surprise (!_!)
    Attention:     0.48
    [Novel — doesn't match any known category]
```

**What happens**: The brain processes the input through its full pipeline — predictive coding (surprise), emotional evaluation, attention competition, prototype classification, and memory storage.

### 3. Build Causal Knowledge

```
brain> cause rain -> wet_ground 0.9
  Learned: 'rain' causes 'wet_ground' (strength: 90%)

brain> cause wet_ground -> slipping 0.6
  Learned: 'wet_ground' causes 'slipping' (strength: 60%)

brain> cause rain -> umbrella 0.7
  Learned: 'rain' causes 'umbrella' (strength: 70%)
```

### 4. Ask Why & What-If

```
brain> why slipping
  'slipping' because: wet_ground(60%) <- rain(90%)

brain> whatif rain
  'rain' will cause: wet_ground(100%), slipping(97%), umbrella(99%)
```

**What happens**: The causal network traces backward (why) or forward (what-if) through cause-effect chains.

### 5. Feed Multi-Sensory Input

```
brain> show red_ball
  Brain saw 'red_ball': ...

brain> hear bouncing
  Brain heard 'bouncing':
    Multi-modal binding: strength=0.62  confidence=0.78

brain> touch rubber_soft
  Brain felt 'rubber_soft':
    Multi-modal binding (3 senses): strength=0.71
```

**What happens**: Visual, auditory, and tactile inputs are fused using Bayesian precision-weighted binding — just like the brain's Superior Temporal Sulcus.

### 6. Let It Think & Dream

```
brain> think 50
  Brain is thinking for 50 ticks...
  Done. Current state: understanding=0.127 surprise=0.099
  Emotion: familiar (~_~)

brain> sleep 10
  Brain entering sleep for 10 cycles...
  (NREM replay + REM recombination + insight detection)
  Brain is awake. Memories consolidated.
  Memories: 3  Understanding: 0.114
```

### 7. Communication

```
brain> speak
  Brain says: "cat dog bird"
  Current emotion: familiar (~_~)

brain> listen cat see bird
  Brain heard 3 words and processed them.
  Understanding: 0.220
```

### 8. Inspect the Brain

```
brain> status
  ======================================
         RETHINK BRAIN STATUS
  ======================================
   State:         IDLE
   Tick:          75
   Experiences:   4
   Attention:     0.41
   ...

brain> categories
  === Known Categories (3) ===
  [0] 'cat' (1 examples)
  [1] 'dog' (1 examples)
  [2] 'bird' (1 examples)

brain> memory
  === Stored Memories (3) ===
  [0] strength=0.82 importance=0.65 dim=32
  ...

brain> log
  -- Brain Log (last 20 entries) --
  ...
```

### 9. Check Novelty

```
brain> show weird_alien
brain> novel
  Yes — this is NOVEL. The brain hasn't seen anything like this.
brain> teach alien
  Brain learned category 'alien'
brain> show weird_alien
  Recognized as: 'alien' (confidence: 78%)
```

### 10. Shut Down

```
brain> quit
  Shutting down brain...
  Brain destroyed. Goodbye.
```

---

## Command Reference

| Command | Description | Example |
|---------|-------------|---------|
| `teach <cat>` | Learn a new category | `teach cat` |
| `vocab <word>` | Add word to vocabulary | `vocab running` |
| `cause <A> -> <B> [str]` | Add causal link | `cause fire -> smoke 0.9` |
| `show <thing>` | Show something (visual) | `show fluffy_ball` |
| `hear <sound>` | Feed auditory input | `hear birdsong` |
| `touch <stim>` | Feed tactile input | `touch hot_metal` |
| `what` | Classify last input | `what` |
| `why <event>` | Explain causes | `why slipping` |
| `whatif <cause>` | Predict effects | `whatif earthquake` |
| `novel` | Check if last input is novel | `novel` |
| `speak` | Brain describes its state | `speak` |
| `listen <words>` | Feed sentence | `listen cat see bird` |
| `think [N]` | Idle thinking (N ticks) | `think 100` |
| `sleep [N]` | Dream consolidation | `sleep 10` |
| `train <file>` | Batch-train from file | `train data/animals.txt` |
| `status` | Brain status summary | `status` |
| `log` | Show brain activity log | `log` |
| `memory` | List stored memories | `memory` |
| `categories` | List known categories | `categories` |
| `help` | Show all commands | `help` |
| `quit` | Shut down | `quit` |

---

## Tips

1. **Teach before showing** — The brain needs categories to classify things
2. **Build causal chains** — `cause A -> B` then `cause B -> C` lets the brain reason across steps
3. **Multi-sensory** — `show` + `hear` + `touch` together triggers multi-modal binding
4. **Sleep helps** — After lots of experiences, `sleep 20` consolidates memories
5. **Touch keywords** — `hot`, `cold`, `sharp`, `soft`, `rough`, `vibrate` trigger special touch processing
6. **Train from file** — Use `train data/animals.txt` to batch-load knowledge (see Training Guide)
