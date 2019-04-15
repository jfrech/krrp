#include "atomlist.h"
#include "atom.h"
#include "debug.h"
#include "memorymanagement.h"


static AtomListNode *atomlistnode_new(Atom *atom, AtomListNode *next);


Atom *atomlist_representation(AtomList *lst) {
    if (!atomlist_is(lst))
        return error("atomlist :: atomlist_representation: Expected AtomList.\n"), NULL;

    if (!lst->head)
        return atom_string_newfl("[]");

    Atom *repr = atom_string_newfl("[");
    AtomListNode *node = lst->head;
    while (node) {
        repr = atom_string_concat3(
            repr,
            atom_representation(node->atom),
            atom_string_newfl(node->next ? ", " : "]")
        );

        node = node->next;
    }

    return repr;
}

AtomList *atomlist_new(AtomListNode *head) {
    AtomList *lst = mm_malloc("atomlist_new", sizeof *lst);
    lst->head = head;

    return lst;
}

bool atomlist_is(AtomList *lst) {
    if (!lst)
        return error("atomlist :: atomlist_is: Malformed atom.\n"), false;

    return true;
}

static AtomListNode *atomlistnode_new(Atom *atom, AtomListNode *next) {
    if (!atom_is(atom))
        return error("atomlist :: atomlistnode_new: Given invalid atom.\n"), NULL;

    AtomListNode *node = mm_malloc("atomlistnode_new", sizeof *node);
    node->atom = atom;
    node->next = next;

    return node;
}

void atomlist_push(AtomList *lst, Atom *atom) {
    if (!atomlist_is(lst) || !atom_is(atom))
        return;

    AtomListNode **pnode = &lst->head;
    while (*pnode)
        pnode = &(*pnode)->next;

    *pnode = atomlistnode_new(atom, NULL);
}

void atomlist_push_front(AtomList *lst, Atom *atom) {
    if (!atomlist_is(lst) || !atom_is(atom))
        return;

    AtomListNode *node = atomlistnode_new(atom, lst->head);
    lst->head = node;
}

Atom *atomlist_pop_front(AtomList *lst) {
    if (!atomlist_is(lst))
        return NULL;

    if (!lst->head)
        return error("atomlist :: atomlist_pop_front: List length is zero.\n"), NULL;

    AtomListNode *node = lst->head;
    lst->head = node->next;
    Atom *atom = node->atom;
    atomlistnode_free(node);

    return atom;
}

int atomlist_len(AtomList *lst) {
    if (!atomlist_is(lst))
        return -1;

    int len = 0;

    AtomListNode *node = lst->head;
    while (node) {
        len++;
        node = node->next;
    }

    return len;
}

bool atomlist_empty(AtomList *lst) {
    if (!atomlist_is(lst))
        return true;

    return !lst->head;
}

AtomList *atomlist_copy(AtomList *lst) {
    if (!atomlist_is(lst))
        return NULL;

    AtomList *nlst = atomlist_new(NULL);
    AtomListNode **pnnode = &nlst->head;
    AtomListNode *node = lst->head;
    while (node) {
        *pnnode = atomlistnode_new(node->atom, NULL);
        pnnode = &(*pnnode)->next;
        node = node->next;
    }

    return nlst;
}

bool atomlist_equal(AtomList *lstA, AtomList *lstB) {
    if (!atomlist_is(lstA) || !atomlist_is(lstB))
        return false;

    AtomListNode *nodeA = lstA->head, *nodeB = lstB->head;
    while (nodeA && nodeB) {
        if (!atom_equal(nodeA->atom, nodeB->atom))
            return false;

        nodeA = nodeA->next; nodeB = nodeB->next;
    }

    return nodeA == nodeB;
}

bool atomlist_purely(AtomList *lst, atom_type type) {
    if (!atomlist_is(lst))
        return false;

    AtomListNode *node = lst->head;
    while (node) {
        if (!atom_is(node->atom) || node->atom->type != type)
            return false;

        node = node->next;
    }

    return true;
}

void atomlist_remove_by_pointer(AtomList *lst, Atom *atom) {
    if (!atomlist_is(lst))
        return;

    AtomListNode **node = &lst->head;
    while (*node) {
        if ((*node)->atom == atom) {
            AtomListNode *next = (*node)->next;
            atomlistnode_free(*node);
            *node = next;
            return;
        }
        node = &(*node)->next;
    }

    error("atomlist :: atomlist_remove_by_pointer: Did not find %s.\n", atom_repr(atom));
}
