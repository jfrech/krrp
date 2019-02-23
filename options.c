#include <stdlib.h>
#include <stdbool.h>

#include "error.h"
#include "options.h"

Options *GlobalOptions;

void globaloptions_init() {
    GlobalOptions = malloc(sizeof *GlobalOptions);

    if (!GlobalOptions) {
        error_malloc("globaloptions_init");
        return;
    }

    GlobalOptions->color = true;
    GlobalOptions->debug_address = false;
    GlobalOptions->debug_printGlobalAtomTable = !true; // TODO
}
