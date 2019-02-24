#include <stdlib.h>
#include <stdbool.h>

#include "error.h"
#include "options.h"
#include "memorymanagement.h"

Options *GlobalOptions;

void globaloptions_init() {
    GlobalOptions = mm_malloc("globaloptions_init", sizeof *GlobalOptions);

    if (!GlobalOptions) {
        error_malloc("globaloptions_init");
        return;
    }

    GlobalOptions->color = true;
    GlobalOptions->debug_address = false;
    GlobalOptions->debug_printGlobalAtomTable = !true; // TODO
    GlobalOptions->pedantic_scope_verification = true;

    GlobalOptions->maximum_interpretation_recursion_depth = 2048;
}
