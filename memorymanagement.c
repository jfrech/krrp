#include "memorymanagement.h"

#include <stdlib.h>
#include <stdio.h>

#include "atom.h"
#include "debug.h"


extern AtomList* GlobalAtomTable;
extern AtomList* GlobalAtomTableMutable;

static void memorymanagement_ABORT(const char *msg);
static void memorymanagement_FATAL_ERROR(const char *msg);

static MemoryManagementDebug dbg = {
    .allocations = 0,
    .deallocations = 0,
    .nullpointer_frees = 0,
    .allocated_bytes = 0
};

void *mm_malloc(const char *msg, size_t n) {
    void *ptr = malloc(n);

    if (!ptr)
        memorymanagement_ABORT("mm_malloc");

    dbg.allocations++;
    dbg.allocated_bytes += n;

    return ptr;
}

void mm_free(const char *msg, void *ptr) {
    if (!ptr) { dbg.nullpointer_frees++; return; }

    free(ptr);
    dbg.deallocations++;
}

void mm_prematurely_free_mutable(Atom *atom) {
    atomlist_remove_by_pointer(GlobalAtomTableMutable, atom);
    atom_free(atom);
}

void mm_print_status() {
    info("* Memory management status *\n");
    info("Allocated        : %ld\n", dbg.allocations);
    info("Deallocated      : %ld\n", dbg.deallocations);
    info("Alloc Discrepancy: %ld\n", dbg.allocations-dbg.deallocations);
    info("NULL deallocated : %ld\n", dbg.nullpointer_frees);
    info("Allocated bytes  : %ld (~ %ld Mb)\n", dbg.allocated_bytes, dbg.allocated_bytes / 1000000);
    if (dbg.allocations == dbg.deallocations && dbg.nullpointer_frees == 0)
        info("Conclusio: No memory management problems detected.\n");
    else
        error("Conclusio: PROBLEMATIC MEMORY STATE.\n");
}

static void memorymanagement_ABORT(const char *msg) {
    memorymanagement_free_all();

    memorymanagement_FATAL_ERROR(msg);
    exit(EXIT_FAILURE);
}

static void memorymanagement_FATAL_ERROR(const char *msg) {
    error("A *FATAL* memory error has occurred.\n    %s\n", msg);

    exit(EXIT_FAILURE);
}


void memorymanagement_free_all() {
    if (GlobalAtomTable)
        mm_free_gat(GlobalAtomTable);

    if (GlobalAtomTableMutable)
        mm_free_gat(GlobalAtomTableMutable);

    mm_print_status();
}


void mm_free_gat(AtomList *gat) {
    info("Freeing %d ...\n", atomlist_len(gat));

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
    else if (atom->type == atom_type_list) {
        ListAtom *list_atom = atom->atom;
        atomlist_free(list_atom->list);
        mm_free("atom_free: list_atom", list_atom);
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
