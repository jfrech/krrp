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
static long allocated_bytes = 0;

void *mm_malloc(size_t n) {
    void *ptr = malloc(n);

    if (!ptr)
        memorymanagement_ABORT("mm_malloc");

    allocations++;
    allocated_bytes += n;

    return ptr;
}

void mm_free(const char *msg, void *ptr) {
    if (!ptr) { nullpointer_frees++; return; }

    free(ptr);
    de_allocations++;
}

void mm_print_status() {
    printf("* Memory management status *\n");
    printf("Allocated        : %ld\n", allocations);
    printf("Deallocated      : %ld\n", de_allocations);
    printf("NULL deallocated : %ld\n", nullpointer_frees);
    printf("Discrepency      : %ld\n", allocations-de_allocations);
    printf("Allocated bytes  : %ld (~ %ld Mb)\n", allocated_bytes, allocated_bytes / 1000000);
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
        mm_free("GlobalOptions", GlobalOptions);

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
        mm_free("atom_free", atom);
        return;
    }

    if (atom->type == atom_type_null
    || atom->type == atom_type_nullcondition
    || atom->type == atom_type_nullscope
    || atom->type == atom_type_Enull)
        ;

    else if (atom->type == atom_type_name) {
        NameAtom *name_atom = atom->atom;
        mm_free("atom_free: name_atom->name", name_atom->name);
        mm_free("atom_free: name_atom", name_atom);
    }
    else if (atom->type == atom_type_integer) {
        IntegerAtom *integer_atom = atom->atom;
        mm_free("atom_free: integer_atom", integer_atom);
    }
    else if (atom->type == atom_type_primitive) {
        PrimitiveAtom *primitive_atom = atom->atom;
        mm_free("atom_free: primitive_atom", primitive_atom);
    }
    else if (atom->type == atom_type_functiondeclaration) {
        FunctionDeclarationAtom *functiondeclaration_atom = atom->atom;
        atomlist_free(functiondeclaration_atom->parameters);
        atomlist_free(functiondeclaration_atom->body);
        mm_free("atom_free: functiondeclaration_atom", functiondeclaration_atom);
    }
    else if (atom->type == atom_type_function) {
        FunctionAtom *function_atom = atom->atom;
        if (function_atom->parameters)
            atomlist_free(function_atom->parameters);
        if (function_atom->body)
            atomlist_free(function_atom->body);
        if (function_atom->primitive)
            mm_free("atom_free: function_atom->primitive", function_atom->primitive);
        mm_free("atom_free: function_atom", function_atom);
    }
    else if (atom->type == atom_type_scope) {
        ScopeAtom *scope_atom = atom->atom;
        atomlist_free(scope_atom->names);
        atomlist_free(scope_atom->binds);
        mm_free("atom_free: scope_atom", scope_atom);
    }
    else if (atom->type == atom_type_structinitializer) {
        StructInitializerAtom *structinitializer_atom = atom->atom;
        atomlist_free(structinitializer_atom->fields);
        mm_free("atom_free: structinitializer_atom", structinitializer_atom);
    }
    else if (atom->type == atom_type_struct) {
        StructAtom *struct_atom = atom->atom;
        mm_free("atom_free: struct_atom", struct_atom);
    }
    else if (atom->type == atom_type_string) {
        StringAtom *string_atom = atom->atom;
        mm_free("atom_free: string_atom->str", string_atom->str);
        mm_free("atom_free: string_atom", string_atom);
    }
    else
        memorymanagement_FATAL_ERROR("atom_free: Trying to mm_free atom of unknown type\n");

    mm_free("atom_free: atom", atom);
}


/* Does not free any atoms. */
void atomlist_free(AtomList *lst) {
    if (!lst || !atomlist_is(lst))
        return;

    AtomListNode *node = lst->head;
    mm_free("atomlist_free", lst);

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

    mm_free("atomlistnode_free", node);
}
