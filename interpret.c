#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "atom.h"
#include "error.h"
#include "interpret.h"
#include "util.h"
#include "memorymanagement.h"

#define E(...) return error_interpret(__VA_ARGS__), atom_Enull_new();

Atom *interpret(AtomList *atoms, Atom *scope, bool active) {
    if (!atomlist_is(atoms))
        E("interpret: Given invalid atoms AtomList.\n")

    if (!atom_scope_is(scope))
        E("interpret: Given invalid scope ScopeAtom.\n")

    int execution_depth = 0;

    while (atoms->head) {
        Atom *atom = atomlist_pop_front(atoms);

        // unbind name
        if (atom_name_is(atom)) {
            Atom *unbound = atom_scope_unbind(scope, atom);

            if (!atom_is(unbound))
                E("interpret: Could not find name %s in scope.\n", atom_repr(atom))

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
        else if (execution_depth < 0)
            E("interpret: Negative execution depth (%d).\n", execution_depth)

        /* execution_depth >= 0 guaranteed */



        // call function
        if (atom_function_is(atom)) {
            // higher-order capability; return function
            if (execution_depth == 0)
                return active ? atom : atom_nullcondition_new();

            // apply function
            execution_depth--;
            FunctionAtom *function_atom = atom->atom;
            char *p = function_atom->primitive;

            // primitive function
            if (p && (strcmp(p, "!") == 0 || strcmp(p, "?") == 0 || strcmp(p, "|") == 0 || strcmp(p, "&") == 0)) {
                if (strcmp(p, "!") == 0) {
                    Atom *name = atomlist_pop_front(atoms);
                    if (!atom_name_is(name))
                        E("interpret: Bind needs NameAtom, got %s.\n", atom_repr(name))

                    Atom *bind = interpret(atoms, scope, active);
                    if (!atom_is(bind))
                        E("interpret: Bind needs Atom, got %s.\n", atom_repr(bind))

                    // TODO :: possibly `active &&` in this condition
                    if (!atom_scope_push(scope, name, bind))
                        E("interpret: Could not bind %s.\n", atom_repr(name))
                }

                else if (strcmp(p, "?") == 0) {
                    if (!active) {
                        for (int j = 0; j < 3; j++)
                            if (!atom_nullcondition_is(interpret(atoms, scope, false)))
                                E("interpret: Conditional primitive expected three arguments, got too few.\n")

                        return atom_nullcondition_new();
                    }

                    Atom *condition = interpret(atoms, scope, true);
                    if (!atom_integer_is(condition))
                        E("interpret: Conditional primitive expected integer as condition, got %s.\n", atom_repr(condition))

                    Atom *return_;
                    if (((IntegerAtom *) condition->atom)->value) {
                        return_ = interpret(atoms, scope, true);
                        interpret(atoms, scope, false);
                    }
                    else {
                        interpret(atoms, scope, false);
                        return_ = interpret(atoms, scope, true);
                    }

                    return active ? return_ : atom_nullcondition_new();
                }

                else if (strcmp(p, "|") == 0 || strcmp(p, "&") == 0)
                    E("TODO |&\n")
                else
                    E("?!?!?\n")

                continue; // TODO :: lexical extent ... breaks `if` chain
            }


            // PROBLEM TODO :: names shall not be evaluated (`!`, `#!`, ...)
            // lexical extent of inactive evaluation known, swallow arguments and return (as the function's return is not called (higher-order, could else be function)
            if (execution_depth == 0 && !active) {
                for (int j = 0; j < function_atom->arity; j++)
                    interpret(atoms, atom_scope_new_fake(scope), false);

                return atom_nullcondition_new();
            }

            /* active == true guaranteed */

            // TODO
            if (p) {
                if (execution_depth != 0)
                    E("interpret: First-order primitive called with remaining execution depth (%d).\n", execution_depth)

                if (strcmp(p, "=") == 0) {
                    Atom *atomA = interpret(atoms, scope, true);
                    if (!atom_is(atomA))
                        E("interpret: = expected atom as first argument (%s).\n", atom_repr(atomA))

                    Atom *atomB = interpret(atoms, scope, true);
                    if (!atom_is(atomB))
                        E("interpret: = expected atom as second argument (%s).\n", atom_repr(atomA))

                    return atom_integer_new(atom_equal(atomA, atomB));
                }


                // TODO
                else {
                    Atom *atomA = interpret(atoms, scope, true);
                    if (!atom_integer_is(atomA))
                        E("interpret: Integer primitive's first argument is not an integer (%s).\n", atom_repr(atomA))

                    Atom *atomB = interpret(atoms, scope, true);
                    if (!atom_integer_is(atomB))
                        E("interpret: Integer primitive's second argument is not an integer (%s).\n", atom_repr(atomB))

                    integer A = ((IntegerAtom *) atomA->atom)->value;
                    integer B = ((IntegerAtom *) atomB->atom)->value;

                    if (B == 0 && (strcmp(p, "%") == 0 || strcmp(p, "/") == 0))
                        return error_interpret("interpret: Division by zero.\n"), atom_null_new();

                    if      (strcmp(p, "%") == 0) return atom_integer_new(A % B);
                    else if (strcmp(p, "*") == 0) return atom_integer_new(A * B);
                    else if (strcmp(p, "+") == 0) return atom_integer_new(A + B);
                    else if (strcmp(p, "-") == 0) return atom_integer_new(A - B);
                    else if (strcmp(p, "/") == 0) return atom_integer_new(A / B);
                    else if (strcmp(p, "<") == 0) return atom_integer_new(A < B);
                    else if (strcmp(p, "=") == 0) return atom_integer_new(A ==B);
                    else if (strcmp(p, ">") == 0) return atom_integer_new(A > B);

                    else
                        E("interpret: Unknown primitive (\"%s\").\n", atom_repr(atom))
                }
            }

            // non-primitive function
            else {
                // bind parameters
                AtomListNode *node = function_atom->parameters->head;
                Atom *scp = atom_scope_new_inherits(function_atom->scope);
                while (node) {
                    Atom *a = interpret(atoms, scope, true);
                    if (!atom_is(a))
                        E("interpret: Found no atom to bind function parameter %s to.\n", atom_repr(node->atom))

                    if (!atom_scope_push(scp, node->atom, a))
                        E("interpret: Attempt at rebind.\n")

                    node = node->next;
                }

                AtomList *atoms_copy = atomlist_copy(function_atom->body);
                Atom *ret = interpret(atoms_copy, scp, true);
                atomlist_free(atoms_copy);

                if (execution_depth > 0)
                    atomlist_push_front(atoms, ret);
                else
                    return active ? ret : atom_nullcondition_new();
            }
        }

        // TODO :: implement as ordinary function
        else if (atom_structinitializer_is(atom)) {
            if (execution_depth == 0)
                return active ? atom : atom_nullcondition_new();

            execution_depth--;
            StructInitializerAtom *structinitializer_atom = atom->atom;
            if (execution_depth != 0)
                return error_interpret("???"), atom_null_new();

            if (execution_depth == 0 && !active) {
                for (int j = 0; j < atomlist_len(structinitializer_atom->fields); j++)
                    interpret(atoms, atom_scope_new_fake(scope), false);

                return atom_nullcondition_new();
            }

            Atom *type = structinitializer_atom->type;
            AtomList *fields = structinitializer_atom->fields;
            Atom *struct_scope = atom_scope_new_empty();
            AtomListNode *node = fields->head;

            while (node) {
                atom_scope_push(struct_scope, node->atom, interpret(atoms, scope, true));
                node = node->next;
            }

            struct_scope = atom_scope_freeze(struct_scope);
            return atom_struct_new(type, struct_scope);
        }

        // set up lexical scoping and turn function declaration into function
        else if (atom_functiondeclaration_is(atom)) {
            // TODO :: assure functionality
            FunctionDeclarationAtom *fd = atom->atom;
            Atom *scp = atom_scope_new_inherits(scope);
            Atom *f_scp = atom_scope_new_inherits(scp);
            Atom *f = atom_function_new(
                fd->arity,
                atomlist_copy(fd->parameters),
                atomlist_copy(fd->body),
                f_scp,
                NULL
            );
            atom_scope_push(scp, atom_name_new(strdup("@")), f);
            atom_scope_setflag_isselfref(scp, true);

            if (execution_depth > 0)
                atomlist_push_front(atoms, f);
            else if (execution_depth == 0)
                return active ? f : atom_nullcondition_new();
            else
                E("interpret: Execution depth negative when encountering function declaration.\n")
        }

        /*** Guarantee: execution_depth == 0 ***/
        else if (execution_depth != 0)
            E("interpret: Execution depth non-zero when encountering neither function nor higher-order primitive (encountered %s).\n", atom_repr(atom))




        else if (atom_primitive_is(atom)) {
            char c = ((PrimitiveAtom *) atom->atom)->c;

            // execution depth should have been interpreted above
            if (c == ',' || c == ';')
                E("interpret: Cannot interpret execution depth modifiers this late.\n")

            // TODO :: extract
            // struct type check
            else if (c == '#' + '?') {
                if (!active) {
                    atomlist_pop_front(atoms);
                    interpret(atoms, scope, false);
                    return atom_nullcondition_new();
                }

                Atom *type = atomlist_pop_front(atoms);
                if (!atom_name_is(type))
                    E("interpret: Struct type check requires NameAtom, got %s.\n", atom_repr(type))

                Atom *struct_ = interpret(atoms, scope, active);
                if (!atom_struct_is(struct_))
                    E("interpret: Struct type check requires StructAtom, got %s.\n", atom_repr(struct_))

                StructAtom *struct_atom = struct_->atom;

                return atom_integer_new(atom_equal(struct_atom->type, type));
            }

            // TODO :: extract
            // struct field extraction
            else if (c == '#' + '!') {
                if (!active) {
                    atomlist_pop_front(atoms);
                    interpret(atoms, atom_scope_new_fake(scope), false);
                    return atom_nullcondition_new();
                }

                Atom *field_name = atomlist_pop_front(atoms);
                if (!atom_name_is(field_name))
                    E("interpret: Struct field extraction requires NameAtom, got %s.\n", atom_repr(field_name))

                Atom *struct_ = interpret(atoms, scope, active);
                if (!atom_struct_is(struct_))
                    E("interpret: Struct field extraction requires StructAtom, got %s.\n", atom_repr(struct_))

                StructAtom *struct_atom = struct_->atom;
                Atom *extracted = atom_scope_unbind(struct_atom->scope, field_name);
                if (!extracted)
                    E("interpret: Could not extract field name %s from struct %s.\n", atom_repr(field_name), atom_repr(struct_))

                return extracted;
            }

            else
                E("interpret: Unknown primitive %s.\n", atom_repr(atom))
        }

        else if (atom_integer_is(atom) || atom_structinitializer_is(atom) || atom_struct_is(atom))
            return active ? atom : atom_nullcondition_new();

        else
            E("interpret: ???\n")
    }

    if (!active)
        E("Fell inactively through.\n")

    E("Fell through.\n")
}

// generate the main scope (every scope is descendant of this scope or the null scope itself)
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
    #define BAP3(str) B(strdup(str), atom_function_new(3, NULL, NULL, atom_nullscope_new(), strdup(str)))

    BAP("%") BAP("*") BAP("+") BAP("-") BAP("/") BAP("<") BAP("=") BAP(">")
    BAP("!") BAP3("?") BAP("|") BAP("&")

    #undef BD
    #undef B

    #undef BAP
    #undef BAP3

    ((ScopeAtom *) scope->atom)->is_main = true;

    return scope;
}
