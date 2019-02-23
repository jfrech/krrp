#include <stdlib.h>
#include <stdio.h>

#include "options.h"
extern Options *GlobalOptions;

#include "error.h"

void error_malloc(const char *err) {
    if (GlobalOptions->color)
        fprintf(stderr, "%sMallocError%s :: %s\n", ERR, CLR, err);
    else
        fprintf(stderr, "MallocError :: %s\n", err);
}

// TODO: Make escaping a utility function and add colors
void print_escaped(const char *source, int p) {
    fprintf(stderr, "\n    ");

    int len = 0, width = 0;
    unsigned char c;
    for (int j = 0; (c = source[j]); j++) {
        if (c == '\t' || c == '\n') {
            fprintf(stderr, c == '\t' ? "\\t" : "\\n");
            if (j < p) len += 2;
            if (j == p) width = 2;
        }
        else if ((0 <= c && c <= 31) || c >= 127) {
            fprintf(stderr, "\\x%02x", c);
            if (j < p) len += 4;
            if (j == p) width = 4;
        }
        else {
            fprintf(stderr, "%c", c);
            if (j < p) len++;
            if (j == p) width = 1;
        }
    }

    fprintf(stderr, "\n    ");
    for (int j = 0; j < len; j++)
        fprintf(stderr, " ");
    for (int j = 0; j < width; j++)
        fprintf(stderr, "^");
    fprintf(stderr, "\n");
}

void error_parse(const char *source, int p, const char *err) {
    if (p < 0) {
        if (GlobalOptions->color)
            fprintf(stderr, "%sParseError%s :: %s\n", ERR, CLR, err);
        else
            fprintf(stderr, "ParseError :: %s\n", err);
        return;
    }

    print_escaped(source, p);
    if (GlobalOptions->color)
        fprintf(stderr, "%sParseError%s at byte %d :: %s\n", ERR, CLR, p, err);
    else
        fprintf(stderr, "ParseError at byte %d :: %s\n", p, err);
}

void warning_parse(const char *source, int p, const char *wrn) {
    print_escaped(source, p);
    if (GlobalOptions->color)
        fprintf(stderr, "%sParseWarning%s at byte %d :: %s\n", WRN, CLR, p, wrn);
    else
        fprintf(stderr, "ParseWarning at byte %d :: %s\n", p, wrn);
}
