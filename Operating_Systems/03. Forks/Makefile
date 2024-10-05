CC = gcc
CFLAGS = -Wall -Wextra

TARGET = main
SRCDIR = $(HOME)/quiz-3
BINDIR = $(HOME)/quiz-3/bins

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o ./$@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	mkdir -p $(BINDIR)
	mv $(TARGET) $(BINDIR)/

clean:
	rm -f $(OBJECTS) $(BINDIR)/$(TARGET) $(TARGET)
