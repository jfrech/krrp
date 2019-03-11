#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "atomlist.h"
#include "atom.h"
#include "memorymanagement.h"


PArgs parse_args(int argc, char **argv) {
    PArgs pargs = (PArgs) {
        .parsing_successful = false,

        .do_test = false,
        .print_help = false,
        .sources = new_boxed_atomlist()
    };


    #define ERR(...) return fprintf(stderr, __VA_ARGS__), pargs
    if (argc < 1)
        ERR("Please specify a krrp source file. Use `-h` for help.\n");

    bool interpret_arguments = true;

    for (int j = 1; j < argc; j++) {
        const char *arg = argv[j], *_arg = arg;

        if (interpret_arguments && arg[0] == '-') {
            if (arg[1] == '\0')
                ERR("Unknown argument `%s`.\n", arg);

            if (arg[1] == '-') {
                if (arg[2] == '\0')
                    interpret_arguments = false;
                else if (strcmp(arg, "--help") == 0)
                    pargs.print_help = true;
                else if (strcmp(arg, "--test") == 0)
                    pargs.do_test = true;
                else
                    ERR("Unknown argument `%s`.\n", arg);
            }
            else {
                for (char cflg; (cflg = *++_arg); )
                    if (cflg == 'h')
                        pargs.print_help = true;
                    else if (cflg == 't')
                        pargs.do_test = true;
                    else
                        ERR("Unknown character flag `-%c` in argument `%s`.\n", cflg, arg);
            }
        }
        else
            atomlist_push(pargs.sources, atom_string_newfl(arg));
    }
    #undef ERR

    pargs.parsing_successful = true;
    return pargs;
}
