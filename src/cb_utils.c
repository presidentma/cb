#include "cb_utils.h"

char* strdup(const char* str)
{
    u_int len = strlen(str)+1;
    char *p = malloc(len);
    return p ? memcpy(p, str, len) : NULL;
}

void get_hostname(int bufferSize, char *buffer)
{
    char *b=buffer;
    printf("%s\n",buffer);
    if(access(PROC_HOSTNAME, F_OK) != -1) {
        FILE *file = fopen(PROC_HOSTNAME, "r");
        b=fgets(buffer, bufferSize, file);
        fclose(file);
        if(b) {
            b[strlen(b)-1]=0;
            return;
        }
    }
    strcpy(buffer, "localhost");
}