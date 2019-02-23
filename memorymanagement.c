#include "memorymanagement.h"

#include <stdlib.h>
#include <stdio.h>

#include "atom.h"
#include "options.h"


extern Options* GlobalOptions;
extern AtomList* GlobalAtomTable;
extern AtomList* GlobalAtomTableMutable;

static void memorymanagement_ABORT(const char *msg);
static void memorymanagement_FATAL_ERROR(const char *msg);

static long allocations = 0;
static long de_allocations = 0;
static long nullpointer_frees = 0;

void *mm_malloc(size_t n) {
    allocations++;

    void *ptr = malloc(n);

    if (!ptr)
        memorymanagement_ABORT("mm_malloc");

    return ptr;
}

void mm_free(void *ptr) {
    if (!ptr) { nullpointer_frees++; return; }
    de_allocations++;

    free(ptr);
}

void mm_print_status() {
    // for (int i = 0; i < 1000; i++) mm_free(NULL);
    printf("  Allocated: %ld\nDeallocated: %ld\n", allocations, de_allocations);
    printf("Discrepency: %ld (freeing NULL: %ld)\n", allocations-de_allocations, nullpointer_frees);
}

static void memorymanagement_ABORT(const char *msg) {
    memorymanagement_free_all();

    memorymanagement_FATAL_ERROR(msg);
    exit(EXIT_FAILURE);
}

static void memorymanagement_FATAL_ERROR(const char *msg) {
    fprintf(stderr, "A fatal memory error has occurred.\n    %s\n", msg);

    exit(EXIT_FAILURE);
}


void memorymanagement_free_all() {
    if (GlobalOptions)
        mm_free(GlobalOptions);

    if (GlobalAtomTable)
        mm_free_gat(GlobalAtomTable);

    if (GlobalAtomTableMutable)
        mm_free_gat(GlobalAtomTableMutable);
}


void mm_free_gat(AtomList *gat) {
    printf("Freeing %d ...\n", atomlist_len(gat));

    AtomListNode *node = gat->head;
    while (node) {
        atom_free(node->atom);
        node = node->next;
    }

    atomlist_free(gat);
}


void atom_free(Atom *atom) {
    if (!atom)
        return;

    if (!atom_is(atom)) {
        memorymanagement_FATAL_ERROR("atom_free: Malformed atom -- cannot properly mm_free.\n");
        mm_free(atom);
        return;
    }

    if (atom->type == atom_type_null
    || atom->type == atom_type_nullcondition
    || atom->type == atom_type_nullscope
    || atom->type == atom_type_Enull)
        ;

    else if (atom->type == atom_type_name) {
        NameAtom *name_atom = atom->atom;
        mm_free(name_atom->name);
        mm_free(name_atom);
    }
    else if (atom->type == atom_type_integer) {
        IntegerAtom *integer_atom = atom->atom;
        mm_free(integer_atom);
    }
    else if (atom->type == atom_type_primitive) {
        PrimitiveAtom *primitive_atom = atom->atom;
        mm_free(primitive_atom);
    }
    else if (atom->type == atom_type_functiondeclaration) {
        FunctionDeclarationAtom *functiondeclaration_atom = atom->atom;
        atomlist_free(functiondeclaration_atom->parameters);
        atomlist_free(functiondeclaration_atom->body);
        mm_free(functiondeclaration_atom);
    }
    else if (atom->type == atom_type_function) {
        FunctionAtom *function_atom = atom->atom;
        if (function_atom->parameters)
            atomlist_free(function_atom->parameters);
        if (function_atom->body)
            atomlist_free(function_atom->body);
        mm_free(function_atom->primitive);
        mm_free(function_atom);
    }
    else if (atom->type == atom_type_scope) {
        ScopeAtom *scope_atom = atom->atom;
        atomlist_free(scope_atom->names);
        atomlist_free(scope_atom->binds);
        mm_free(scope_atom);
    }
    else if (atom->type == atom_type_structinitializer) {
        StructInitializerAtom *structinitializer_atom = atom->atom;
        atomlist_free(structinitializer_atom->fields);
        mm_free(structinitializer_atom);
    }
    else if (atom->type == atom_type_struct) {
        StructAtom *struct_atom = atom->atom;
        mm_free(struct_atom);
    }
    else if (atom->type == atom_type_string) {
        StringAtom *string_atom = atom->atom;
        mm_free(string_atom->str);
        mm_free(string_atom);
    }
    else
        memorymanagement_FATAL_ERROR("atom_free: Trying to mm_free atom of unknown type\n");

    mm_free(atom);
}


/* Does not free any atoms. */
void atomlist_free(AtomList *lst) {
    if (!lst || !atomlist_is(lst))
        return;

    AtomListNode *node = lst->head;
    mm_free(lst);

    while (node) {
        AtomListNode *nnode = node->next;
        atomlistnode_free(node);
        node = nnode;
    }
}

/* Does not free the atom. */
void atomlistnode_free(AtomListNode *node) {
    if (!node)
        return;

    mm_free(node);
}
