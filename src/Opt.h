#ifndef OPT_H
#define OPT_H

#include <stdbool.h>


typedef struct Opt {
    bool ERR, WRN, INF;
    long maximum_interpretation_recursion_depth;
    bool string_view;

    bool pedantic;
} Opt;

#endif
