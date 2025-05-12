CC = gcc
CFLAGS = -I../common -Wall -Wextra
LDFLAGS = -lm -lpthread -lutil
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o ) $(wildcard ../common/*.o)
EXECUTABLE = xShellServer

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) ../config.h
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
