// krrp by Jonathan Frech (2018 to 2019)

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "atom.h"
#include "parse.h"
#include "interpret.h"
#include "debug.h"
#include "util.h"
#include "test.h"
#include "memorymanagement.h"
#include "argparse.h"

#include "Opt.h"
extern Opt GlobOpt;


// memory-management-awareness
#define RETURN return memorymanagement_free_all(),
#define MAIN_ERR(...) return error(__VA_ARGS__), memorymanagement_free_all(), EXIT_FAILURE


int main(int argc, char **argv) {

    // memory initialization
    globalatomtable_init();

    PArgs pargs = parse_args(argc, argv);
    if (!pargs.parsing_successful)
        MAIN_ERR("Unsuccessful argument parsing.\n");

    GlobOpt.string_view = pargs.string_view;

    if (pargs.err < 0) GlobOpt.ERR = false;
    if (pargs.wrn < 0) GlobOpt.WRN = false;
    if (pargs.inf < 0) GlobOpt.INF = false;
    if (pargs.err > 0) GlobOpt.ERR = true;
    if (pargs.wrn > 0) GlobOpt.WRN = true;
    if (pargs.inf > 0) GlobOpt.INF = true;

    if (pargs.print_help)
        RETURN fprintf(stdout,\
            "ABOUT\n"\
            "   krrp by Jonathan Frech, 2018 to 2019.\n"\
            "\n"\
            "USAGE\n"\
            "    krrp [options] [source files]\n"\
            "\n"\
            "OPTIONS\n"\
            "    -h, --help         : Print this message.\n"\
            "    -t, --test         : Perform a self-test.\n"\
            "\n"\
            "    --str              : Activate string view;\n"\
            "                         struct names appear invisible and\n"\
            "                         integers appear as characters.\n"\
            "\n"\
            "    -c, --code [source]: Intepret specified source.\n"\
            "\n"\
            "    --[no]error        : Toggle error messages (on by default).\n"\
            "    --[no]warning      : Toggle warning messages (on by default).\n"\
            "    --[no]info         : Toggle info messages (off by default).\n"\
        ), EXIT_SUCCESS;

    if (pargs.do_test) {
        info("Testing ...\n");
        test_all();

        RETURN EXIT_SUCCESS;
    }

    // read `sources` into `code`
    while (!atomlist_empty(pargs.sources)) {
        const char *filename = string_from_atom(atomlist_pop_front(pargs.sources));
        info("* Reading file `%s` ...\n", filename);
        Atom *source_a = atom_string_read_from_file(filename);
        if (!atom_is_of_type(source_a, atom_type_string))
            MAIN_ERR("Could not open krrp source file `%s`.\n", filename);

        atomlist_push(pargs.codes, source_a);
    }


    if (atomlist_empty(pargs.codes))
        MAIN_ERR("No source code given. When in doubt, use the '--help' option.\n");

    while (!atomlist_empty(pargs.codes)) {
        const char *source = string_from_atom(atomlist_pop_front(pargs.codes));

        info("=== Source ===\n");
        print_escaped_source(source);

        info("=== Parsing ===\n");
        AtomList *parsed = parse(source);
        if (parsed == NULL)
            MAIN_ERR("Could not parse source.\n");
        info("    %s\n", string_from_atom(atomlist_representation(parsed)));

        info("=== Interpreting ===\n");
        Atom *scope = atom_scope_new_double_empty();
        inject_main(scope);
        while (!atomlist_empty(parsed))
            printf("%s\n", atom_repr(interpret_with_scope(parsed, scope)));
    }


    RETURN EXIT_SUCCESS;
}
