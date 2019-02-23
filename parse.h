#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include "atom.h"

typedef enum {
    parse_state_main,
    parse_state_functiondeclaration_parameters,
    parse_state_functiondeclaration_body,
    parse_state_struct_fields
} parse_state;

AtomList *parse(const char *source);

#endif
