#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "atom.h"
#include "atomlist.h"
#include "debug.h"
#include "util.h"
#include "memorymanagement.h"

#include "Opt.h"
extern Opt GlobOpt;


/* GlobalAtomTable

    The global atom table retains a pointer to every atom in use.
    It allows for both atom anti-aliasing (only applicable to immutable atoms;
    mutable atoms cannot be anti-aliased) and eventual leakless freeing of all atoms.
*/

AtomList *GlobalAtomTable;
AtomList *GlobalAtomTableMutable;
Atom *GlobalNullAtom, *GlobalNullConditionAtom, *GlobalNullScopeAtom, *GlobalENullAtom;
Atom *ImportedSource;

void globalatomtable_init() {
    GlobalAtomTable = atomlist_new(NULL);
    GlobalAtomTableMutable = atomlist_new(NULL);

    GlobalNullAtom = GlobalNullConditionAtom = GlobalNullScopeAtom = NULL;
    ImportedSource = atom_scope_new_empty();

    #include "../stdlib/krrp_stdlib.c_fragment"
}


static Atom *_atom_new(atom_type type, void *atom) {
    Atom *natom = mm_malloc("_atom_new", sizeof *natom);

    natom->type = type;
    natom->atom = atom;

    return natom;
}

Atom *atom_new(atom_type type, void *atom) {
    Atom *natom = _atom_new(type, atom);

    if (natom->type == atom_type_scope
    || natom->type == atom_type_function
    || natom->type == atom_type_list)
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

bool atom_equal(Atom *atomA, Atom *atomB) {
    /* Due to the GlobalAtomTable, equivalent atoms are anti-aliased and thereby
    equal to one another iff their pointers are; identity and equivalence are
    indistinguishable. */
    return atomA == atomB;
}


const char *atom_repr(Atom *atom) {
    StringAtom *string_atom = atom_representation(atom)->atom;
    return string_atom->str;
}

static Atom *atom_struct_representation(Atom *scope) {
    Atom *repr = atom_string_newfl("");
    ScopeAtom *scope_atom = scope->atom;
    AtomListNode *noden = scope_atom->names->head;
    AtomListNode *nodeb = scope_atom->binds->head;
    while (noden && nodeb) {
        repr = atom_string_concat(
            repr,
            atom_representation(nodeb->atom)
        );
        noden = noden->next;
        nodeb = nodeb->next;
    }
    return repr;
}

Atom *atom_representation(Atom *atom) {
    if (!atom_is(atom))
        return atom_string_newfl("NULLPOINTER");

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

        if (GlobOpt.string_view)
            return atom_string_fromchar(integer_atom->value & 0xFF);

        else {
            if (integer_atom->value >= 0 && integer_atom->value <= 9)
                return atom_string_fromlong(integer_atom->value);
            else
                return atom_string_concat3(
                    atom_string_newfl("$"),
                    atom_string_fromlong(integer_atom->value),
                    atom_string_newfl(".")
                );
        }

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

        if (GlobOpt.string_view)
            return atom_struct_representation(struct_atom->scope);
        else
            return atom_string_concat(
                atom_representation(struct_atom->type),
                atom_struct_representation(struct_atom->scope)
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


// start_GlobalAtomTable_antialiasing ... end_GlobalAtomTable_antialiasing
#define start_GlobalAtomTable_antialiasing { \
    AtomListNode *node = GlobalAtomTable->head; \
    while (node) { Atom *atom = node->atom;
#define end_GlobalAtomTable_antialiasing node = node->next; } }


bool atom_is_of_type(Atom *atom, atom_type type) {
    return atom_is(atom) && atom->type == type;
}

#define MAKE__ATOM_NULLX_NEW(x, X) \
    Atom *atom_##x##_new() { \
        if (!Global##X##Atom) \
            Global##X##Atom = atom_new(atom_type_##x, NULL); \
        return Global##X##Atom; }

#define MAKE__ATOM_NULLX_IS(x) \
    bool atom_##x##_is(Atom *atom) { \
        return atom_is_of_type(atom, atom_type_##x); }

#define MAKE__ATOM_NULLX(x, X) \
    MAKE__ATOM_NULLX_NEW(x, X) \
    MAKE__ATOM_NULLX_IS(x)

MAKE__ATOM_NULLX(null, Null)
MAKE__ATOM_NULLX(nullcondition, NullCondition)
MAKE__ATOM_NULLX(nullscope, NullScope)
MAKE__ATOM_NULLX(Enull, ENull)

#undef MAKE__ATOM_NULLX_NEW
#undef MAKE__ATOM_NULLX_IS
#undef MAKE__ATOM_NULLX


Atom *atom_name_new(char *name) {
    start_GlobalAtomTable_antialiasing
        if (atom_name_is(atom)) {
            NameAtom *name_atom = atom->atom;
            if (strcmp(name_atom->name, name) == 0) {
                mm_free("atom_name_new", name);
                return atom;
            }
        }
    end_GlobalAtomTable_antialiasing


    NameAtom *name_atom = mm_malloc("atom_name_new", sizeof *name_atom);
    name_atom->name = name;

    return atom_new(atom_type_name, name_atom);
}

bool atom_name_is(Atom *atom) {
    return atom_is_of_type(atom, atom_type_name);
}


Atom *atom_integer_new(integer value) {
    start_GlobalAtomTable_antialiasing
        if (atom_integer_is(atom)) {
            IntegerAtom *integer_atom = atom->atom;
            if (integer_atom->value == value)
                /* no mm_freeing required */
                return atom;
        }
    end_GlobalAtomTable_antialiasing


    IntegerAtom *integer_atom = mm_malloc("atom_integer_new", sizeof *integer_atom);
    integer_atom->value = value;

    return atom_new(atom_type_integer, integer_atom);
}

bool atom_integer_is(Atom *atom) {
    return atom_is_of_type(atom, atom_type_integer);
}


Atom *atom_primitive_new(char c) {
    start_GlobalAtomTable_antialiasing
        if (atom_primitive_is(atom)) {
            PrimitiveAtom *primitive_atom = atom->atom;
            if (primitive_atom->c == c)
                /* no mm_freeing required */
                return atom;
        }
    end_GlobalAtomTable_antialiasing


    PrimitiveAtom *primitive_atom = mm_malloc("atom_primitive_new", sizeof *primitive_atom);
    primitive_atom->c = c;

    return atom_new(atom_type_primitive, primitive_atom);
}

bool atom_primitive_is(Atom *atom) {
    return atom_is_of_type(atom, atom_type_primitive);
}


Atom *atom_functiondeclaration_new(int arity, AtomList *parameters, AtomList *body) {
    if (!atomlist_is(parameters)
    || !atomlist_purely(parameters, atom_type_name)
    || !atomlist_is(body))
        return error_atom("atom_functiondeclaration_new: Invalid arguments.\n"), NULL;

    FunctionDeclarationAtom *functiondeclaration_atom
        = mm_malloc("atom_functiondeclaration_new", sizeof *functiondeclaration_atom);
    functiondeclaration_atom->arity = arity;
    functiondeclaration_atom->parameters = parameters;
    functiondeclaration_atom->body = body;

    return atom_new(atom_type_functiondeclaration, functiondeclaration_atom);
}

bool atom_functiondeclaration_is(Atom *atom) {
    return atom_is_of_type(atom, atom_type_functiondeclaration);
}

Atom *atom_function_new(int arity, AtomList *parameters, AtomList *body, Atom *scope, char *primitive) {
    if (!atom_scope_is(scope) && !atom_nullscope_is(scope))
        return error_atom("atom_function_new: Given invalid scope ScopeAtom.\n"), NULL;

    if (primitive && (parameters != NULL || body != NULL))
            return error_atom("atom_function_new: Invalid primitive initialization.\n"), NULL;

    if (!primitive && (!atomlist_is(parameters) || !atomlist_is(body)))
        return error_atom("atom_function_new: Given invalid parameters AtomList and/or body AtomList and/or scope ScopeAtom.\n"), NULL;

    FunctionAtom *function_atom = mm_malloc("atom_function_new", sizeof *function_atom);
    function_atom->arity = arity;
    function_atom->parameters = parameters;
    function_atom->body = body;
    function_atom->scope = scope;
    function_atom->primitive = primitive;

    return atom_new(atom_type_function, function_atom);
}

bool atom_function_is(Atom *atom) {
    if (!atom_is_of_type(atom, atom_type_function))
        return false;

    return true;
}

Atom *atom_scope_new(AtomList *names, AtomList *binds, Atom *upper_scope) {
    /* Scope atoms are mutable and therefor shall not be anti-aliased. */

    if (!atomlist_is(names) || !atomlist_purely(names, atom_type_name))
        return error_atom("atom_scope_new: Given invalid names AtomList (or one which conatins non-NameAtom elements).\n"), NULL;
    if (!atomlist_is(binds))
        return error_atom("atom_scope_new: Given invalid binds AtomList.\n"), NULL;

    if (!atom_scope_is(upper_scope) && !atom_nullscope_is(upper_scope))
        return error_atom("atom_scope_new: Given invalid upper_scope Atom.\n"), NULL;

    ScopeAtom *scope_atom = mm_malloc("atom_scope_new", sizeof *scope_atom);
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

    start_GlobalAtomTable_antialiasing
        if (atom_scope_is(atom)) {
            ScopeAtom *scope_atom = atom->atom;
            if (scope_atom->is_frozen
            && atomlist_equal(scope_atom->names, original_scope_atom->names)
            && atomlist_equal(scope_atom->binds, original_scope_atom->binds)
            && atom_equal(scope_atom->upper_scope, original_scope_atom->upper_scope))
                return atom_free(scope), atom;
        }
    end_GlobalAtomTable_antialiasing


    atomlist_push_front(GlobalAtomTable, scope);
    return scope;
}

Atom *atom_scope_new_empty() {
    return atom_scope_new(atomlist_new(NULL), atomlist_new(NULL), atom_nullscope_new());
}

Atom *atom_scope_new_double_empty() {
    return atom_scope_new(atomlist_new(NULL), atomlist_new(NULL), atom_scope_new_empty());
}

bool atom_scope_is(Atom *atom) {
    if (!atom_is_of_type(atom, atom_type_scope))
        return false;

    if (GlobOpt.pedantic) {
        ScopeAtom *scope_atom = atom->atom;

        if (!atomlist_is(scope_atom->names) || !atomlist_purely(scope_atom->names, atom_type_name) || !atomlist_is(scope_atom->binds))
            return error_atom("atom_scope_is: Malformed ScopeAtom.\n"), false;

        // trying to implement (at least partial) tail recursion
        if (!atom_nullscope_is(scope_atom->upper_scope) && !atom_scope_is(scope_atom->upper_scope))
            return error_atom("atom_scope_is: Malformed ScopeAtom.\n"), false;
    }

    return true;
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
        return error_atom("atom_scope_getflag_ismain: Scope is frozen.\n"), false;

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
        return error_atom("atom_scope_getflag_isselfref: Scope is frozen.\n"), false;

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
        return error_atom("atom_scope_unbind: Given invalid scope ScopeAtom and/or name NameAtom.\n"), NULL;

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

bool atom_scope_contains_bind(Atom *scope, Atom *name) {
    if (!atom_scope_is(scope) || !atom_name_is(name))
        return error_atom("atom_scope_contains_bind: Expected scope and name.\n"), false;

    ScopeAtom *scope_atom = scope->atom;
    AtomListNode *names_node = scope_atom->names->head;
    while (names_node) {
        if (atom_equal(names_node->atom, name))
            return true;

        names_node = names_node->next;
    }

    return false;
}

Atom *atom_scope_new_fake(Atom *scope) { return atom_scope_new(atomlist_new(NULL), atomlist_new(NULL), scope); }
Atom *atom_scope_new_inherits(Atom *scope) { return atom_scope_new(atomlist_new(NULL), atomlist_new(NULL), scope); }

Atom *atom_structinitializer_new(Atom *type, AtomList *fields) {
    start_GlobalAtomTable_antialiasing
        if (atom_structinitializer_is(atom)) {
            StructInitializerAtom *structinitializer_atom = atom->atom;
            if (atom_equal(structinitializer_atom->type, type)
            && atomlist_equal(structinitializer_atom->fields, fields))
                return atomlist_free(fields), atom;
        }
    end_GlobalAtomTable_antialiasing


    if (!atom_is(type) || !fields)
        return error_atom("atom_structinitializer_new: Given invalid struct type or fields AtomList.\n"), NULL;

    AtomListNode *node = fields->head;
    while (node) {
        if (!atom_name_is(node->atom))
            return error_atom("atom_structinitializer_new: Found non-NameAtom in fields AtomList.\n"), NULL;

        node = node->next;
    }

    StructInitializerAtom *structinitializer_atom = mm_malloc("atom_structinitializer_new", sizeof *structinitializer_atom);
    structinitializer_atom->type = type;
    structinitializer_atom->fields = fields;

    return atom_new(atom_type_structinitializer, structinitializer_atom);
}

bool atom_structinitializer_is(Atom *atom) {
    if (!atom_is_of_type(atom, atom_type_structinitializer))
        return false;

    StructInitializerAtom *structinitializer_atom = atom->atom;
    return !!structinitializer_atom->fields;
}

Atom *atom_struct_new(Atom *type, Atom *scope) {
    start_GlobalAtomTable_antialiasing
        if (atom_struct_is(atom)) {
            StructAtom *struct_atom = atom->atom;
            if (atom_equal(type, struct_atom->type)
            && atom_equal(scope, struct_atom->scope))
                return atom;
        }
    end_GlobalAtomTable_antialiasing


    if (!atom_name_is(type))
        return error_atom("atom_struct_new: Given invalid type.\n"), NULL;

    if (!atom_scope_is(scope))
        return error_atom("atom_struct_new: Given invalid scope.\n"), NULL;

    if (!(((ScopeAtom *) scope->atom)->is_frozen))
        return error_atom("atom_struct_new: Thawed scope.\n"), NULL;

    StructAtom *struct_atom = mm_malloc("atom_struct_new", sizeof *struct_atom);
    struct_atom->type = type;
    struct_atom->scope = scope;


    return atom_new(atom_type_struct, struct_atom);
}

bool atom_struct_is(Atom *atom) {
    if (!atom_is_of_type(atom, atom_type_struct))
        return false;

    StructAtom *struct_atom = atom->atom;
    return atom_is(struct_atom->type) && atom_scope_is(struct_atom->scope);
}

Atom *atom_string_new(char *str) {
    start_GlobalAtomTable_antialiasing
        if (atom_string_is(atom)) {
            StringAtom *string_atom = atom->atom;
            if (strcmp(string_atom->str, str) == 0)
                return mm_free("atom_string_new", str), atom;
        }
    end_GlobalAtomTable_antialiasing


    StringAtom *string_atom = mm_malloc("atom_string_new", sizeof *string_atom);
    string_atom->str = str;

    return atom_new(atom_type_string, string_atom);
}

bool atom_string_is(Atom *atom) {
    return atom_is_of_type(atom, atom_type_string);
}

Atom *atom_string_concat(Atom *atomA, Atom *atomB) {
    if (!atom_string_is(atomA) || !atom_string_is(atomB))
        return error_atom("atom_string_concat: Expected two string atoms.\n"), NULL;

    StringAtom *string_atomA = atomA->atom, *string_atomB = atomB->atom;
    const char *strA = string_atomA->str, *strB = string_atomB->str;

    char *str = mm_malloc("atom_string_concat", (strlen(strA)+strlen(strB)+1) * sizeof *str);
    sprintf(str, "%s%s", strA, strB);

    return atom_string_new(str);
}

Atom *atom_string_concat3(Atom *atomA, Atom *atomB, Atom *atomC) { return atom_string_concat(atomA, atom_string_concat(atomB, atomC)); }
Atom *atom_string_concat5(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE) { return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atomE)))); }
Atom *atom_string_concat6(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE, Atom *atomF) { return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atom_string_concat(atomE, atomF))))); }
Atom *atom_string_concat7(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE, Atom *atomF, Atom *atomG) { return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atom_string_concat(atomE, atom_string_concat(atomF, atomG)))))); }
Atom *atom_string_concat9(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE, Atom *atomF, Atom *atomG, Atom *atomH, Atom *atomI) { return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atom_string_concat(atomE, atom_string_concat(atomF, atom_string_concat(atomG, atom_string_concat(atomH, atomI)))))))); }
Atom *atom_string_concat10(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE, Atom *atomF, Atom *atomG, Atom *atomH, Atom *atomI, Atom *atomJ) { return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atom_string_concat(atomE, atom_string_concat(atomF, atom_string_concat(atomG, atom_string_concat(atomH, atom_string_concat(atomI, atomJ))))))))); }
Atom *atom_string_concat11(Atom *atomA, Atom *atomB, Atom *atomC, Atom *atomD, Atom *atomE, Atom *atomF, Atom *atomG, Atom *atomH, Atom *atomI, Atom *atomJ, Atom *atomK) { return atom_string_concat(atomA, atom_string_concat(atomB, atom_string_concat(atomC, atom_string_concat(atomD, atom_string_concat(atomE, atom_string_concat(atomF, atom_string_concat(atomG, atom_string_concat(atomH, atom_string_concat(atomI, atom_string_concat(atomJ, atomK)))))))))); }

Atom *atom_string_fromlong(long n) {
    char *str = mm_malloc("atom_string_fromlong", (snprintf(NULL, 0, "%ld", n)+1) * sizeof *str);
    sprintf(str, "%ld", n);

    return atom_string_new(str);
}

Atom *atom_string_fromchar(char c) {
    char *str = mm_malloc("atom_string_fromchar", sizeof *str + 1);
    str[0] = c, str[1] = '\0';

    return atom_string_new(str);
}

// 'fl' meaning 'from literal'
Atom *atom_string_newfl(const char *str) {
    char *new_str = mm_malloc("atom_string_newfl", (strlen(str)+1) * sizeof *str);
    sprintf(new_str, "%s", str);

    return atom_string_new(new_str);
}

Atom *atom_string_newcopy(const char *str) {
    return atom_string_newfl(str);
}

#undef start_GlobalAtomTable_antialiasing
#undef end_GlobalAtomTable_antialiasing

Atom *atom_string_read_from_file(const char *file_name) {
    FILE *f = fopen(file_name, "rb");
    if (!f)
        return error_atom("atom_string_read_from_file: Could not open file `%s`.\n", file_name), NULL;

    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = mm_malloc("atom_string_read_from_file", sizeof *content * (length+1));
    bool success = fread(content, sizeof *content, length, f) == length;
    content[length] = '\0';

    if (!success) {
        mm_free("atom_string_read_from_file", content);
        return error_atom("atom_string_read_from_file: Reading failed."), NULL;
    }

    return atom_string_new(content);
}

const char *string_from_atom(Atom *atom) {
    if (atom_string_is(atom))
        return ((StringAtom *) atom->atom)->str;

    if (atom_name_is(atom))
        return ((NameAtom *) atom->atom)->name;

    return error_atom("string_from_atom: Given an invalid atom `%s`.\n", atom_repr(atom)), NULL;
}

Atom *atom_list_new(AtomList *list) {
    ListAtom *list_atom = mm_malloc("atom_list_new", sizeof *list_atom);
    list_atom->list = list;

    return atom_new(atom_type_list, list_atom);
}

bool atom_list_is(Atom *atom) {
    if (!atom_is_of_type(atom, atom_type_list))
        return false;

    ListAtom *list_atom = atom->atom;
    if (!atomlist_is(list_atom->list))
        return error_atom("atom_list_is: Invalid list atom.\n"), false;

    return true;
}

AtomList *atom_list_get(Atom *atom) {
    if (!atom_list_is(atom))
        return error_atom("atom_list_get: Expected list atom, got `%s`.\n", atom_repr(atom)), NULL;

    ListAtom *list_atom = atom->atom;
    return list_atom->list;
}

AtomList *new_boxed_atomlist() {
    return atom_list_get(atom_list_new(atomlist_new(NULL)));
}
