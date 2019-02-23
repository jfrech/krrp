#include "memorymanagement.h"

#include <stdlib.h>
#include <stdio.h>

#include "atom.h"
#include "options.h"


extern Options* GlobalOptions;
extern AtomList* GlobalAtomTable;
extern AtomList* GlobalAtomTableMutable;


void memorymanagement_error(const char *msg) {
    fprintf(stderr, "A fatal memory error has occurred.\n    %s\n", msg);
}

void memorymanagement_free_all() {
    if (GlobalOptions)
        free(GlobalOptions);

    if (GlobalAtomTable)
        free_gat(GlobalAtomTable);

    if (GlobalAtomTableMutable)
        free_gat(GlobalAtomTableMutable);
}

void memorymanagement_abort(const char *msg) {
    memorymanagement_free_all();

    memorymanagement_error(msg);
    exit(EXIT_FAILURE);
}


void free_gat(AtomList *gat) {
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
        memorymanagement_error("atom_free: Malformed atom -- cannot properly free.\n");
        free(atom);
        return;
    }

    if (atom->type == atom_type_null
    || atom->type == atom_type_nullcondition
    || atom->type == atom_type_nullscope
    || atom->type == atom_type_Enull)
        ;

    else if (atom->type == atom_type_name) {
        NameAtom *name_atom = atom->atom;
        free(name_atom->name);
        free(name_atom);
    }
    else if (atom->type == atom_type_integer) {
        IntegerAtom *integer_atom = atom->atom;
        free(integer_atom);
    }
    else if (atom->type == atom_type_primitive) {
        PrimitiveAtom *primitive_atom = atom->atom;
        free(primitive_atom);
    }
    else if (atom->type == atom_type_functiondeclaration) {
        FunctionDeclarationAtom *functiondeclaration_atom = atom->atom;
        atomlist_free(functiondeclaration_atom->parameters);
        atomlist_free(functiondeclaration_atom->body);
        free(functiondeclaration_atom);
    }
    else if (atom->type == atom_type_function) {
        FunctionAtom *function_atom = atom->atom;
        if (function_atom->parameters)
            atomlist_free(function_atom->parameters);
        if (function_atom->body)
            atomlist_free(function_atom->body);
        free(function_atom->primitive);
        free(function_atom);
    }
    else if (atom->type == atom_type_scope) {
        ScopeAtom *scope_atom = atom->atom;
        atomlist_free(scope_atom->names);
        atomlist_free(scope_atom->binds);
        free(scope_atom);
    }
    else if (atom->type == atom_type_structinitializer) {
        StructInitializerAtom *structinitializer_atom = atom->atom;
        atomlist_free(structinitializer_atom->fields);
        free(structinitializer_atom);
    }
    else if (atom->type == atom_type_struct) {
        StructAtom *struct_atom = atom->atom;
        free(struct_atom);
    }
    else if (atom->type == atom_type_string) {
        StringAtom *string_atom = atom->atom;
        free(string_atom->str);
        free(string_atom);
    }
    else
        memorymanagement_error("atom_free: Trying to free atom of unknown type\n");

    free(atom);
}


/* Does not free any atoms. */
void atomlist_free(AtomList *lst) {
    if (!lst || !atomlist_is(lst))
        return;

    AtomListNode *node = lst->head;
    free(lst);

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

    free(node);
}
