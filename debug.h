#ifndef ERROR_H
#define ERROR_H


#include <stdio.h>

#include "Opt.h"
extern Opt GlobOpt;


#define error(...) (GlobOpt.ERR && fprintf(stderr, "*** Error :: " __VA_ARGS__))
#define warning(...) (GlobOpt.WRN && fprintf(stderr, "*** Warning :: " __VA_ARGS__))
#define info(...) (GlobOpt.INF && fprintf(stderr, "*** Info :: " __VA_ARGS__))

#define error_atom(...) error("Atom :: " __VA_ARGS__)


#endif
