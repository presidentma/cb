/* main.c Copyright presidentma */
#include "cb.h"

int main(int argc,char* argv[])
{
    setlocale(LC_ALL, "");
    /* HashSet commandMap;
    hashset_init(&commandMap); */
    init_cb();
    return 0;
}

void parse_argv(int argc,char* argv[])
{
    command_t *commandTest = cb_malloc(sizeof(command_t));
    commandTest->last = commandTest->str = cb_malloc(COMMAND_STR_LEN_DEFAULT);
   
    commandTest->len = COMMAND_STR_LEN_DEFAULT;
    for(int i=0;i<argc;){
        if((commandTest->len-(commandTest->last-commandTest->str))<strlen(argv[i])){
            command_t *temp = commandTest;
            commandTest = cb_malloc(sizeof(command_t));
            int size = remalloc_size(temp->len,strlen(argv[i]));
            commandTest->str = cb_malloc(size);
            commandTest->len = size;
            memcpy(commandTest->str,temp->str,(temp->last-temp->str));
            commandTest->last = commandTest->str + (temp->last-temp->str);
            cb_free(temp);
        }
        memcpy(commandTest->last,argv[i],strlen(argv[i]));
        commandTest->last += strlen(argv[i]);
        i++;
    }
    printf("%s\n",commandTest->str);
}

int remalloc_size(int oldSize,int min)
{
    return oldSize<<1>min?oldSize<<1:remalloc_size(oldSize<<1,min);
}

int init_cb()
{
    cycle cbCycle;
    init_cycle(&cbCycle);
    

}

int init_cycle(cycle* cbCycle)
{
    json object;
    if(parse_init(&object)){
        return CB_FAIL;
    }
    HashSet hashMap;
    int resCode;
    hashset_init(&hashMap);
    resCode = create_hashset(cbCycle,object,&hashMap);
    if(resCode!=CB_SUCCESS) return resCode;
    cbCycle->hashSet=&hashMap;
    printf("cbCycle:%s\n",cbCycle->hashSet->lists[hash_code("nginx")]->child->command);
    return CB_SUCCESS;
}

int create_hashset(cycle* cbCycle,json object,HashSet* hashMap)
{
    json root = parse_obj_item(object,CB_ROOT_NAME_STRING);
    json rootIntr = parse_obj_item(object,CB_ROOT_INTRO_STRING);
    
    int size = get_array_size(root);
    if(rootIntr==NULL||size==0){
        return CB_FAIL;
    }
    cbCycle->rootComment = rootIntr->valuestring;
    json group;
    json groupName;
    json groupComment;
    int resCode;
    for(int i=0;i<size;i++){
        group = parse_array_item(root,i);
        groupName = parse_obj_item(group,CB_GROUP_NAME_STRING);
        groupComment = parse_obj_item(group,CB_GROUP_COMMENT_STRING);
        int resCode = hashset_push(hashMap,groupName->valuestring,groupComment->valuestring);
        if(resCode!=CB_SUCCESS){
            return resCode;
        }
        resCode = generate_child(group,&hashMap->lists[hash_code(groupName->valuestring)]);
        if(resCode!=CB_SUCCESS){
            return resCode;
        }
    }
    return CB_SUCCESS;
}

int generate_child(json group,struct HashSetNodeP **p)
{
    json childArray;
    json child;
    json childShrt;
    json childComamnd;
    json childComment;
    struct HashSetNodeC *childSet;
    struct HashSetNodeC *temp;
    struct HashSetNodeC *head;
    childArray = parse_obj_item(group,CB_GROUP_CHILD_STRING);
    int size=get_array_size(childArray);
    if(size==0){
        return CB_FAIL;
    }
    childSet = (struct HashSetNodeC*)cb_malloc(sizeof(struct HashSetNodeC));
    childSet->next = NULL;
    (*p)->child=childSet;
    for(int i=0;i<size;i++){
        child = parse_array_item(childArray,i);
        childShrt = parse_obj_item(child,CB_CHILD_NAME_STRING);
        childComamnd = parse_obj_item(child,CB_CHILD_COMMAND_STRING);
        childComment = parse_obj_item(child,CB_CHILD_COMMENT_STRING);
        temp = (struct HashSetNodeC*)cb_malloc(sizeof(struct HashSetNodeC));
        
        temp->shrt = childShrt->valuestring?childShrt->valuestring:"";
        temp->command = childComamnd->valuestring?childComamnd->valuestring:"";
        temp->comment = childComment->valuestring?childComment->valuestring:"";
        childSet->next = temp;
        childSet = childSet->next;
    }
    childSet->next = NULL;
    /* delete head */
    head = (*p)->child;
    (*p)->child = head->next;
    cb_free(head);
    return CB_SUCCESS;
}
