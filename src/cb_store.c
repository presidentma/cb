#include "cb_store.h"
json init_root(json object,char *introduce)
{
    json root=cJSON_CreateArray();
    json intro = cJSON_CreateString(introduce);
    cJSON_AddItemToObject(object, CB_ROOT_INTRO_STRING, intro);
    cJSON_AddItemToObject(object, CB_ROOT_NAME_STRING, root);
    return root;
}

json add_group(json root,char *groupName,char *comment)
{
    json group = cJSON_CreateObject();
    json child=cJSON_CreateArray();
    json gName = cJSON_CreateString(groupName);
    json comments = cJSON_CreateString(comment);
    cJSON_AddItemToArray(root, group);
    cJSON_AddItemToObject(group, CB_GROUP_NAME_STRING, gName);
    cJSON_AddItemToObject(group, CB_GROUP_COMMENT_STRING, comments);
    cJSON_AddItemToObject(group, CB_GROUP_CHILD_STRING, child);
    return group;
}

json add_group_child(json group,char *shortCommand,char *command,char *comment)
{
    json child = cJSON_CreateObject();
    json sCommand = cJSON_CreateString(shortCommand);
    json commands = cJSON_CreateString(command);
    json comments = cJSON_CreateString(comment);
    printf("%d",group->child->next->next->type);
    /* has */
    if(!cJSON_HasObjectItem(group,CB_GROUP_CHILD_STRING)) return child;
    cJSON_AddItemToArray(cJSON_GetObjectItem(group,CB_GROUP_CHILD_STRING), child);
    cJSON_AddItemToObject(child, CB_CHILD_NAME_STRING, sCommand);
    cJSON_AddItemToObject(child, CB_CHILD_COMMAND_STRING, commands);
    cJSON_AddItemToObject(child, CB_CHILD_COMMENT_STRING, comments);
    return child;
}

char* get_json(json object){
    return cJSON_Print(object);
}

int write_json(char *string){
    FILE *file = fopen(CB_FILE_PATH,"w+");
    size_t count = fwrite(string,sizeof(char),strlen(string),file);
    fclose(file);
    return count;
}

int parse_init(json *object)
{
    FILE *file = fopen(CB_FILE_PATH,"r");
    if(file==NULL){
        fprintf(stderr, "open file failed\n");
        return CB_FAIL;
    }
    fseek(file,0,SEEK_END);
    unsigned long fileSize = ftell(file);
    fseek(file,0,SEEK_SET);
    char *jsonString = cb_malloc((fileSize+1)*sizeof(char));
    fread(jsonString,sizeof(char),fileSize,file);
    *object = cJSON_Parse(jsonString);
    if(*object==NULL){
        const char *errPtr = cJSON_GetErrorPtr();
        if (errPtr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", errPtr);
            return CB_FAIL;
        }
    }
    return CB_SUCCESS;
}

json parse_obj_item(json object,char *name)
{
    return cJSON_GetObjectItemCaseSensitive(object, name);
}
json parse_array_item(json object,int index)
{
    return cJSON_GetArrayItem(object, index);
}

int get_array_size(json array)
{
    return cJSON_GetArraySize(array);
}

