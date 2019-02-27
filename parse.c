#include <stdlib.h>
#include <stdbool.h>

#include "parse.h"
#include "atom.h"
#include "error.h"
#include "memorymanagement.h"


static int _parse(const char *source, int p, AtomList *parsed, parse_state state);
static void error_parse(const char *source, int p, const char *err);

AtomList *parse(const char *source) {
    AtomList *parsed = atomlist_new(NULL);
    if (_parse(source, 0, parsed, parse_state_main))
        return atomlist_free(parsed), error_parse(source, -1, "parse: Parsing failed."), NULL;

    return parsed;
}


// TODO :: `fprintf`
static void print_escaped(const char *source, int p) {
    fprintf(stderr, "    ");

    int len = 0, width = 1;
    unsigned char c;
    for (int j = 0; (c = source[j]); j++) {
        if (c == '\t' || c == '\n') {
            fprintf(stderr, c == '\t' ? "\\t" : "\\n");
            if (j < p) len += 2;
            if (j == p) width = 2;
        }
        else if ((0 <= c && c <= 31) || c >= 127) {
            fprintf(stderr, "\\x%02x", c);
            if (j < p) len += 4;
            if (j == p) width = 4;
        }
        else {
            fprintf(stderr, "%c", c);
            if (j < p) len++;
            if (j == p) width = 1;
        }
    }

    if (p >= 0) {
        fprintf(stderr, "\n    ");
        for (int j = 0; j < len; j++)
            fprintf(stderr, " ");
        for (int j = 0; j < width; j++)
            fprintf(stderr, "^ byte %d", p);
    }
    else
        fprintf(stderr, "\n   ^ (byte unknown)");

    fprintf(stderr, "\n");
}

static void error_parse(const char *source, int p, const char *err) {
    print_escaped(source, p);
    error("ParseError :: %s\n", err);
}

static void warning_parse(const char *source, int p, const char *wrn) {
    print_escaped(source, p);
    warning("ParseWarning :: %s\n", wrn);
}

/* --- */

static int parse_comment(const char *source, int p) {
    if (source[p] != '~')
        return error_parse(source, p, "parse_comment: Attempting to parse non-comment."), -1;

    while (source[p] && source[p] != '\n')
        p++;

    if (source[p] == '\0')
        return p-1;

    return p;
}

static int parse_functiondeclaration(const char *source, int p, AtomList *parsed) {
    if (source[p] != '^')
        return error_parse(source, p, "parse_functiondeclaration: Attempting to parse non-function-declaration."), -1;

    // parse parameters
    int s = p;
    AtomList *parameters = atomlist_new(NULL);
    #define RETURN return atomlist_free(parameters),
    p = _parse(source, ++p, parameters, parse_state_functiondeclaration_parameters);



    if (p == -1)
        RETURN error_parse(source, s, "parse_functiondeclaration: Could not parse function declaration parameters."), -1;

    if (source[p] != ':')
        RETURN error_parse(source, p, "parse_functiondeclaration: Function declaration needs body."), -1;

    // only names are permitted as parameters
    AtomListNode *node = parameters->head;
    while (node) {
        if (!atom_name_is(node->atom))
            RETURN error_parse(source, s, "parse_functiondeclaration: Found non-name in function declaration parameters."), -1;

        node = node->next;
    }

    if (!parameters->head)
        warning_parse(source, s, "parse_functiondeclaration: Function declaration without parameters.");

    // parse body
    AtomList *body = atomlist_new(NULL);
    #undef RETURN
    #define RETURN return atomlist_free(parameters), atomlist_free(body),
    p = _parse(source, ++p, body, parse_state_functiondeclaration_body);

    if (p == -1)
        RETURN error_parse(source, s, "parse_functiondeclaration: Could not parse function declaration body."), -1;

    if (source[p] != '.')
        RETURN error_parse(source, p, "parse_functiondeclaration: Function declaration body unfinished."), -1;

    if (!body->head)
        warning_parse(source, s, "parse_functiondeclaration: Function declaration without body.");

    // assemble function declaration
    atomlist_push(parsed, atom_functiondeclaration_new(atomlist_len(parameters), parameters, body));

    #undef RETURN
    return p;
}

static int parse_structinitializer(const char *source, int p, AtomList *parsed) {
    if (source[p] != '#')
        return error_parse(source, p, "parse_structinitializer: Attempting to parse non-struct-declaration."), -1;

    if (source[p+1] == '!' || source[p+1] == '?')
        return atomlist_push(parsed, atom_primitive_new('#' + source[p+1])), p+1;

    AtomList *fields = atomlist_new(NULL);
    #define RETURN return atomlist_free(fields), // TODO
    p = _parse(source, ++p, fields, parse_state_struct_fields);

    if (p == -1)
        return error_parse(source, p, "parse_structinitializer: Could not parse struct declaration fields."), -1;

    if (source[p] != '.')
        return error_parse(source, p, "parse_structinitializer: Struct fields unfinished."), -1;

    AtomListNode *node = fields->head;
    while (node) {
        if (!atom_name_is(node->atom))
            return error_parse(source, p, "parse_structinitializer: Found non-name in struct declaration fields."), -1;

        node = node->next;
    }

    if (atomlist_len(fields) < 1)
        return error_parse(source, p, "parse_structinitializer: Found no struct type name."), -1;

    // TODO error on duplicate fields

    Atom *type = atomlist_pop_front(fields);

    atomlist_push(parsed, atom_structinitializer_new(type, fields));

    return p;
}

// TODO refactor
static int parse_literal(const char *source, int p, AtomList *parsed) {
    if (source[p] != '$')
        return error_parse(source, p, "parse_literal: Attempting to parse non-literal."), -1;

    int s = p++;
    while (source[p] && source[p] != '.')
        p++;

    if (source[p] != '.')
        return error_parse(source, s, "parse_literal: Unfinished literal."), -1;

    char *literalstr = mm_malloc("parse_literal", (p-s) * sizeof *literalstr);
    for (int j = 0; j < p-s-1; j++)
        literalstr[j] = source[s+1+j];
    literalstr[p-s-1] = '\0';

    atomlist_push(parsed, atom_integer_new(atol(literalstr))); // TODO: check if string is valid literal
    mm_free("parse_literal", literalstr);

    return p;
}

static int parse_name(const char *source, int p, AtomList *parsed) {
    if (source[p] != '[') {
        char *name = mm_malloc("parse_name", sizeof *name + 1);
        name[0] = source[p];
        name[1] = '\0';
        atomlist_push(parsed, atom_name_new(name));

        return p;
    }

    int s = ++p;
    while (source[p] && source[p] != ']')
        p++;

    if (source[p] != ']')
        return error_parse(source, p, "parse_name: Unfinished name."), -1;

    char *name = mm_malloc("parse_name", (p-s+1) * sizeof *name);
    for (int j = 0; j < p-s; j++)
        name[j] = source[s+j];
    name[p-s] = '\0';
    atomlist_push(parsed, atom_name_new(name));

    return p;
}

// internal parse function (recursively called in each function declaration)
static int _parse(const char *source, int p, AtomList *parsed, parse_state state) {
    for (char c; (c = source[p]); p++) {
        int _p = p;

        // <comment>
        if (c == '~')
            p = parse_comment(source, p);

        // <whitespace>
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            ;

        // <function_declaration>
        else if (c == '^')
            p = parse_functiondeclaration(source, p, parsed);

        else if (c == '#')
            p = parse_structinitializer(source, p, parsed);

        // <primitive>
        else if (c == ',' || c == ';' || c == '!' || c == '?' || c == '|' || c == '&')
            atomlist_push(parsed, atom_primitive_new(c));

        // <long_literal>
        else if (c == '$')
            p = parse_literal(source, p, parsed);

        // ':' is only permitted in function declaration
        else if (c == ':') {
            if (state != parse_state_functiondeclaration_parameters) {
                if (state == parse_state_functiondeclaration_body)
                    return error_parse(source, p, "parse: Stray ':' in function declaration body."), -1;
                return error_parse(source, p, "parse: Stray ':' (only permitted in function declaration)."), -1;
            }

            return p;
        }

        // '.' is only permitted as a delimiter
        else if (c == '.') {
            if (state != parse_state_functiondeclaration_body && state != parse_state_struct_fields) {
                if (state == parse_state_functiondeclaration_parameters)
                    return error_parse(source, p, "parse: Stray '.' in function declaration parameters or struct fields."), -1;
                return error_parse(source, p, "parse: Stray '.' (only permitted as delimiter)."), -1;
            }

            return p;
        }

        // <name>
        else
            p = parse_name(source, p, parsed);

        // propagate error
        if (p == -1)
            return error_parse(source, _p, "parse: Parsing failed."), -1;
    }

    if (state != parse_state_main)
        return error_parse(source, p, "parse: Parsing ended in non-main state."), -1;

    return 0;
}
