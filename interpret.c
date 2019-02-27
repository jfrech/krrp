#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "atom.h"
#include "error.h"
#include "interpret.h"
#include "util.h"
#include "memorymanagement.h"

extern Options *GlobalOptions;

#define ASSERT(cnd, ...) { if (!(cnd)) return error_interpret(__VA_ARGS__), atom_Enull_new(); }


Atom *interpret(long recursion_depth, AtomList *atoms, Atom *scope, bool active) {
    ASSERT(GlobalOptions, "interpret: Cannot access global options.\n")
    ASSERT(recursion_depth < GlobalOptions->maximum_interpretation_recursion_depth,
        "interpret: Maximum interpretation iterations reached.\n")
    ASSERT(atomlist_is(atoms), "interpret: Given invalid atoms AtomList.\n");
    ASSERT(atom_scope_is(scope), "interpret: Given invalid scope ScopeAtom.\n");

    int execution_depth = 0;
    while (atoms->head) {
        Atom *atom = atomlist_pop_front(atoms);

        ASSERT(atom_is(atom), "interpret: Encountered non-atom.\n")

        if (atom_name_is(atom)) {
            Atom *unbound = atom_scope_unbind(scope, atom);
            ASSERT(atom_is(unbound), "interpret: Could not find name %s in scope.\n", atom_repr(atom))

            if (atom_function_is(unbound) || atom_structinitializer_is(unbound)) {
                atomlist_push_front(atoms, unbound);
                execution_depth++;
                continue;
            }

            else
                return active ? unbound : atom_nullcondition_new();
        }


        if (atom_equal(atom, atom_primitive_new(','))) {
            execution_depth++;
            continue;
        }
        else if (atom_equal(atom, atom_primitive_new(';'))) {
            execution_depth--;
            continue;
        }


        ASSERT(execution_depth >= 0, "Execution depth in an invalid state (%d).", execution_depth)


        if (atom_function_is(atom)) {
            // higher-order capability; return function
            if (execution_depth == 0)
                return active ? atom : atom_nullcondition_new();

            // apply function
            execution_depth--;
            FunctionAtom *function_atom = atom->atom;
            char *p = function_atom->primitive;


            // lexical extent of inactive evaluation known, swallow arguments
            // and return (as the function's return is not called
            // (higher-order, could else be function)
            if (execution_depth == 0 && !active) {
                for (int j = 0; j < function_atom->arity; j++)
                    interpret(recursion_depth+1, atoms, atom_scope_new_fake(scope), false);

                return atom_nullcondition_new();
            }


            ASSERT(active, "interpret: Invalid inactiveness.\n")


            // boxed primitive function
            if (p) {
                ASSERT(execution_depth == 0, "interpret: First-order primitive called with remaining execution depth (%d).\n", execution_depth)

                // atom equality
                if (strcmp(p, "=") == 0) {
                    Atom *atomA = interpret(recursion_depth+1, atoms, scope, true);
                    Atom *atomB = interpret(recursion_depth+1, atoms, scope, true);

                    ASSERT(atom_is(atomA), "interpret: `=` expected atom as first argument (got `%s`).\n", atom_repr(atomA))
                    ASSERT(atom_is(atomB), "interpret: `=` expected atom as second argument (got `%s`).\n", atom_repr(atomA))

                    return atom_integer_new(atom_equal(atomA, atomB));
                }

                // integer operations
                else {
                    Atom *atomA = interpret(recursion_depth+1, atoms, scope, true);
                    Atom *atomB = interpret(recursion_depth+1, atoms, scope, true);

                    ASSERT(atom_integer_is(atomA), "interpret: Integer primitive's first argument is not an integer (got `%s`).\n", atom_repr(atomA))
                    ASSERT(atom_integer_is(atomB), "interpret: Integer primitive's second argument is not an integer (got `%s`).\n", atom_repr(atomB))

                    integer A = ((IntegerAtom *) atomA->atom)->value;
                    integer B = ((IntegerAtom *) atomB->atom)->value;

                    if (B == 0 && (strcmp(p, "%") == 0 || strcmp(p, "/") == 0))
                        return error_interpret("interpret: Division by zero.\n"), atom_integer_new(0);

                    if (p[0] != '\0' && p[1] == '\0')
                        switch (p[0]) {
                            #define MK(c, op) case c: return atom_integer_new(A op B);
                            MK('%', %) MK('*', *) MK('+', +) MK('-', -) MK('/', /)
                            MK('<', <) MK('=', ==) MK('>', >)
                            #undef MK

                            default: ASSERT(false, "interpret: Unknown primitive `%s`.\n", atom_repr(atom))
                        }

                    else
                        ASSERT(false, "interpret: Unknown primitive `%s`.\n", atom_repr(atom))
                }
            }

            // non-primitive function
            else {
                // bind parameters
                AtomListNode *node = function_atom->parameters->head;
                Atom *scp = atom_scope_new_inherits(function_atom->scope);
                while (node) {
                    Atom *a = interpret(recursion_depth+1, atoms, scope, true);

                    ASSERT(atom_is(a), "interpret: Found no atom to bind function parameter %s to.\n", atom_repr(node->atom))
                    ASSERT(atom_scope_push(scp, node->atom, a), "interpret: Attempt at rebind.\n")

                    node = node->next;
                }

                AtomList *atoms_copy = atomlist_copy(function_atom->body);
                Atom *ret = interpret(recursion_depth+1, atoms_copy, scp, true);
                atomlist_free(atoms_copy);

                if (execution_depth > 0)
                    atomlist_push_front(atoms, ret);
                else
                    return active ? ret : atom_nullcondition_new();
            }

            continue;
        }


        else if (atom_structinitializer_is(atom)) {
            if (execution_depth == 0)
                return active ? atom : atom_nullcondition_new();

            execution_depth--;
            StructInitializerAtom *structinitializer_atom = atom->atom;

            ASSERT(execution_depth == 0, "Unexpected execution depth in struct initializer (%d).\n", execution_depth)

            if (execution_depth == 0 && !active) {
                for (int j = 0; j < atomlist_len(structinitializer_atom->fields); j++)
                    interpret(recursion_depth+1, atoms, atom_scope_new_fake(scope), false);

                return atom_nullcondition_new();
            }

            Atom *type = structinitializer_atom->type;
            AtomList *fields = structinitializer_atom->fields;
            Atom *struct_scope = atom_scope_new_empty();
            AtomListNode *node = fields->head;

            while (node) {
                atom_scope_push(struct_scope, node->atom, interpret(recursion_depth+1, atoms, scope, true));
                node = node->next;
            }

            return atom_struct_new(type, atom_scope_freeze(struct_scope));
        }


        // set up lexical scoping and turn function declaration into function
        else if (atom_functiondeclaration_is(atom)) {
            FunctionDeclarationAtom *fd = atom->atom;
            Atom *scp = atom_scope_new_inherits(scope);
            Atom *f_scp = atom_scope_new_inherits(scp);
            Atom *f = atom_function_new(
                fd->arity,
                atomlist_copy(fd->parameters),
                atomlist_copy(fd->body),
                f_scp,
                NULL // not primitive
            );

            // self-referring name
            atom_scope_push(scp, atom_name_new(strdup("@")), f);
            atom_scope_setflag_isselfref(scp, true);

            ASSERT(execution_depth >= 0, "interpret: Execution depth negative when encountering function declaration.\n")

            if (execution_depth == 0)
                return active ? f : atom_nullcondition_new();

            atomlist_push_front(atoms, f);
            continue;
        }


        ASSERT(execution_depth == 0, "interpret: Execution depth non-zero when encountering neither function nor higher-order primitive (encountered %s).\n", atom_repr(atom))


        if (atom_primitive_is(atom)) {
            const char c = ((PrimitiveAtom *) atom->atom)->c;

            // execution depth should have been interpreted above
            ASSERT(c != ',' && c != ';', "interpret: Cannot interpret execution depth modifiers this late.\n")

            // extended primitive function
            if (true /* TODO */) {
                if (c == '!') {
                    Atom *name = atomlist_pop_front(atoms);
                    ASSERT(atom_name_is(name), "interpret: Bind needs NameAtom, got %s.\n", atom_repr(name))

                    Atom *bind = interpret(recursion_depth+1, atoms, scope, active);
                    ASSERT(atom_is(bind), "interpret: Bind needs Atom, got %s.\n", atom_repr(bind))

                    ASSERT(atom_scope_push(scope, name, bind), "interpret: Could not bind %s.\n", atom_repr(name))
                    continue;
                }

                else if (c == '?') {
                    if (!active) {
                        for (int j = 0; j < 3; j++)
                            ASSERT(atom_nullcondition_is(interpret(recursion_depth+1, atoms, scope, false)), "interpret: Conditional primitive expected three arguments, got too few.\n")

                        return atom_nullcondition_new();
                    }

                    Atom *condition = interpret(recursion_depth+1, atoms, scope, true);
                    ASSERT(atom_integer_is(condition), "interpret: Conditional primitive expected integer as condition, got %s.\n", atom_repr(condition))

                    Atom *return_;
                    if (((IntegerAtom *) condition->atom)->value) {
                        return_ = interpret(recursion_depth+1, atoms, scope, true);
                        interpret(recursion_depth+1, atoms, scope, false);
                    }
                    else {
                        interpret(recursion_depth+1, atoms, scope, false);
                        return_ = interpret(recursion_depth+1, atoms, scope, true);
                    }

                    return active ? return_ : atom_nullcondition_new();
                }

                else if (c == '|' ||  c == '&')
                    ASSERT(false, "TODO :: Functionality for `|` and `&` is not yet implemented.\n")

                // TODO :: extract
                // struct type check
                else if (c == '#' + '?') {
                    if (!active) {
                        atomlist_pop_front(atoms);
                        interpret(recursion_depth+1, atoms, scope, false);
                        return atom_nullcondition_new();
                    }

                    Atom *type = atomlist_pop_front(atoms);
                    ASSERT(atom_name_is(type), "interpret: Struct type check requires NameAtom, got %s.\n", atom_repr(type))

                    Atom *struct_ = interpret(recursion_depth+1, atoms, scope, active);
                    ASSERT(atom_struct_is(struct_), "interpret: Struct type check requires StructAtom, got %s.\n", atom_repr(struct_))

                    StructAtom *struct_atom = struct_->atom;

                    return atom_integer_new(atom_equal(struct_atom->type, type));
                }

                // TODO :: extract
                // struct field extraction
                else if (c == '#' + '!') {
                    if (!active) {
                        atomlist_pop_front(atoms);
                        interpret(recursion_depth+1, atoms, atom_scope_new_fake(scope), false);
                        return atom_nullcondition_new();
                    }

                    Atom *field_name = atomlist_pop_front(atoms);
                    ASSERT(atom_name_is(field_name), "interpret: Struct field extraction requires NameAtom, got %s.\n", atom_repr(field_name))

                    Atom *struct_ = interpret(recursion_depth+1, atoms, scope, active);
                    ASSERT(atom_struct_is(struct_), "interpret: Struct field extraction requires StructAtom, got %s.\n", atom_repr(struct_))

                    StructAtom *struct_atom = struct_->atom;
                    Atom *extracted = atom_scope_unbind(struct_atom->scope, field_name);
                    ASSERT(extracted, "interpret: Could not extract field name %s from struct %s.\n", atom_repr(field_name), atom_repr(struct_))

                    return extracted;
                }

                else
                    ASSERT(false, "interpret: Unknown primitive `%s`.\n", atom_repr(atom))
            }

            continue;
        }

        if (atom_integer_is(atom)) {
            ASSERT(execution_depth == 0, "Integer used with non-zero execution depth.\n")
            return atom;
        }

        if (atom_structinitializer_is(atom) || atom_struct_is(atom))
            return active ? atom : atom_nullcondition_new();

        ASSERT(false, "interpret: Invalid state.\n")
    }

    ASSERT(false, "Fell through (active = %s).\n", active ? "true" : "false")
}


// generate the main scope (every scope is a descendant of this scope or the null scope itself)
Atom *main_scope() {
    Atom *scope = atom_scope_new_empty();

    // bind macro
    #define B(str, bnd) atom_scope_push(scope, atom_name_new(str), bnd);

    // bind digit macro
    #define BD(str, dgt) B(strdup(str), atom_integer_new(dgt))
    BD("0", 0) BD("1", 1) BD("2", 2) BD("3", 3) BD("4", 4)
    BD("5", 5) BD("6", 6) BD("7", 7) BD("8", 8) BD("9", 9)

    // bind arithmetic primitive macro
    #define BAP(str) B(strdup(str), atom_function_new(2, NULL, NULL, atom_nullscope_new(), strdup(str)))
    // #define BAP3(str) B(strdup(str), atom_function_new(3, NULL, NULL, atom_nullscope_new(), strdup(str)))

    BAP("%") BAP("*") BAP("+") BAP("-") BAP("/") BAP("<") BAP("=") BAP(">")
    // BAP("!") BAP3("?") BAP("|") BAP("&")

    #undef BD
    #undef B

    #undef BAP
    #undef BAP3

    ((ScopeAtom *) scope->atom)->is_main = true;

    return scope;
}
