/* cb_store.h Copyright presidentma */

json *init_root(json *object,char *introduce);

json *add_group(json *root,char *groupName,char *comment);

json *add_group_child(json *group,char *childName,char *shortCommand,char *command,char *comment);
