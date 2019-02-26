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
#include <string.h>

#include "util.h"
#include "test.h"
#include "memorymanagement.h"


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

#define MAIN_ERR(...) return fprintf(stderr, __VA_ARGS__), memorymanagement_free_all(), EXIT_FAILURE
#define PRINT_HELP RETURN printf(\
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

    // arguments
    bool test_mode = false;
    AtomList *al_sources = atomlist_new(NULL);
    {
        if (argc < 1)
            MAIN_ERR("Please specify a krrp source file. Use `-h` for help.\n");

        bool interpret_arguments = true;

        for (int j = 1; j < argc; j++) {
            const char *arg = argv[j], *_arg = arg;

            if (interpret_arguments && arg[0] == '-') {
                if (arg[1] == '\0')
                    MAIN_ERR("Unknown argument `%s`.\n", arg);

                if (arg[1] == '-') {
                    if (arg[2] == '\0')
                        interpret_arguments = false;
                    else if (strcmp(arg, "--help") == 0)
                        PRINT_HELP;
                    else if (strcmp(arg, "--test") == 0)
                        test_mode = true;
                    else
                        MAIN_ERR("Unknown argument `%s`.\n", arg);
                }
                else {
                    for (char cflg; (cflg = *++_arg); )
                        if (cflg == 'h')
                            PRINT_HELP;
                        else if (cflg == 't')
                            test_mode = true;
                        else
                            MAIN_ERR("Unknown character flag `-%c` in argument `%s`.\n", cflg, arg);
                }
            }
            else {
                atomlist_push(al_sources, atom_string_newfl(arg));
                //if (filename == NULL)
                //    filename = arg;
                //else
                //    MAIN_ERR("Cannot interpret two source files (`%s` and `%s`).\n", filename, arg);
            }
        }
    }

    // testing
    if (test_mode) {
        printf("Testing ...\n");
        test_all();
    }

    // interpreting
    else {
        while (!atomlist_empty(al_sources)) {
            Atom *a_source = atomlist_pop_front(al_sources);
            if (!atom_string_is(a_source))
                MAIN_ERR("Invalid state.");
            const char *filename = ((StringAtom *) a_source->atom)->str;

            printf("* Reading file `%s` ...\n", filename);
            FILE *f = fopen(filename, "rb");
            if (!f)
                MAIN_ERR("Could not open krrp source file ??`%s`.\n", filename);
            fseek(f, 0, SEEK_END);
            int source_length = ftell(f);
            fseek(f, 0, SEEK_SET);
            // TODO :: On mm_malloc error, the file is not closed.
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

            atomlist_free(parsed); // TODO :: This list also never gets freed upon an error.


            mm_free("main: source", source);
        }
    }

    atomlist_free(al_sources); // TODO :: This list also never gets freed upon an error.

    // memory destruction
    RETURN EXIT_SUCCESS;
}
