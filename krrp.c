// krrp by Jonathan Frech (2018 to 2019)

/*
    Proto-3: 14th, 15th, 16th, 21st, 28th of February 2018
    Proto-2: 8th, 10th, 11th, 12th, 13th of March 2018
    Proto-1: 16th, 17th, 18th, 19th, 20th of March, 2nd of April 2018
    Current: 9th, 11th, 12th, 13th, 15th, 16th, 17th, 19th, 21st, 22nd, 23rd, 24th, 25th of April, 2nd of May 2018,
             16th, 17th, 18th, 20th, 21st of November, 4th, 9th, 11th of December 2018, 23rd of February 2019
 */

/*
    ![factorial]^n:?n*n@-n11.
    ![choose]^nk:!f;[factorial]/fn*fkf-nk.
*/

#include <stdio.h>
#include "atom.h"
#include "parse.h"
#include "interpret.h"
#include "options.h"
#include "error.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "util.h"
#include "test.h"
#include "memorymanagement.h"

#include "argparse.h"

// TODO: Potentially make an AtomList into its own full atom.
// TODO: Potentially implement AtomList as a doubly linked list.
// TODO: Potentially implement own special color markup.


// TODO
static const char *atomlist_str(AtomList *lst) {
    StringAtom *string_atom = atomlist_representation(lst)->atom;
    return string_atom->str;
}

// memory-management-aware return
#define RETURN return memorymanagement_free_all(),

#define MAIN_ERR(...) return error(__VA_ARGS__), memorymanagement_free_all(), EXIT_FAILURE
#define PRINT_HELP RETURN error(\
    "\nABOUT\n"\
    "   krrp by Jonathan Frech.\n"\
    "\n"\
    "USAGE\n"\
    "    -h, --help: Print this message.\n"\
    "    -t, --test: Perform a self-test.\n"\
    "\n"), EXIT_SUCCESS

int main(int argc, char **argv) {

    // memory initialization
    globaloptions_init();
    globalatomtable_init();

    // TODO
    PArgs *pargs = mm_malloc("pargs", sizeof *pargs);
    if (pargs == NULL)
        MAIN_ERR("Could not allocate memory.");
    pargs = parse_args(pargs, argc, argv);
    bool print_help = pargs->print_help;
    bool do_test = pargs->do_test;
    AtomList *al_sources = pargs->sources;
    mm_free("pargs", pargs);

    if (print_help)
        PRINT_HELP;

    // testing
    if (do_test)
        info("Testing ...\n"), test_all();

    // interpreting
    else {
        if (atomlist_empty(al_sources))
            MAIN_ERR("Please specify a krrp source file.\n");

        while (!atomlist_empty(al_sources)) {
            const char *file_name = atom_from_string(atomlist_pop_front(al_sources));

            info("* Reading file `%s` ...\n", file_name);

            Atom *source_a = atom_string_read_from_file(file_name);
            if (!atom_is_of_type(source_a, atom_type_string))
                MAIN_ERR("Could not open krrp source file `%s`.\n", file_name);
            StringAtom *source_a_atom = source_a->atom;
            const char *source = source_a_atom->str;


            print_escaped_source(source);

            info("=== Parsing ===\n");
            AtomList *parsed = parse(source);

            if (parsed == NULL)
                MAIN_ERR("Could not parse source.\n");

            info(".> %s\n", atomlist_str(parsed));
            info("=== Interpreting ===\n");
            Atom *scope = main_scope();
            while (!atomlist_empty(parsed))
                printf("%s\n", atom_repr(interpret(0, parsed, scope, true)));
        }
    }

    // memory destruction
    RETURN EXIT_SUCCESS;
}
