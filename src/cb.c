/* main.c Copyright presidentma */
#include "cb.h"

int main(){
    typedef cJSON json;
    char *jsonString = NULL;
    json *root = NULL;
    json *group = NULL;


    json *const object = cJSON_CreateObject();
    root = init_root(object,"command set");
    group = add_group(root,"nginx","nginx命令");
    jsonString = cJSON_Print(object);

    FILE *file = fopen("cb.json","w+");
    fwrite(jsonString,sizeof(char),sizeof(jsonString),file);
    fclose(file);
    printf("generate over!");
    return 0;
}
