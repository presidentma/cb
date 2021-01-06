#include "cb_utils.h"

char* strdup(const char* str)
{
    u_int len = strlen(str)+1;
    char *p = malloc(len);
    return p ? memcpy(p, str, len) : NULL;
}