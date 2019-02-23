#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#include "atom.h"


void free_gat();
void atomlist_free(AtomList *lst);
void atom_free(Atom *atom);
void atomlistnode_free(AtomListNode *node);

void memorymanagement_free_all();
void memorymanagement_abort();

#endif
