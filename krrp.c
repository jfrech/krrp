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

// TODO: potentially implement AtomList as a doubly linked list.
// TODO: potentially implement own special color markup

int main() {
    globaloptions_init();
    globalatomtable_init();


    test_all();


    // const char *source = "![factorial]^n:?n*n@-n11. ![choose]^nk:!f;[factorial]/fn*fkf-nk. [choose]83";
    const char *source = "~ List\n!E#E. !L#Lfr.\n~ Tuple\n!T#Tlr.\n![range_step]^abs:?<abLa@+asbsE. ![range]^ab:[range_step]ab1. T[range]$-13.6[range_step]$-20.$87.8";

    printf("=== Source ===\n");
    char *esource = stresc(source);
    printf(".> \"%s\"\n", esource);
    free(esource);

    printf("\n=== Parsing ===\n");
    AtomList *parsed = parse(source);
    printf(".> %s\n", atomlist_str(parsed));

    printf("\n=== Interpreting ===\n");
    Atom *scope = main_scope();
    while (!atomlist_empty(parsed))
        printf(".> %s\n", atom_repr(interpret(parsed, scope, true)));
    printf("\n");

    atomlist_free(parsed);


    memorymanagement_free_all();
}
