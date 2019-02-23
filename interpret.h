#ifndef INTERPRET_H
#define INTERPRET_H

#include <stdbool.h>
#include "atom.h"

Atom *interpret(AtomList *parsed, Atom *scope, bool active);
Atom *main_scope();

#endif
