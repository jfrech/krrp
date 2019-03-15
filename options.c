#include <stdlib.h>
#include <stdbool.h>

#include "error.h"
#include "options.h"
#include "memorymanagement.h"

Options *GlobalOptions;

void globaloptions_init() {
    GlobalOptions = mm_malloc("globaloptions_init", sizeof *GlobalOptions);

    GlobalOptions->debug_address = false;
    GlobalOptions->debug_printGlobalAtomTable = !true; // TODO
    GlobalOptions->pedantic_scope_verification = true;

    GlobalOptions->maximum_interpretation_recursion_depth = 2048;
}
