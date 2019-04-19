#include "Opt.h"


Opt GlobOpt = (Opt) {
    .ERR = true,
    .WRN = true,
    .INF = false,
    .maximum_interpretation_recursion_depth = 2048,
    .string_view = false,

    .pedantic = false
};
