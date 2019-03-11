#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "atomlist.h"

struct PArgs {
    bool do_test;
    bool print_help;
    AtomList *sources;
};
typedef struct PArgs PArgs;

PArgs *parse_args(PArgs *pargs, int argc, char **argv);

#endif
