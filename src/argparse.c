#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "atomlist.h"
#include "atom.h"
#include "memorymanagement.h"
#include "debug.h"


#define ARG_HELP pargs.print_help = true;
#define ARG_TEST pargs.do_test = true, pargs.inf = 1;
#define ARG_CODE {\
        if (++j >= argc)\
            ERR("Code flag without content.\n");\
        atomlist_push(pargs.codes, atom_string_newfl(argv[j]));\
    }
#define ARG_STR pargs.string_view = true;

#define ERR(...) return error("ArgParse :: " __VA_ARGS__), pargs
PArgs parse_args(int argc, char **argv) {
    PArgs pargs = (PArgs) {
        .parsing_successful = false,

        .do_test = false,
        .print_help = false,
        .sources = new_boxed_atomlist(),
        .codes = new_boxed_atomlist(),

        .err = 0,
        .wrn = 0,
        .inf = 0,

        .string_view = false
    };

    bool interpret_arguments = true;


    for (int j = 1; j < argc; j++) {
        const char *arg = argv[j], *_arg = arg;

        if (interpret_arguments && arg[0] == '-') {
            if (arg[1] == '\0')
                ERR("Unknown argument `-`.\n");

            else if (arg[1] == '-') {
                if (arg[2] == '\0')
                    interpret_arguments = false;

                else if (strcmp(arg, "--help") == 0) { ARG_HELP }
                else if (strcmp(arg, "--test") == 0) { ARG_TEST }
                else if (strcmp(arg, "--code") == 0) { ARG_CODE }
                else if (strcmp(arg, "--str" ) == 0) { ARG_STR }

                else if (strcmp(arg, "--error"    ) == 0) pargs.err = 1;
                else if (strcmp(arg, "--warning"  ) == 0) pargs.wrn = 1;
                else if (strcmp(arg, "--info"     ) == 0) pargs.inf = 1;
                else if (strcmp(arg, "--noerror"  ) == 0) pargs.err = -1;
                else if (strcmp(arg, "--nowarning") == 0) pargs.wrn = -1;
                else if (strcmp(arg, "--noinfo"   ) == 0) pargs.inf = -1;

                else
                    ERR("Unknown argument `%s`.\n", arg);
            }
            else {
                for (char cflg; (cflg = *++_arg); )

                    /**/ if (cflg == 'h') { ARG_HELP }
                    else if (cflg == 't') { ARG_TEST }
                    else if (cflg == 'c') { ARG_CODE }

                    else
                        ERR("Unknown character flag `-%c` in argument `%s`.\n", cflg, arg);
            }
        }
        else
            atomlist_push(pargs.sources, atom_string_newfl(arg));
    }


    pargs.parsing_successful = true;
    return pargs;
}
#undef ERR
#undef ARG_HELP
#undef ARG_TEST
#undef ARG_CODE
#undef ARG_STR
