#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define CB_SUCCESS (int)0
#define CB_FAIL (int)-1

#define cb_malloc malloc
#define cb_free free

char* strdup(const char* str);
