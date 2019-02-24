// krrp by Jonathan Frech (2018 to 2019)

/*
    Proto-3: 14th, 15th, 16th, 21st, 28th of February 2018
    Proto-2: 8th, 10th, 11th, 12th, 13th of March 2018
    Proto-1: 16th, 17th, 18th, 19th, 20th of March, 2nd of April 2018
    Current: 9th, 11th, 12th, 13th, 15th, 16th, 17th, 19th, 21st, 22nd, 23rd, 24th, 25th of April, 2nd of May 2018,
             16th, 17th, 18th, 20th, 21st of November, 4th, 9th, 11th of December 2018, 23rd of February 2019
 */

#include <stdio.h>
#include "atom.h"
#include "parse.h"
#include "interpret.h"
#include "options.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "util.h"
#include "test.h"
#include "memorymanagement.h"


// TODO :: Infinite recursion.
// TODO: potentially implement AtomList as a doubly linked list.
// TODO: potentially implement own special color markup


// TODO
static const char *atomlist_str(AtomList *lst) {
    StringAtom *string_atom = atomlist_representation(lst)->atom;
    return string_atom->str;
}


int main(int argc, char **argv) {
    globaloptions_init();
    globalatomtable_init();
    test_all();

    if (argc != 2)
        return fprintf(stderr, "Please specify a krrp source file.\n"), EXIT_FAILURE;

    const char *filename = argv[1];
    FILE *f = fopen(filename, "rb");
    if (!f)
        return fprintf(stderr, "Could not open specified file (%s).\n", filename), EXIT_FAILURE;

    fseek(f, 0, SEEK_END);
    int source_length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *source = mm_malloc("main: source", sizeof *source * (source_length + 1));
    fread(source, sizeof *source, source_length, f);
    source[source_length] = '\0';
    fclose(f);


    print_escaped_source(source);

    printf("\n=== Parsing ===\n");
    AtomList *parsed = parse(source);
    printf(".> %s\n", atomlist_str(parsed));


    printf("\n=== Interpreting ===\n");
    Atom *scope = main_scope();
    while (!atomlist_empty(parsed))
        printf(".> %s\n", atom_repr(interpret(0, parsed, scope, true)));
    printf("\n");

    atomlist_free(parsed);


    mm_free("main: source", source);
    memorymanagement_free_all();

    printf("\n");
    mm_print_status();
}
