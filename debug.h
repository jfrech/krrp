#ifndef ERROR_H
#define ERROR_H

#define ERROR true
#define WARNING true
#define INFO true

#include <stdio.h>

int no_debug();

#define error(...) (ERROR ? fprintf(stderr, "*** Error :: " __VA_ARGS__) : no_debug())
#define warning(...) (WARNING ? fprintf(stderr, "*** Warning :: " __VA_ARGS__) : no_debug())
#define info(...) (INFO ? fprintf(stderr, "*** Info :: " __VA_ARGS__) : no_debug())

#define error_atom(...) error("Atom :: " __VA_ARGS__)

#endif
