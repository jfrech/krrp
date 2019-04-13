#ifndef ATOMLIST_H
#define ATOMLIST_H

#include "typedefs.h"
#include <stdbool.h>


struct AtomList { AtomListNode *head; };
struct AtomListNode { Atom *atom; AtomListNode *next; };


AtomList *atomlist_new(AtomListNode *head);
bool atomlist_is(AtomList *lst);

void atomlist_push(AtomList *lst, Atom *atom);
void atomlist_push_front(AtomList *lst, Atom *atom);
Atom *atomlist_pop_front(AtomList *lst);
int atomlist_len(AtomList *lst);
bool atomlist_empty(AtomList *lst);
AtomList *atomlist_copy(AtomList *lst);
bool atomlist_equal(AtomList *lstA, AtomList *lstB);
bool atomlist_purely(AtomList *lst, atom_type type);

void atomlist_remove_by_pointer(AtomList *lst, Atom *atom);
Atom *atomlist_representation(AtomList *lst);

#endif
