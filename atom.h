#ifndef ATOM_H
#define ATOM_H

#include <stdbool.h>

// used to declare empty structs
#define EMPTY char _;

// atom-encapsulated types
typedef long integer;

// atom types
typedef enum {
    atom_type_null,
    atom_type_nullcondition,
    atom_type_nullscope,
    atom_type_Enull,

    atom_type_name,
    atom_type_integer,
    atom_type_primitive,

    atom_type_functiondeclaration,
    atom_type_function,
    atom_type_scope,

    atom_type_structinitializer,
    atom_type_struct,

    atom_type_string
} atom_type;

// atom structs
typedef struct Atom Atom;
typedef struct NullAtom NullAtom;
typedef struct NullConditionAtom NullConditionAtom;
typedef struct NullScopeAtom NullScopeAtom;
typedef struct ENullAtom ENullAtom;
typedef struct NameAtom NameAtom;
typedef struct IntegerAtom IntegerAtom;
typedef struct PrimitiveAtom PrimitiveAtom;
typedef struct FunctionDeclarationAtom FunctionDeclarationAtom;
typedef struct FunctionAtom FunctionAtom;
typedef struct ScopeAtom ScopeAtom;
typedef struct StructInitializerAtom StructInitializerAtom;
typedef struct StructAtom StructAtom;
typedef struct StringAtom StringAtom;

typedef struct AtomList AtomList;
typedef struct AtomListNode AtomListNode;

// global atom table
void globalatomtable_init();
void globalatomtable_print();
void globalatomtable_free();

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

// atom list creation and modification
struct AtomList { AtomListNode *head; };
struct AtomListNode { Atom *atom; AtomListNode *next; };

AtomList *atomlist_new(AtomListNode *head);
bool atomlist_is(AtomList *lst);
void atomlist_free(AtomList *lst);

//TODO
Atom *atomlist_representation(AtomList *lst);
const char *atomlist_str(AtomList *lst);

void atomlist_push(AtomList *lst, Atom *atom);
void atomlist_push_front(AtomList *lst, Atom *atom);
Atom *atomlist_pop_front(AtomList *lst);
int atomlist_len(AtomList *lst);
bool atomlist_empty(AtomList *lst);
AtomList *atomlist_copy(AtomList *lst);
bool atomlist_equal(AtomList *lstA, AtomList *lstB);
bool atomlist_purely(AtomList *lst, atom_type type);
void atomlist_remove_by_pointer(AtomList *lst, Atom *atom);

#endif
