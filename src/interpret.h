#ifndef INTERPRET_H
#define INTERPRET_H

#include <stdbool.h>
#include "atom.h"


Atom *_interpret(long recursion_depth, AtomList *parsed, Atom *scope, bool active);
Atom *interpret(AtomList *parsed);
Atom *interpret_with_scope(AtomList *parsed, Atom *scope);

Atom *TODO_main_scope(); // TODO

#endif
