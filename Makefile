CC = clang
CFLAGS = -Wall -Wpedantic -O0

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

OTHER = $(wildcard stdlib/stdlib.c_fragment)

krrp: $(SOURCES) $(HEADERS) $(OTHER)
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(CC) $(OBJECTS) -o $@
	rm -f $(OBJECTS)
