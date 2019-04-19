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


// memory-management-aware return
#define RETURN return memorymanagement_free_all(),

#define MAIN_ERR(...) return error(__VA_ARGS__), memorymanagement_free_all(), EXIT_FAILURE
#define PRINT_HELP RETURN fprintf(stderr,\
    "ABOUT\n"\
    "   krrp by Jonathan Frech.\n"\
    "\n"\
    "USAGE\n"\
    "    -h, --help: Print this message.\n"\
    "    -t, --test: Perform a self-test.\n"\
    "\n"), EXIT_SUCCESS


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
        RETURN fprintf(stderr,
            "krrp by Jonathan Frech, 2018-2019.\n"
            "\n"
            "Usage:\n"
            "    krrp [options] [source files]\n"
            "Options:\n"
            "    -h, --help         : Display this help message.\n"
            "    -t, --test         : Perform a self-test. (Sets `--info`.)\n"
            "    -c, --code [source]: Execute given source.\n"
            "\n"
            "    --[no]error        : Toggle error messages.\n"
            "    --[no]warning      : Toggle warning messages.\n"
            "    --[no]info         : Toggle info messages.\n"
        ), EXIT_SUCCESS;

    if (pargs.do_test) {
        info("Testing ...\n");
        test_all();

        RETURN EXIT_SUCCESS;
    }

    // read `sources` into `code`
    while (!atomlist_empty(pargs.sources)) {
        const char *file_name = string_from_atom(atomlist_pop_front(pargs.sources));
        info("* Reading file `%s` ...\n", file_name);
        Atom *source_a = atom_string_read_from_file(file_name);
        if (!atom_is_of_type(source_a, atom_type_string))
            MAIN_ERR("Could not open krrp source file `%s`.\n", file_name);

        atomlist_push(pargs.codes, source_a);
    }


    if (atomlist_empty(pargs.codes))
        MAIN_ERR("Please specify a krrp source file or code piece.\n");

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
        Atom *scope = TODO_main_scope();
        while (!atomlist_empty(parsed))
            printf("%s\n", atom_repr(interpret_with_scope(parsed, scope)));
    }


    RETURN EXIT_SUCCESS;
}
