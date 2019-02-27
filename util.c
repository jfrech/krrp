#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "options.h"
#include "memorymanagement.h"
extern Options *GlobalOptions;

char *strdup(const char *str) {
    char *dup = mm_malloc("strdup", (strlen(str)+1) * sizeof *dup);
    strcpy(dup, str);

    return dup;
}

static int escaped_character_length(unsigned char c) {
    if (c == '\t' || c == '\n' || c == '"' || c == '\\')
        return 2;
    if ((0 <= c && c <= 31) || (127 <= c && c <= 255))
        return 4;
    return 1;
}

char *stresc(const char *str) {
    int len = 0;
    for (int j = 0; str[j]; j++)
        len += escaped_character_length(str[j]);

    char *estr = mm_malloc("stresc", (len+1) * sizeof *str), *s = estr;
    unsigned char c;

    for (int j = 0; (c = str[j]); j++) {
        if ((32 <= c && c <= 126) && c != '"' && c != '\\') s += sprintf(s, "%c", c);
        else {
            if (c == '\t') s += sprintf(s, "\\t");
            else if (c == '\n') s += sprintf(s, "\\n");
            else if (c == '"') s += sprintf(s, "\\\"");
            else if (c == '\\') s += sprintf(s, "\\\\");
            else if ((0 <= c && c <= 31) || (127 <= c && c <= 255)) s += sprintf(s, "\\x%2X", c);
        }
    }

    estr[len] = '\0';
    return estr;
}

void print_escaped_source(const char *source) {
    info("=== Source ===\n");
    char *esource = stresc(source);
    info(".> \"%s\"\n", esource);
    mm_free("print_escaped_source", esource);
}
