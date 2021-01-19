/* Copyright presidentma */
#include "cb_utils.h"
#include "cJSON.h"
#include <sys/stat.h>

typedef struct cJSON* json;

#define CB_FILE_NAME ".cb.json"
#define CB_FILE_BAK_NAME ".cb.json.bak"
#define CB_ROOT_NAME_STRING "root"
#define CB_ROOT_INTRO_STRING "introduce"
#define CB_GROUP_NAME_STRING "name"
#define CB_GROUP_COMMENT_STRING "comment"
#define CB_GROUP_CHILD_STRING "child"
#define CB_CHILD_NAME_STRING "shrt"
#define CB_CHILD_COMMAND_STRING "command"
#define CB_CHILD_COMMENT_STRING "comment"
/* init object */
json init_object();
/* init root */
json init_root(json object,char *introduce);
/* add group to the root */
json add_group(json root,char *groupName,char *comment);
/* add child to group */
json add_group_child(json group,char *shortCommand,char *command,char *comment);
/* get json string */
char* get_json(json object);
/* write json string to file */
int write_json(char* string);
/* parse json init */
int parse_init(json *object);
/* parse json item */
json parse_obj_item(json object,char *name);
json parse_array_item(json object,int index);
/* parse json array size */
int get_array_size(json array);