#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

typedef struct Options Options;

struct Options {
    bool color;
    bool debug_address;
    bool debug_printGlobalAtomTable;
};

void globaloptions_init();
void globaloptions_free();

#endif
