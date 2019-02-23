#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#include "atom.h"

#include <stdlib.h>

void *mm_malloc(size_t n);
void mm_free(void *ptr);
void mm_print_status();

void mm_free_gat();
void atomlist_free(AtomList *lst);
void atom_free(Atom *atom);
void atomlistnode_free(AtomListNode *node);

void memorymanagement_free_all();

#endif
