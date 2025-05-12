CC = gcc
CFLAGS = -I../common -Wall -Wextra
LDFLAGS = -lpthread
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o ) $(wildcard ../common/*.o)
EXECUTABLE = xShellClient

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) ../config.h
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
