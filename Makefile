CC	= gcc
CFLAGS	= -std=c11 -Wall -Wextra -g
SRC	= $(wildcard src/*.c)
OBJ	= $(SRC:.c=.o)
TARGET	= pagesim.exe

all: CFLAGS += -O2 -DNDEBUG
all: $(TARGET)

debug: CFLAGS += -O0 -DDEBUG
debug: clean $(TARGET)

run: $(TARGET)
	mkdir -p results
	./$(TARGET) --gen random --out results/random.csv
	./$(TARGET) --gen locality --out results/locality.csv
	./$(TARGET) --gen mine --out results/mine.csv

PYTHON ?= py

plot:
	$(PYTHON) scripts/plot.py results/random.csv results/locality.csv results/mine.csv

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

src/%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all debug clean run plot

