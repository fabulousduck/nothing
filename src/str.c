#include <stdlib.h>
#include <string.h>

#include "str.h"

char *string_duplicate(const char *str,
                       const char *str_end)
{
    if (str_end != NULL && str > str_end) {
        return NULL;
    }

    const size_t n = str_end == NULL ? strlen(str) : (size_t) (str_end - str);
    char *dup_str = malloc(sizeof(char) * (n + 1));
    if (dup_str == NULL) {
        return NULL;
    }

    memcpy(dup_str, str, n);
    dup_str[n] = '\0';

    return dup_str;
}
