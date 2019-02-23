#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "options.h"
extern Options *GlobalOptions;

char *strdup(const char *str) {
    char *dup = malloc((strlen(str)+1) * sizeof *dup);
    if (!dup)
        return error_malloc("strdup"), NULL;

    strcpy(dup, str);

    return dup;
}

#define HGHLEN 7
#define HGH "\033[1;32m"
#define CLRLEN 4
#define CLR "\033[0m"

static int escaped_character_length(unsigned char c) {
    int z = GlobalOptions->color ? HGHLEN+CLRLEN : 0;

    if (c == '\t' || c == '\n' || c == '"' || c == '\\')
        return 2+z;
    if ((0 <= c && c <= 31) || (127 <= c && c <= 255))
        return 4+z;
    return 1+z;
}

char *stresc(const char *str) {
    int len = 0;
    for (int j = 0; str[j]; j++)
        len += escaped_character_length(str[j]);

    char *estr = malloc((len+1) * sizeof *str), *s = estr;
    unsigned char c;

    for (int j = 0; (c = str[j]); j++) {
        if ((32 <= c && c <= 126) && c != '"' && c != '\\') s += sprintf(s, "%c", c);
        else {
            if (GlobalOptions->color) s += sprintf(s, HGH);
            if (c == '\t') s += sprintf(s, "\\t");
            else if (c == '\n') s += sprintf(s, "\\n");
            else if (c == '"') s += sprintf(s, "\\\"");
            else if (c == '\\') s += sprintf(s, "\\\\");
            else if ((0 <= c && c <= 31) || (127 <= c && c <= 255)) s += sprintf(s, "\\x%2X", c);
            if (GlobalOptions->color) s += sprintf(s, CLR);
        }
    }

    estr[len] = '\0';
    return estr;
}
