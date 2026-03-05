# 🧠 Brain Science: The Thalamus — Routing, Filtering, and Attention

## The Brain's Switchboard

The thalamus sits at the center of the brain and acts as the **router** for almost all information. Without the thalamus, the cortex would be overwhelmed with raw data.

### What the Thalamus Does

```
1. ROUTING:   Sends visual signals to visual cortex, audio to auditory cortex, etc.
2. FILTERING: Blocks irrelevant information (you don't notice your shirt touching your skin)
3. GATING:    Adjusts signal strength based on attention
4. RELAYING:  Passes processed information between cortical regions
```

### The Attention Gate

This is the most important function for us. The thalamus can **amplify or suppress** specific signals:

```
You're reading this guide:
  Visual signal (text):    thalamus gain = HIGH → passes to cortex
  Background noise:        thalamus gain = LOW  → mostly blocked
  Your phone buzzing:      thalamus gain = MEDIUM → partially passes
  
Now your phone rings (important!):
  Visual signal (text):    thalamus gain = DROPS
  Phone ring:              thalamus gain = SPIKES
  → Attention shifts!
```

### The Thalamic Reticular Nucleus (TRN)

The TRN is a thin shell around the thalamus that acts as the **attention controller**:

```
Cortex → TRN: "I'm interested in visual stuff right now"
TRN → Thalamus: Amplify visual relay, suppress others

Cortex → TRN: "Wait, what was that sound?"
TRN → Thalamus: Amplify auditory relay, suppress visual
```

This is a **top-down attention** mechanism. The cortex tells the thalamus what to pay attention to.

## Information Flow Architecture

```
                    ┌──────────┐
                    │  CORTEX   │
                    │(processes)│
                    └─────┬────┘
                          │ feedback (what to attend to)
                    ┌─────▼────┐
    Senses ────→   │ THALAMUS  │ ────→ Cortical Regions
    (raw input)    │ (routes + │
                   │  filters) │
                    └──────────┘
                          │
                    ┌─────▼────┐
                    │   TRN    │
                    │(attention│
                    │ control) │
                    └──────────┘
```

## Implementing a Brain Router

### The Architecture

```c
typedef enum {
    REGION_VISUAL,      /* Processes visual input (retina + SOM) */
    REGION_MEMORY,      /* Stores and recalls (Hopfield + decay) */
    REGION_LEARNING,    /* Categorizes (prototype learner) */
    REGION_THINKING,    /* Internal activity (spontaneous + dreams) */
    REGION_EMOTION,     /* Evaluates importance (amygdala analog) */
    REGION_EXECUTIVE,   /* Decides what to do (prefrontal analog) */
    NUM_REGIONS
} BrainRegion;

typedef struct {
    float gain[NUM_REGIONS];          /* Attention gain per region */
    float priority[NUM_REGIONS];      /* Current priority */
    float signal_strength[NUM_REGIONS]; /* Incoming signal strength */
} Thalamus;
```

### Routing Rules

```
Route(input_type, input_data):
  1. Determine which region handles this type
  2. Apply attention gain
  3. If gain × signal > threshold: forward to region
  4. Record that this region is active
  5. Active regions can request attention changes
```

### Attention Control

```c
void thalamus_update_attention(Thalamus *thal) {
    /* Bottom-up: strong signals demand attention */
    for (int r = 0; r < NUM_REGIONS; r++) {
        if (thal->signal_strength[r] > SURPRISE_THRESHOLD) {
            thal->gain[r] = fminf(1.0f, thal->gain[r] + 0.3f);
        }
    }
    
    /* Top-down: executive sets priorities */
    for (int r = 0; r < NUM_REGIONS; r++) {
        thal->gain[r] = thal->gain[r] * 0.7f + thal->priority[r] * 0.3f;
    }
    
    /* Competition: gains sum to a constant (zero-sum attention) */
    float total = 0;
    for (int r = 0; r < NUM_REGIONS; r++) total += thal->gain[r];
    if (total > 0) {
        for (int r = 0; r < NUM_REGIONS; r++) 
            thal->gain[r] /= total;
    }
}
```

### Why Zero-Sum Attention?

You can't attend to everything equally. When visual attention goes up, something else goes down. This is why:
- You can't read and listen to a conversation at the same time
- Drivers miss things when on the phone
- "In the zone" means other inputs are suppressed

The thalamus enforces this by making attention **competitive** — regions literally inhibit each other.

## The Message Bus Pattern

Every region communicates through a shared **message bus**, routed by the thalamus:

```c
typedef struct {
    BrainRegion source;
    BrainRegion target;
    float data[MAX_SIGNAL_SIZE];
    int data_size;
    float urgency;    /* How important is this message */
    float timestamp;
} BrainSignal;
```

Regions don't call each other directly. They post signals, and the thalamus decides:
- **Where** it goes
- **Whether** it passes (attention gate)
- **How strong** it is when it arrives (gain)

This is elegant because:
1. Regions are **decoupled** — they don't know about each other
2. Attention is **centralized** — one system controls information flow
3. New regions can be **added** without changing existing ones
4. The system is **debuggable** — watch the message bus to see what's happening

---

*Next: [Build — Brain Architecture with Routing](03_build_brain_architecture.md)*
