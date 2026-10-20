CC	= gcc
CFLAGS	= -std=c11 -Wall -Wextra -g
SRC	= $(wildcard src/*.c)
OBJ	= $(SRC:.c=.o)
TARGET	= pagesim.exe

all: CFLAGS += -O2 -DNDEBUG
all: $(TARGET)

debug: CFLAGS += -O0 -DDEBUG
debug: clean $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all debug clean

