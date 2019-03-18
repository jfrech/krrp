CC = clang
CFLAGS = -Wall -Wpedantic -O0

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

krrp: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(CC) $(OBJECTS) -o $@
	rm -f $(OBJECTS)
