#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "atom.h"
#include "error.h"
#include "util.h"
#include "memorymanagement.h"

#include "options.h"
extern Options *GlobalOptions;

// TODO -- Generates invalid memory behaviour
#define PEDANTIC_ERROR_CHECKING false



/*** GlobalAtomTable ***/
/*
The global atom table retains a pointer to every atom in use.
This allows for both atom caching (only applicable to immutable atoms;
atoms like scopes or functions are mutable) and eventual leakless atom freeing.
*/
AtomList *GlobalAtomTable;
AtomList *GlobalAtomTableMutable;
Atom *GlobalNullAtom, *GlobalNullConditionAtom, *GlobalNullScopeAtom, *GlobalENullAtom;

void globalatomtable_init() {
    GlobalAtomTable = atomlist_new(NULL);
    GlobalAtomTableMutable = atomlist_new(NULL);
    GlobalNullAtom = GlobalNullConditionAtom = GlobalNullScopeAtom = NULL;
}

void globalatomtable_print() {
    printf("\n=== GlobalAtomTable ===\n");

    AtomListNode *node = GlobalAtomTable->head;
    while (node) {
        printf(".> %s\n", atom_repr(node->atom));
        node = node->next;
    }
}






/*** Atom ***/
static Atom *atom_new_local(atom_type type, void *atom) {
    Atom *natom = malloc(sizeof *natom);

    if (!natom)
        return error_malloc("atom_new"), NULL;

    natom->type = type;
    natom->atom = atom;

    return natom;
}

Atom *atom_new(atom_type type, void *atom) {
    Atom *natom = atom_new_local(type, atom);

    if (natom->type == atom_type_scope || natom->type == atom_type_function)
        atomlist_push_front(GlobalAtomTableMutable, natom);
    else
        atomlist_push_front(GlobalAtomTable, natom);

    return natom;
}


bool atom_is(Atom *atom) {
    if (!atom)
        return error_atom("atom_is: Encountered NULL.\n"), false;

    if (atom->type == atom_type_null
    || atom->type == atom_type_nullcondition
    || atom->type == atom_type_nullscope
    || atom->type == atom_type_Enull)
        return true;

    if (!atom->atom)
        return error_atom("atom_is: Malformed atom.\n"), false;

    return true;
}

bool atom_purely(Atom *atom, atom_type type) {
    if (!atom_is(atom))
        return false;

    return atom->type == type;
}

bool atom_equal(Atom *atomA, Atom *atomB) {
    /* TODO Due to the GlobalAtomTable, equivalent atoms are cached and thereby have the same pointer. */
    return atomA == atomB;
}

const char *atom_repr(Atom *atom) {
    StringAtom *string_atom = atom_representation(atom)->atom;
    return string_atom->str;
}

// TODO
Atom *atom_struct_helper_repr(Atom *scope) {
    Atom *repr = atom_string_newfl("");
    ScopeAtom *scope_atom = scope->atom;
    AtomListNode *noden = scope_atom->names->head;
    AtomListNode *nodeb = scope_atom->binds->head;
    while (noden && nodeb) {
        repr = atom_string_concat(
            repr,
            //atom_representation(noden->atom),
            atom_representation(nodeb->atom)
        );
        noden = noden->next;
        nodeb = nodeb->next;
    }
    return repr;
}

Atom *atom_representation(Atom *atom) {
    if (!atom_is(atom))
        return atom_string_newfl("NULL (!)");

    if (atom->type == atom_type_null)
        return atom_string_newfl("Null");

    else if (atom->type == atom_type_nullcondition)
        return atom_string_newfl("NullCondition");

    else if (atom->type == atom_type_nullscope)
        return atom_string_newfl("NullScope");

    else if (atom->type == atom_type_Enull)
        return atom_string_newfl("ENull");

    else if (atom->type == atom_type_name) {
        NameAtom *name_atom = atom->atom;
        return atom_string_newcopy(name_atom->name);
    }

    else if (atom->type == atom_type_integer) {
        IntegerAtom *integer_atom = atom->atom;
        if (integer_atom->value >= 0 && integer_atom->value <= 9)
            return atom_string_fromlong(integer_atom->value);
        else
            return atom_string_concat3(
                atom_string_newfl("$"),
                atom_string_fromlong(integer_atom->value),
                atom_string_newfl(".")
            );
    }

    else if (atom->type == atom_type_primitive) {
        PrimitiveAtom *primitive_atom = atom->atom;
        return atom_string_concat(
            atom_string_newfl("'"),
            atom_string_fromchar(primitive_atom->c)
        );
    }

    else if (atom->type == atom_type_functiondeclaration) {
        FunctionDeclarationAtom *functiondeclaration_atom = atom->atom;
        return atom_string_concat7(
            atom_string_newfl("FunctionDeclaration(arity: "),
            atom_string_fromlong(functiondeclaration_atom->arity),
            atom_string_newfl(", parameters: "),
            atomlist_representation(functiondeclaration_atom->parameters),
            atom_string_newfl(", body: "),
            atomlist_representation(functiondeclaration_atom->body),
            atom_string_newfl(")")
        );
    }

    else if (atom->type == atom_type_function) {
        FunctionAtom *function_atom = atom->atom;
        if (function_atom->primitive == NULL)
            return atom_string_concat9(
                atom_string_newfl("Function(arity: "),
                atom_string_fromlong(function_atom->arity),
                atom_string_newfl(", parameters: "),
                atomlist_representation(function_atom->parameters),
                atom_string_newfl(", body: "),
                atomlist_representation(function_atom->body),
                atom_string_newfl(", scope: "),
                atom_representation(function_atom->scope),
                atom_string_newfl(")")
            );
        else
            return atom_string_concat5(
                atom_string_newfl("PrimitiveFunction(arity: "),
                atom_string_fromlong(function_atom->arity),
                atom_string_newfl(", primitive: "),
                atom_string_newcopy(function_atom->primitive),
                atom_string_newfl(")")
            );
    }

    else if (atom->type == atom_type_scope) {
        ScopeAtom *scope_atom = atom->atom;
        if (scope_atom->is_main)
            return atom_string_newfl("MainScope");
        if (scope_atom->is_selfref)
            return atom_string_concat5(
                atom_string_newfl("SelfRefScope(upper_scope: "),
                atom_representation(scope_atom->upper_scope),
                atom_string_newfl(", frozen: "),
                atom_string_fromlong(scope_atom->is_frozen),
                atom_string_newfl(")")
            );

        return atom_string_concat10(
            atom_string_newfl("Scope(names: "),
            atomlist_representation(scope_atom->names),
            atom_string_newfl(", binds: "),
            atomlist_representation(scope_atom->binds),
            atom_string_newfl(", upper_scope: "),
            atom_representation(scope_atom->upper_scope),
            atom_string_newfl(", frozen: "),
            atom_string_fromlong(scope_atom->is_frozen),
            atom_string_newfl(")@"),
            atom_string_fromlong((long) (void *) atom)
        );
    }

    else if (atom->type == atom_type_structinitializer) {
        StructInitializerAtom *structinitializer_atom = atom->atom;
        return atom_string_concat5(
            atom_string_newfl("StructInitializer(type: "),
            atom_representation(structinitializer_atom->type),
            atom_string_newfl(", fields: "),
            atomlist_representation(structinitializer_atom->fields),
            atom_string_newfl(")")
        );
    }

    else if (atom->type == atom_type_struct) {
        StructAtom *struct_atom = atom->atom;

        //TODO
        return atom_string_concat(
            atom_representation(struct_atom->type),
            atom_struct_helper_repr(struct_atom->scope)
        );

        return atom_string_concat5(
            atom_string_newfl("Struct("),
            atom_representation(struct_atom->type),
            atom_string_newfl(", "),
            atom_representation(struct_atom->scope),
            atom_string_newfl(")")
        );
    }

    else if (atom->type == atom_type_string) {
        return atom_string_concat3(
            atom_string_newfl("\""),
            atom,
            atom_string_newfl("\"")
        );
    }

    else return atom_string_newfl("UnknownAtom");
}

// start_GlobalAtomTable_cashing ... end_GlobalAtomTable_cashing
#define start_GlobalAtomTable_cashing { \
    AtomListNode *node = GlobalAtomTable->head; \
    while (node) { Atom *atom = node->atom;
#define end_GlobalAtomTable_cashing node = node->next; } }

Atom *atom_null_new() {
    if (!GlobalNullAtom)
        GlobalNullAtom = atom_new(atom_type_null, NULL);

    return GlobalNullAtom;
}

bool atom_null_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_null;
}

Atom *atom_nullcondition_new() {
    if (!GlobalNullConditionAtom)
        GlobalNullConditionAtom = atom_new(atom_type_nullcondition, NULL);

    return GlobalNullConditionAtom;
}

bool atom_nullcondition_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_nullcondition;
}

Atom *atom_nullscope_new() {
    if (!GlobalNullScopeAtom)
        GlobalNullScopeAtom = atom_new(atom_type_nullscope, NULL);

    return GlobalNullScopeAtom;
}

bool atom_nullscope_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_nullscope;
}

Atom *atom_Enull_new() {
    if (!GlobalENullAtom)
        GlobalENullAtom = atom_new(atom_type_Enull, NULL);

    return GlobalENullAtom;
}

bool atom_Enull_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_Enull;
}

Atom *atom_name_new(char *name) {
    start_GlobalAtomTable_cashing
        if (atom_name_is(atom)) {
            NameAtom *name_atom = atom->atom;
            if (strcmp(name_atom->name, name) == 0) {
                free(name);
                return atom;
            }
        }
    end_GlobalAtomTable_cashing

    // atom not found, create new atom
    NameAtom *name_atom = malloc(sizeof *name_atom);

    if (!name_atom)
        return error_malloc("atom_name_new"), NULL;

    name_atom->name = name;

    return atom_new(atom_type_name, name_atom);
}

bool atom_name_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_name;

    if (!atom_is(atom) || atom->type != atom_type_name)
        return false;

    NameAtom *name_atom = atom->atom;

    if (!name_atom->name)
        return error_atom("atom_name_is: Malformed NameAtom.\n"), false;

    return true;
}

Atom *atom_integer_new(integer value) {
    start_GlobalAtomTable_cashing
        if (atom_integer_is(atom)) {
            IntegerAtom *integer_atom = atom->atom;
            if (integer_atom->value == value)
                /* no freeing required */
                return atom;
        }
    end_GlobalAtomTable_cashing



    IntegerAtom *integer_atom = malloc(sizeof *integer_atom);

    if (!integer_atom)
        return error_malloc("atom_integer_new"), NULL;

    integer_atom->value = value;

    return atom_new(atom_type_integer, integer_atom);
}

bool atom_integer_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_integer;
}

Atom *atom_primitive_new(char c) {
    start_GlobalAtomTable_cashing
        if (atom_primitive_is(atom)) {
            PrimitiveAtom *primitive_atom = atom->atom;
            if (primitive_atom->c == c)
                /* no freeing required */
                return atom;
        }
    end_GlobalAtomTable_cashing



    PrimitiveAtom *primitive_atom = malloc(sizeof *primitive_atom);

    if (!primitive_atom)
        return error_malloc("atom_primitive_new"), NULL;

    primitive_atom->c = c;

    return atom_new(atom_type_primitive, primitive_atom);
}

bool atom_primitive_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_primitive;
}

Atom *atom_functiondeclaration_new(int arity, AtomList *parameters, AtomList *body) {
    /*start_GlobalAtomTable_cashing
        if (atom_functiondeclaration_is(atom)) {
            FunctionDeclarationAtom *functiondeclaration_atom = atom->atom;
            if (functiondeclaration_atom->arity == arity
            && atomlist_equal(functiondeclaration_atom->parameters, parameters)
            && atomlist_equal(functiondeclaration_atom->body, body))
                return atom;
        }
    end_GlobalAtomTable_cashing TODO*/

    if (!atomlist_is(parameters) || !atomlist_purely(parameters, atom_type_name) || !atomlist_is(body))
        return error_atom("atom_functiondeclaration_new: Invalid arguments.\n"), NULL;

    FunctionDeclarationAtom *functiondeclaration_atom = malloc(sizeof *functiondeclaration_atom);

    if (!functiondeclaration_atom)
        return error_malloc("atom_functiondeclaration_new"), NULL;

    functiondeclaration_atom->arity = arity;
    functiondeclaration_atom->parameters = parameters;
    functiondeclaration_atom->body = body;

    return atom_new(atom_type_functiondeclaration, functiondeclaration_atom);
}

bool atom_functiondeclaration_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_functiondeclaration;

    if (!atom_is(atom) || atom->type != atom_type_functiondeclaration)
        return false;

    FunctionDeclarationAtom *functiondeclaration_atom = atom->atom;
    if (functiondeclaration_atom->arity < 0
    || !atomlist_is(functiondeclaration_atom->parameters)
    || !atomlist_purely(functiondeclaration_atom->parameters, atom_type_name)
    || !atomlist_is(functiondeclaration_atom->body))
        return error_atom("atom_functiondeclaration_is: Malformed FunctionDeclarationAtom.\n"), false;

    return true;
}

Atom *atom_function_new(int arity, AtomList *parameters, AtomList *body, Atom *scope, char *primitive) {
    if (!atom_scope_is(scope) && !atom_nullscope_is(scope))
        return error_atom("atom_function_new: Given invalid scope ScopeAtom.\n"), NULL;

    if (primitive && (parameters != NULL || body != NULL))
            return error_atom("atom_function_new: Invalid primitive initialization.\n"), NULL;

    if (!primitive && (!atomlist_is(parameters) || !atomlist_is(body)))
        return error_atom("atom_function_new: Given invalid parameters AtomList and/or body AtomList and/or scope ScopeAtom.\n"), NULL;

    FunctionAtom *function_atom = malloc(sizeof *function_atom);

    if (!function_atom)
        return error_malloc("atom_function_new"), NULL;

    function_atom->arity = arity;
    function_atom->parameters = parameters;
    function_atom->body = body;
    function_atom->scope = scope;
    function_atom->primitive = primitive;

    return atom_new(atom_type_function, function_atom);
}

bool atom_function_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_function;

    if (!atom_is(atom) || atom->type != atom_type_function)
        return false;

    FunctionAtom *function_atom = atom->atom;

    if (!atom_scope_is(function_atom->scope) && !atom_nullscope_is(function_atom->scope))
        return error_atom("atom_function_is: Malformed FunctionAtom.\n"), false;

    if (function_atom->primitive) {
        if (atomlist_is(function_atom->parameters) || atomlist_is(function_atom->body))
            return error_atom("atom_function_is: Malformed FunctionAtom.\n"), false;
    }
    else {
        if (!atomlist_is(function_atom->parameters) || !atomlist_is(function_atom->body))
            return error_atom("atom_function_is: Malformed FunctionAtom.\n"), false;
    }

    return true;
}

Atom *atom_scope_new(AtomList *names, AtomList *binds, Atom *upper_scope) {
    /* Scope atoms are mutable and therefor shall not be cached. */

    if (!atomlist_is(names) || !atomlist_purely(names, atom_type_name))
        return error_atom("atom_scope_new: Given invalid names AtomList (or one which conatins non-NameAtom elements).\n"), NULL;
    if (!atomlist_is(binds))
        return error_atom("atom_scope_new: Given invalid binds AtomList.\n"), NULL;

    if (!atom_scope_is(upper_scope) && !atom_nullscope_is(upper_scope))
        return error_atom("atom_scope_new: Given invalid upper_scope Atom.\n"), NULL;

    ScopeAtom *scope_atom = malloc(sizeof *scope_atom);

    if (!scope_atom)
        return error_malloc("atom_scope_new"), NULL;

    scope_atom->names = names;
    scope_atom->binds = binds;
    scope_atom->upper_scope = upper_scope;
    scope_atom->is_main = false;
    scope_atom->is_selfref = false;
    scope_atom->is_frozen = false;

    return atom_new(atom_type_scope, scope_atom);
}

Atom *atom_scope_freeze(Atom *scope) {
    if (!atom_scope_is(scope))
        return error_atom("atom_scope_freeze: Expected ScopeAtom, got %s.\n", atom_repr(scope)), NULL;

    ScopeAtom *original_scope_atom = scope->atom;
    if (original_scope_atom->is_frozen)
        return error_atom("atom_scope_freeze: Too cold.\n"), NULL;
    original_scope_atom->is_frozen = true;

    atomlist_remove_by_pointer(GlobalAtomTableMutable, scope);

    start_GlobalAtomTable_cashing
        if (atom_scope_is(atom)) {
            ScopeAtom *scope_atom = atom->atom;
            if (scope_atom->is_frozen
            && atomlist_equal(scope_atom->names, original_scope_atom->names)
            && atomlist_equal(scope_atom->binds, original_scope_atom->binds)
            && atom_equal(scope_atom->upper_scope, original_scope_atom->upper_scope)) {
                //TODO WAKKA
                //printf("HIT\n");
                atom_free(scope);
                return atom;
            }
        }
    end_GlobalAtomTable_cashing

    atomlist_push_front(GlobalAtomTable, scope);
    return scope;
}

Atom *atom_scope_new_empty() {
    return atom_scope_new(atomlist_new(NULL), atomlist_new(NULL), atom_nullscope_new());
}

bool atom_scope_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_scope;

    if (PEDANTIC_ERROR_CHECKING) {
        if (!atom_is(atom) || atom->type != atom_type_scope)
            return false;

        ScopeAtom *scope_atom = atom->atom;

        if (!atomlist_is(scope_atom->names) || !atomlist_purely(scope_atom->names, atom_type_name) || !atomlist_is(scope_atom->binds))
            return error_atom("atom_scope_is: Malformed ScopeAtom.\n"), false;

        // trying to implement (at least partial) tail recursion
        if (!atom_nullscope_is(scope_atom->upper_scope) && !atom_scope_is(scope_atom->upper_scope))
            return error_atom("atom_scope_is: Malformed ScopeAtom.\n"), false;

        return true;
    }
}

void atom_scope_setflag_ismain(Atom *atom, bool flg) {
    if (!atom_scope_is(atom))
        { error_atom("atom_scope_setflag_ismain: Expected ScopeAtom.\n"); return; }

    ScopeAtom *scope_atom = atom->atom;

    if (scope_atom->is_frozen)
        { error_atom("atom_scope_setflag_ismain: Scope is frozen.\n"); return; }

    scope_atom->is_main = flg;
}

bool atom_scope_getflag_ismain(Atom *atom) {
    if (!atom_scope_is(atom))
        return error_atom("atom_scope_getflag_ismain: Expected ScopeAtom.\n"), false;

    ScopeAtom *scope_atom = atom->atom;

    if (scope_atom->is_frozen)
        return error_atom("atom_scope_getflag_ismain: Scope is frozen.\n");

    return scope_atom->is_main;
}

void atom_scope_setflag_isselfref(Atom *atom, bool flg) {
    if (!atom_scope_is(atom))
        { error_atom("atom_scope_setflag_isselfref: Expected ScopeAtom.\n"); return; }

    ScopeAtom *scope_atom = atom->atom;

    if (scope_atom->is_frozen)
        { error_atom("atom_scope_setflag_isselfref: Scope is frozen.\n"); return; }

    scope_atom->is_selfref = flg;
}

bool atom_scope_getflag_isselfref(Atom *atom) {
    if (!atom_scope_is(atom))
        return error_atom("atom_scope_getflag_isselfref: Expected ScopeAtom.\n"), false;

    ScopeAtom *scope_atom = atom->atom;

    if (scope_atom->is_frozen)
        return error_atom("atom_scope_getflag_isselfref: Scope is frozen.\n");

    return scope_atom->is_selfref;
}

bool atom_scope_push(Atom *scope, Atom *name, Atom *bind) {
    if (!atom_scope_is(scope) || !atom_name_is(name) || !atom_is(bind))
        return error_atom("atom_scope_push: Given invalid scope ScopeAtom and/or name NameAtom and/or bind Atom.\n"), false;

    ScopeAtom *scope_atom = scope->atom;

    if (scope_atom->is_frozen)
        return error_atom("atom_scope_push: Scope is frozen."), false;

    AtomListNode *names_node = scope_atom->names->head;
    AtomListNode *binds_node = scope_atom->binds->head;
    while (names_node && binds_node) {
        if (!atom_name_is(names_node->atom))
            return error_atom("atom_scope_push: Detected invalidity of ScopeAtom's names (%s).\n", atom_repr(names_node->atom)), false;

        if (atom_equal(names_node->atom, name))
            return error_atom("atom_scope_push: Attempt at name rebinding (%s already bound as %s).\n", atom_repr(name), atom_repr(binds_node->atom)), false;

        names_node = names_node->next;
        binds_node = binds_node->next;
    }

    atomlist_push(scope_atom->names, name);
    atomlist_push(scope_atom->binds, bind);

    return true;
}

Atom *atom_scope_unbind(Atom *scope, Atom *name) {
    if (!atom_scope_is(scope) || !atom_name_is(name))
        return error("atom_scope_unbind: Given invalid scope ScopeAtom and/or name NameAtom."), NULL;

    ScopeAtom *scope_atom = scope->atom;
    AtomListNode *names_node = scope_atom->names->head;
    AtomListNode *binds_node = scope_atom->binds->head;
    while (names_node && binds_node) {
        if (!atom_name_is(names_node->atom))
            return error_atom("atom_scope_unbind: Detected invalidity of ScopeAtom's names.\n"), NULL;

        if (atom_equal(names_node->atom, name))
            return binds_node->atom;

        names_node = names_node->next;
        binds_node = binds_node->next;
    }

    if (atom_nullscope_is(scope_atom->upper_scope))
        return error_atom("atom_scope_unbind: Could not find name bind (%s).\n", atom_repr(name)), NULL;

    /* Perfect use of tail recursion! */
    return atom_scope_unbind(scope_atom->upper_scope, name);
}

Atom *atom_scope_new_fake(Atom *scope) { return atom_scope_new(atomlist_new(NULL), atomlist_new(NULL), scope); }
Atom *atom_scope_new_inherits(Atom *scope) { return atom_scope_new(atomlist_new(NULL), atomlist_new(NULL), scope); }

Atom *atom_structinitializer_new(Atom *type, AtomList *fields) {
    start_GlobalAtomTable_cashing
        if (atom_structinitializer_is(atom)) {
            StructInitializerAtom *structinitializer_atom = atom->atom;
            if (atom_equal(structinitializer_atom->type, type) && atomlist_equal(structinitializer_atom->fields, fields)) {
                atomlist_free(fields);
                return atom;
            }
        }
    end_GlobalAtomTable_cashing

    if (!atom_is(type) || !fields)
        return error_atom("atom_structinitializer_new: Given invalid struct type or fields AtomList.\n"), NULL;

    AtomListNode *node = fields->head;
    while (node) {
        if (!atom_name_is(node->atom))
            return error_atom("atom_structinitializer_new: Found non-NameAtom in fields AtomList.\n"), NULL;

        node = node->next;
    }

    StructInitializerAtom *structinitializer_atom = malloc(sizeof *structinitializer_atom);
    if (!structinitializer_atom)
        return error_malloc("atom_structinitializer_new"), NULL;

    structinitializer_atom->type = type;
    structinitializer_atom->fields = fields;

    return atom_new(atom_type_structinitializer, structinitializer_atom);
}

bool atom_structinitializer_is(Atom *atom) {
    if (!atom_is(atom) || atom->type != atom_type_structinitializer)
        return false;

    StructInitializerAtom *structinitializer_atom = atom->atom;
    return !!structinitializer_atom->fields;
}

Atom *atom_struct_new(Atom *type, Atom *scope) {
    start_GlobalAtomTable_cashing
        if (atom_struct_is(atom)) {
            StructAtom *struct_atom = atom->atom;
            if (atom_equal(type, struct_atom->type)
            && atom_equal(scope, struct_atom->scope))
                return atom;
        }
    end_GlobalAtomTable_cashing

    if (!atom_name_is(type))
        return error_atom("atom_struct_new: Given invalid type.\n"), NULL;

    if (!atom_scope_is(scope))
        return error_atom("atom_struct_new: Given invalid scope.\n"), NULL;

    // TODO check frozen

    StructAtom *struct_atom = malloc(sizeof *struct_atom);
    if (!struct_atom)
        return error_malloc("atom_struct_new"), NULL;

    struct_atom->type = type;
    struct_atom->scope = scope;

    return atom_new(atom_type_struct, struct_atom);
}

bool atom_struct_is(Atom *atom) {
    if (!atom_is(atom) || atom->type != atom_type_struct)
        return false;

    StructAtom *struct_atom = atom->atom;
    return atom_is(struct_atom->type) && atom_scope_is(struct_atom->scope);
}

Atom *atom_string_new(char *str) {
    start_GlobalAtomTable_cashing
        if (atom_string_is(atom)) {
            StringAtom *string_atom = atom->atom;
            if (strcmp(string_atom->str, str) == 0) {
                //TODO printf("StringCacheHit \"%s\"\n", str);
                free(str);
                return atom;
            }
        }
    end_GlobalAtomTable_cashing

    StringAtom *string_atom = malloc(sizeof *string_atom);
    if (!string_atom)
        return error_malloc("atom_string_new"), NULL;

    string_atom->str = str;

    return atom_new(atom_type_string, string_atom);
}

bool atom_string_is(Atom *atom) {
    return atom_is(atom) && atom->type == atom_type_string;
}

Atom *atom_string_concat(Atom *atomA, Atom *atomB) {
    if (!atom_string_is(atomA) || !atom_string_is(atomB))
        return error_atom("atom_string_concat: Expected two atoms.\n"), NULL;

    StringAtom *string_atomA = atomA->atom, *string_atomB = atomB->atom;
    const char *strA = string_atomA->str, *strB = string_atomB->str;

    char *str = malloc((strlen(strA)+strlen(strB)+1) * sizeof *str);
    if (!str)
        return error_malloc("atom_string_concat"), NULL;

    sprintf(str, "%s%s", strA, strB);

    return atom_string_new(str);
}

Atom *atom_string_concat3(Atom *atomA, Atom *atomB, Atom *atomC) {
    return atom_string_concat(atomA, atom_string_concat(atomB, atomC));
}
Atom *atom_string_concat5(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE) {
    return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atomE))));
}
Atom *atom_string_concat6(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE, Atom *atomF) {
    return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atom_string_concat(atomE, atomF)))));
}
Atom *atom_string_concat7(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE, Atom *atomF, Atom *atomG) {
    return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atom_string_concat(atomE, atom_string_concat(atomF, atomG))))));
}
Atom *atom_string_concat9(
    Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE,
    Atom *atomF, Atom *atomG, Atom *atomH, Atom *atomI
) {
    return atom_string_concat(atomA, atom_string_concat(atomB,
    atom_string_concat(atomC, atom_string_concat(atomD,
    atom_string_concat(atomE, atom_string_concat(atomF,
    atom_string_concat(atomG, atom_string_concat(atomH,
    atomI))))))));
}

Atom *atom_string_concat10(
    Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE,
    Atom *atomF, Atom *atomG, Atom *atomH, Atom *atomI, Atom *atomJ
) {
    return atom_string_concat(atomA, atom_string_concat(atomB,
    atom_string_concat(atomC, atom_string_concat(atomD,
    atom_string_concat(atomE, atom_string_concat(atomF,
    atom_string_concat(atomG, atom_string_concat(atomH,
    atom_string_concat(atomI, atomJ)))))))));
}
Atom *atom_string_concat11(
    Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE,
    Atom *atomF, Atom *atomG, Atom *atomH, Atom *atomI, Atom *atomJ,
    Atom *atomK
) {
    return atom_string_concat(atomA, atom_string_concat(atomB,
    atom_string_concat(atomC, atom_string_concat(atomD,
    atom_string_concat(atomE, atom_string_concat(atomF,
    atom_string_concat(atomG, atom_string_concat(atomH,
    atom_string_concat(atomI, atom_string_concat(atomJ, atomK))))))))));
}

Atom *atom_string_fromlong(long n) {
    char *str = malloc((snprintf(NULL, 0, "%ld", n)+1) * sizeof *str);
    sprintf(str, "%ld", n);
    return atom_string_new(str);
}

Atom *atom_string_fromchar(char c) {
    char *str = malloc(2 * sizeof *str);
    str[0] = c;
    str[1] = '\0';
    return atom_string_new(str);
}

Atom *atom_string_newfl(const char *str) {
    char *new_str = malloc((strlen(str)+1) * sizeof *str);
    if (!new_str)
        return error_malloc("atom_string_newfl"), NULL;
    sprintf(new_str, "%s", str);

    return atom_string_new(new_str);
}

Atom *atom_string_newcopy(const char *str) {
    return atom_string_newfl(str);
}

#undef start_GlobalAtomTable_cashing
#undef end_GlobalAtomTable_cashing

/* === AtomList === */

static AtomListNode *atomlistnode_new(Atom *atom, AtomListNode *next);

Atom *atomlist_representation(AtomList *lst) {
    if (!atomlist_is(lst))
        return error_atomlist("atomlist_representation: Expected AtomList.\n"), NULL;

    if (!lst->head)
        return atom_string_newfl("[]");

    Atom *repr = atom_string_newfl("[");
    AtomListNode *node = lst->head;
    while (node) {
        if (node->next)
            repr = atom_string_concat3(repr, atom_representation(node->atom), atom_string_newfl(", "));
        else
            repr = atom_string_concat3(repr, atom_representation(node->atom), atom_string_newfl("]"));
        node = node->next;
    }

    return repr;
}

// TODO
const char *atomlist_str(AtomList *lst) {
    StringAtom *string_atom = atomlist_representation(lst)->atom;
    return string_atom->str;
}

AtomList *atomlist_new(AtomListNode *head) {
    AtomList *lst = malloc(sizeof *lst);

    if (!lst)
        return error_malloc("atomlist_new"), NULL;

    lst->head = head;

    return lst;
}

bool atomlist_is(AtomList *lst) {
    if (!lst)
        return error_atomlist("atomlist_is: Malformed atom.\n"), false;

    return true;
}



static AtomListNode *atomlistnode_new(Atom *atom, AtomListNode *next) {
    if (!atom_is(atom))
        return error_atomlist("atomlistnode_new: Given invalid atom.\n"), NULL;

    AtomListNode *node = malloc(sizeof *node);

    if (!node)
        return error_malloc("atomlistnode_new"), NULL;

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

// TODO Atom *atomlist_pop()
/*
Atom *atomlist_pop_back(AtomList *lst) {
    if (!atomlist_is(lst)
        return NULL;

    if (!lst->head)
        return error_atomlist("atomlist_pop_back: List length is zero.\n"), NULL;

    AtomListNode *node = lst->head;
    while
}*/

Atom *atomlist_pop_front(AtomList *lst) {
    if (!atomlist_is(lst))
        return NULL;

    if (!lst->head)
        return error_atomlist("atomlist_pop_front: List length is zero.\n"), NULL;

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
        if (!atom_purely(node->atom, type))
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

    error_atomlist("atomlist_remove_by_pointer: Did not find %s.\n", atom_repr(atom));
}
