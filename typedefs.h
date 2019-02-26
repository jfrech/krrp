#ifndef TYPEDEFS_H
#define TYPEDEFS_H


// atom-encapsulated types
typedef long integer;


// atoms
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

// atom list
typedef struct AtomList AtomList;
typedef struct AtomListNode AtomListNode;


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

#endif
