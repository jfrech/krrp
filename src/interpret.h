#ifndef INTERPRET_H
#define INTERPRET_H

#include <stdbool.h>
#include "atom.h"


Atom *interpret(AtomList *parsed);
Atom *interpret_with_scope(AtomList *parsed, Atom *scope);
void inject_main(Atom *scope);


#endif
