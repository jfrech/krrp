#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "atomlist.h"
#include "atom.h"
#include "memorymanagement.h"

PArgs *parse_args(PArgs *pargs, int argc, char **argv) {
    #define ERR(...) return fprintf(stderr, __VA_ARGS__), NULL


    

    AtomList *sources = new_boxed_atomlist();
    bool do_test = false;
    bool print_help = false;
    {
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
                        print_help = true;
                    else if (strcmp(arg, "--test") == 0)
                        do_test = true;
                    else
                        ERR("Unknown argument `%s`.\n", arg);
                }
                else {
                    for (char cflg; (cflg = *++_arg); )
                        if (cflg == 'h')
                            print_help = true;
                        else if (cflg == 't')
                            do_test = true;
                        else
                            ERR("Unknown character flag `-%c` in argument `%s`.\n", cflg, arg);
                }
            }
            else
                atomlist_push(sources, atom_string_newfl(arg));
        }
    }
    #undef ERR

    pargs->do_test = do_test;
    pargs->print_help = print_help;
    pargs->sources = sources;


    return pargs;
}
