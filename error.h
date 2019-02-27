#ifndef ERROR_H
#define ERROR_H

#define ERROR true
#define WARNING true
#define INFO false

#include <stdio.h>

#define error(...) (ERROR && fprintf(stderr, "*** Error :: " __VA_ARGS__))
#define warning(...) (WARNING && fprintf(stderr, "*** Warning :: " __VA_ARGS__))
#define info(...) (INFO && fprintf(stderr, "*** Info :: " __VA_ARGS__))

#define error_atom(...) error("Atom :: " __VA_ARGS__)

#endif
