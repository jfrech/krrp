#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#include "atom.h"


void free_gat();
void atomlist_free(AtomList*);
void atom_free(Atom*);
void atomlistnode_free(AtomListNode*);

void memorymanagement_free_all();
void memorymanagement_abort();

#endif
