#ifndef ERROR_H
#define ERROR_H

#include "options.h"
extern Options *GlobalOptions;

#define ERR "\033[0;31m"
#define WRN "\033[0;35m"
#define CLR "\033[0m"

void error_malloc(const char *err);
void error_parse(const char *source, int p, const char *err);
void warning_parse(const char *source, int p, const char *wrn);

#define error(...) (GlobalOptions->color ?\
    printf(ERR "Error" CLR " :: " __VA_ARGS__) :\
    printf(    "Error"     " :: " __VA_ARGS__))

#define error_atomlist(...) (GlobalOptions->color ?\
    printf(ERR "AtomListError" CLR " :: " __VA_ARGS__) :\
    printf(    "AtomListError"     " :: " __VA_ARGS__))

#define error_atom(...) (GlobalOptions->color ?\
    printf(ERR "AtomError" CLR " :: " __VA_ARGS__) :\
    printf(    "AtomError"     " :: " __VA_ARGS__))

#define error_interpret(...) (GlobalOptions->color ?\
    printf(ERR "InterpretError" CLR " :: " __VA_ARGS__) :\
    printf(    "InterpretError"     " :: " __VA_ARGS__))

#endif
