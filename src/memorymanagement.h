#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#include <stdlib.h>

#include "atom.h"


typedef struct {
    long allocations;
    long deallocations;
    long nullpointer_frees;
    long allocated_bytes;
} MemoryManagementDebug;


void *mm_malloc(const char *msg, size_t n);
void mm_free(const char *msg, void *ptr);

void mm_prematurely_free_mutable(Atom *atom);

void mm_print_status();

void mm_free_gat();
void atom_free(Atom *atom);
void atomlist_free(AtomList *lst);
void atomlistnode_free(AtomListNode *node);

void memorymanagement_free_all();

#endif
