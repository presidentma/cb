#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<unistd.h>
#include <sys/ioctl.h>


#define CB_SUCCESS (int)0
#define CB_FAIL (int)-1

#define cb_malloc malloc
#define cb_free free

#define ENV_VAR_USER "USER"
#define PROC_HOSTNAME "/proc/sys/kernel/hostname" 
#define ENV_VAR_HOME "HOME"

char* strdup(const char* str);
void get_hostname(int bufferSize, char *buffer);
char* get_file_path(char* fileName);