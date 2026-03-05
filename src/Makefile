# ============================================================
# Makefile — Rethink AI Brain
# ============================================================
# Compile: make
# Clean:   make clean
# Test:    make test && ./test_brain
# ============================================================

CC      = gcc
CFLAGS  = -std=c11 -O2 -Wall -Wextra -Wpedantic -I.
LDFLAGS = -lm

# ──── Source files ────
SRCS = \
	neurons/neuron.c \
	neurons/network.c \
	learning/hebbian.c \
	learning/stdp.c \
	learning/prototype.c \
	perception/retina.c \
	perception/som.c \
	memory/hopfield.c \
	memory/decay.c \
	thinking/spontaneous.c \
	thinking/dream.c \
	modular/brain.c \
	modular/emotion.c \
	understanding/predictor.c \
	understanding/causal.c \
	communication/comm.c \
	rethink_brain.c

OBJS = $(SRCS:.c=.o)

# ──── Main library ────
librethink.a: $(OBJS)
	ar rcs $@ $^

# ──── Test binary ────
test: test_brain
test_brain: test_brain.o librethink.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lrethink $(LDFLAGS)

test_brain.o: test_brain.c rethink_brain.h
	$(CC) $(CFLAGS) -c $< -o $@

# ──── Pattern rule for .c → .o ────
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ──── Clean ────
clean:
	find . -name '*.o' -delete
	rm -f librethink.a test_brain

# ──── Phony targets ────
.PHONY: clean test

# ──── Dependencies (key headers) ────
neurons/neuron.o: neurons/neuron.c neurons/neuron.h
neurons/network.o: neurons/network.c neurons/network.h neurons/neuron.h neurons/synapse.h
learning/hebbian.o: learning/hebbian.c learning/hebbian.h
learning/stdp.o: learning/stdp.c learning/stdp.h
learning/prototype.o: learning/prototype.c learning/prototype.h
perception/retina.o: perception/retina.c perception/retina.h
perception/som.o: perception/som.c perception/som.h
memory/hopfield.o: memory/hopfield.c memory/hopfield.h
memory/decay.o: memory/decay.c memory/decay.h
thinking/spontaneous.o: thinking/spontaneous.c thinking/spontaneous.h
thinking/dream.o: thinking/dream.c thinking/dream.h memory/decay.h
modular/brain.o: modular/brain.c modular/brain.h
modular/emotion.o: modular/emotion.c modular/emotion.h
understanding/predictor.o: understanding/predictor.c understanding/predictor.h
understanding/causal.o: understanding/causal.c understanding/causal.h
communication/comm.o: communication/comm.c communication/comm.h
rethink_brain.o: rethink_brain.c rethink_brain.h
