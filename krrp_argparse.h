AtomList *al_sources = new_boxed_atomlist();
bool test_mode = false;
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
        else
            atomlist_push(al_sources, atom_string_newfl(arg));
    }
}
