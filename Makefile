# Build with: mingw32-make        (Git Bash on Windows; plain `make` elsewhere)
# Targets:    all (default, -O2) | debug (-O0) | run | plot | clean
CC	= gcc
CFLAGS	= -std=c11 -Wall -Wextra -g
SRC	= $(wildcard src/*.c)
OBJ	= $(SRC:.c=.o)
TARGET	= pagesim.exe

all: CFLAGS += -O2 -DNDEBUG
all: $(TARGET)

debug: CFLAGS += -O0 -DDEBUG
debug: clean $(TARGET)

# run: generate the three CSV files the report is based on (seed 1, defaults)
run: $(TARGET)
	mkdir -p results
	./$(TARGET) --gen random --out results/random.csv
	./$(TARGET) --gen locality --out results/locality.csv
	./$(TARGET) --gen mine --out results/mine.csv

# `py` is the Windows Python launcher; override with: mingw32-make plot PYTHON=python3
PYTHON ?= py

plot:
	$(PYTHON) scripts/plot.py results/random.csv results/locality.csv results/mine.csv

# link; -lm because refgen.c uses pow()
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# any header change rebuilds every .o (small project, keep it simple)
src/%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all debug clean run plot

