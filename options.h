#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdio.h>

typedef struct Options Options;

struct Options {
    bool debug_address;
    bool debug_printGlobalAtomTable;

    bool pedantic_scope_verification;

    long maximum_interpretation_recursion_depth;
};

void globaloptions_init();

#endif
