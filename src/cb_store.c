#include "cb_store.h"

json *init_root(json *object,char *introduce)
{
    json *root=cJSON_CreateArray();
    cJSON_AddItemToObject(object, "introduce", introduce);
    cJSON_AddItemToObject(object, "root", array);
    return 0;
}

json *add_group(json *root,char *groupName,char *comment)
{
    json *group = cJSON_CreateObject();
    json *child=cJSON_CreateArray();
    cJSON_AddItemToArray(root, group);
    cJSON_AddItemToObject(group, "name", groupName);
    cJSON_AddItemToObject(group, "comment", comment);
    cJSON_AddItemToObject(group, "child", child);
    return group;
}

json *add_group_child(json *group,char *shortCommand,char *command,char *comment)
{
    json *child = cJSON_CreateObject();
    cJSON_AddItemToArray(group, child);
    cJSON_AddItemToObject(child, "shrt", shortCommand);
    cJSON_AddItemToObject(child, "command", command);
    cJSON_AddItemToObject(child, "comment", comment);
    return child;
}