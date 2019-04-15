CC = clang
CFLAGS = -Wall -Wpedantic -O0

.PHONY: stdlib

SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

FRAGMENT = $(wildcard stdlib/*.c_fragment)
STDLIB = $(wildcard stdlib/*)


krrp: $(SOURCES) $(HEADERS) $(FRAGMENT)
	$(CC) $(CFLAGS) -c $(SOURCES)
	mv *.o src/
	$(CC) $(OBJECTS) -o $@
	rm -f $(OBJECTS)

stdlib: $(STDLIB)
	python3 stdlib/assemble.py
	$(MAKE) krrp
