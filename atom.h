#ifndef ATOM_H
#define ATOM_H

#include "atomlist.h"
#include <stdbool.h>

// used to declare empty structs
#define EMPTY char _;

// global atom table
void globalatomtable_init();
void globalatomtable_print();

// atom creation and modification
struct Atom { atom_type type; void *atom; };
Atom *atom_new(atom_type type, void *atom);
bool atom_is(Atom *atom);
bool atom_is_of_type(Atom *atom, atom_type type);
bool atom_equal(Atom *atomA, Atom *atomB);
const char *atom_repr(Atom *atom);
Atom *atom_representation(Atom *atom);

struct NullAtom { EMPTY };
Atom *atom_null_new();
bool atom_null_is(Atom *atom);

struct NullConditionAtom { EMPTY };
Atom *atom_nullcondition_new();
bool atom_nullcondition_is(Atom *atom);

struct NullScopeAtom { EMPTY };
Atom *atom_nullscope_new();
bool atom_nullscope_is(Atom *atom);

struct ENullAtom { EMPTY };
Atom *atom_Enull_new();
bool atom_Enull_is(Atom *atom);

struct NameAtom { char *name; };
Atom *atom_name_new(char *name);
bool atom_name_is(Atom *atom);

struct IntegerAtom { integer value; };
Atom *atom_integer_new(integer value);
bool atom_integer_is(Atom *atom);

struct PrimitiveAtom { char c; };
Atom *atom_primitive_new(char c);
bool atom_primitive_is(Atom *atom);

struct FunctionDeclarationAtom { int arity; AtomList *parameters; AtomList *body; };
Atom *atom_functiondeclaration_new(int arity, AtomList *parameters, AtomList *body);
bool atom_functiondeclaration_is(Atom *atom);

struct FunctionAtom { int arity; AtomList *parameters; AtomList *body; Atom *scope; char *primitive; };
Atom *atom_function_new(int arity, AtomList *parameters, AtomList *body, Atom *scope, char *primitive);
bool atom_function_is(Atom *atom);

struct ScopeAtom { AtomList *names; AtomList *binds; Atom *upper_scope; bool is_main; bool is_selfref; bool is_frozen; };
Atom *atom_scope_new(AtomList *names, AtomList *binds, Atom *upper_scope);
Atom *atom_scope_freeze(Atom *atom);
Atom *atom_scope_new_empty();
bool atom_scope_is(Atom *atom);

void atom_scope_setflag_ismain(Atom *atom, bool flg);
bool atom_scope_getflag_ismain(Atom *atom);
void atom_scope_setflag_isselfref(Atom *atom, bool flg);
bool atom_scope_getflag_isselfref(Atom *atom);

bool atom_scope_push(Atom *scope, Atom *name, Atom *bind);
Atom *atom_scope_unbind(Atom *scope, Atom *name);
Atom *atom_scope_new_inherits(Atom *scope);
Atom *atom_scope_new_fake(Atom *scope);

struct StructInitializerAtom { Atom *type; AtomList *fields; };
Atom *atom_structinitializer_new(Atom *type, AtomList *fields);
bool atom_structinitializer_is(Atom *atom);

struct StructAtom { Atom *type; Atom *scope; };
Atom *atom_struct_new(Atom *type, Atom *scope);
bool atom_struct_is(Atom *atom);

struct StringAtom { char *str; };
Atom *atom_string_new(char *str);
bool atom_string_is(Atom *atom);
Atom *atom_string_concat(Atom *atomA, Atom *atomB);
Atom *atom_string_concat3();
Atom *atom_string_concat5();
Atom *atom_string_concat6();
Atom *atom_string_concat7();
Atom *atom_string_concat9();
Atom *atom_string_concat10();
Atom *atom_string_concat11();
Atom *atom_string_fromlong(long n);
Atom *atom_string_fromchar(char c);
Atom *atom_string_newfl(const char *str);
Atom *atom_string_newcopy(const char *str);

#endif
