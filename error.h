#ifndef ERROR_H
#define ERROR_H

#include "color.h"
#include "options.h"
extern Options *GlobalOptions;


#define _error_msg(emsg, ...) \
    (GlobalOptions->color ?\
        printf(ERR emsg CLR " :: " __VA_ARGS__) :\
        printf(    emsg     " :: " __VA_ARGS__))

#define error(...)           _error_msg("Error", __VA_ARGS__)
#define error_atomlist(...)  _error_msg("AtomListError", __VA_ARGS__)
#define error_atom(...)      _error_msg("AtomError", __VA_ARGS__)
#define error_interpret(...) _error_msg("InterpretError", __VA_ARGS__)


#endif
