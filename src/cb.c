/* cb.c Copyright presidentma */
#include "include/cb.h"

#define GETOPT_NO_ARGUMENT 0
#define GETOPT_REQUIRED_ARGUMENT 1
#define GETOPT_OPTIONAL_ARGUMENT 2
static const struct option longOptions[] = {
    {"group", GETOPT_REQUIRED_ARGUMENT, NULL, 'g'},
    {"list", GETOPT_NO_ARGUMENT, NULL, 'l'},
    {"add", GETOPT_NO_ARGUMENT, NULL, 'a'},
    {"short", GETOPT_OPTIONAL_ARGUMENT, NULL, 's'},
    {"delete", GETOPT_NO_ARGUMENT, NULL, 'd'},
    {"run", GETOPT_REQUIRED_ARGUMENT, NULL, 'r'},
    {"help", GETOPT_NO_ARGUMENT, NULL, 'h'}};
static cycle *cbCycle;
HashSet *hashMap;
char *groupName;
char *shrtName;
char *command;
char *comment;
bool exitExec = false;
char exitPrint[MAX_EXIT_PRINT_LEN];

int remalloc_size(int oldSize, int min)
{
    return oldSize << 1 > min ? oldSize << 1 : remalloc_size(oldSize << 1, min);
}

int init_cb(int argc, char *argv[])
{
    init_cycle();
    get_options(argc, argv);
    if (exitExec)
    {
        printf("%s", exitPrint);
        return 0;
    }
    exec_option();
    if (exitExec)
    {
        printf("%s", exitPrint);
        return 0;
    }
    destory_cycle();
    return 0;
}

/* 解析参数 */
void get_options(int argc, char *argv[])
{
    char *optstring = "g:las::drh";
    int optionIndex = 0;
    /* opt optarg optind */
    int opt;
    while ((opt = getopt_long(argc, argv, optstring, longOptions, &optionIndex)) != -1)
    {
        switch (opt)
        {
        case 'l':
            cbCycle->list = 1;
            break;
        case 'g':
            if (optarg)
            {
                if (strlen(optarg) + 1 > MAX_GROUP_NAME_LEN)
                {
                    exitExec = true;
                    snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "error! group name not more than %d characters!\n", MAX_GROUP_NAME_LEN - 1);
                    return;
                }
                strcpy(groupName, optarg);
                cbCycle->group = 1;
            }
            break;
        case 'a':
            cbCycle->add = 1;
            break;
        case 's':
            if (optarg)
            {
                if (strlen(optarg) + 1 > MAX_SHORT_NAME_LEN)
                {
                    exitExec = true;
                    snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "error! short command name not more than %d characters!\n", MAX_SHORT_NAME_LEN - 1);
                    return;
                }
                strcpy(shrtName, optarg);
            }
            cbCycle->shrt = 1;
            break;
        case 'd':
            cbCycle->delete = 1;
            break;
        case 'r':
            cbCycle->run = 1;
            break;
        case 'h':
            print_help();
            break;
        default:
            break;
        }
    }
}

void print_help()
{
    const char* helpString=
        "Usage: cb [option] [arg1] [option] [arg2]..."
        "\nCommand batch box:"
        "\n  **If no arg,it's run on the window interface"
        "\n  如果不带参数，默认以图形界面运行"
        "\n  --list    -l ... Show all groups and commands 显示所有组和命令"
        "\n  --delete  -d ... Delete group or command in the group 删除组或组中的命令"
        "\n  --group   -g ... Specify the group name 指定组名"
        "\n  --short   -s ... Specify the short command name,it's optional parameter 可选项，指定命令的短标签"
        "\n  --add     -a ... Add group or command,must Specify the -g 添加组或者给组添加命令，-g参数必须指定"
        "\n  --run     -r ... Run short command,it's must Specify the -g and -s 运行指定标签的命令"
        "\n  --help    -h ... help"
        "\nAuthor presidentma"
        "\nAuthor email <maliang.pr@qq.com>"
        "\nHome page: https://github.com/presidentma/cb"
        "\n";
    exitExec = true;
    snprintf(exitPrint, MAX_EXIT_PRINT_LEN,"%s", helpString);
    return;
}
void exec_option()
{
    int resCode = 0;
    /* ignore other params if exist -l parameter */
    if (cbCycle->list)
    {
        open_curses_terminal();
        return;
    }
    /* add short command */
    if (cbCycle->add && cbCycle->shrt)
    {
        if (!cbCycle->group)
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "missing -g or --group paramter!\n");
            return;
        }
        if (!hasGroup(cbCycle->hashSet, groupName))
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "group is not exist!\n");
            return;
        }
        char command[MAX_CMDLINE_LEN];
        printf("Please input the short command content:\n");
        fgets(command, MAX_CMDLINE_LEN, stdin);
        command[strlen(command) - 1] = 0;
        if (strlen(command) == 0)
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "command str length must between 1-%d!\n", MAX_CMDLINE_LEN - 1);
            return;
        }
        printf("Please input the short command comment(can be empty):\n");
        fgets(comment, MAX_COMMENT_LEN, stdin);
        comment[strlen(comment) - 1] = 0;
        resCode = add_shrt(groupName, shrtName, command, comment);
        if (resCode != CB_SUCCESS)
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "add short command failed!\n");
            return;
        }
        else
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "add short '%s' successed!\n", shrtName);
            return;
        }
    }
    /* add group */
    if (cbCycle->add && cbCycle->group)
    {
        if (hasGroup(cbCycle->hashSet, groupName))
        {
            char confirm[4];
            printf("Group %s already existed!Modify the group?(YES/NO)\n", groupName);
            fgets(confirm, 3, stdin);
            confirm[strlen(confirm) - 1] = 0;
            if (strcasecmp(confirm, "n") == 0 || strcasecmp(confirm, "no") == 0)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "group %s already existed!Cover be cancel!\n", groupName);
                return;
            }
            printf("Please input the new group name:\n");
            char newGroupName[MAX_GROUP_NAME_LEN];
            fgets(newGroupName, MAX_GROUP_NAME_LEN, stdin);
            newGroupName[strlen(newGroupName) - 1] = 0;
            printf("Please input the new group comment(can be empty):\n");
            fgets(comment, MAX_COMMENT_LEN, stdin);
            comment[strlen(comment) - 1] = 0;
            if (modify_group(newGroupName, groupName, comment) == CB_SUCCESS)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "modify group %s to %s is successed!\n", groupName, newGroupName);
                return;
            }
            else
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "modify group %s to %s is failed!\n", groupName, newGroupName);
                return;
            }
        }
        printf("Please input the new group comment(can be empty):\n");
        fgets(comment, MAX_COMMENT_LEN, stdin);
        comment[strlen(comment) - 1] = 0;
        resCode = _add_group(groupName, comment);
        if (resCode == CB_SUCCESS)
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "add group '%s' successed!\n", groupName);
            return;
        }
        else
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "add group failed!\n");
            return;
        }
    }
    /* run short command */
    if (cbCycle->run && cbCycle->shrt)
    {
        if (!cbCycle->group)
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "missing -g or --group paramter!\n");
            return;
        }
        if (!hasGroup(cbCycle->hashSet, groupName))
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "group is not exist!\n");
            return;
        }
        struct HashSetNodeP *group = hash_get_group(cbCycle->hashSet, groupName);
        struct HashSetNodeC *child = group->child;
        while (child)
        {
            if (strcasecmp(child->shrt, shrtName) == 0)
            {
                terminal_input(child->command);
                terminal_input("\n");
                return;
            }
            child = child->next;
        }
    }
    if (cbCycle->delete)
    {
        char confirm[4];
        if (!cbCycle->group)
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "missing -g or --group paramter!\n");
            return;
        }
        if (!hasGroup(cbCycle->hashSet, groupName))
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "group '%s' is not exist!\n", groupName);
            return;
        }
        if (cbCycle->shrt)
        {
            if (!has_shrt(groupName, shrtName))
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "shrt '%s' is not exist!\n", shrtName);
                return;
            }
            printf("confirm delete the %s command?(YES/NO)\n", shrtName);
            fgets(confirm, 3, stdin);
            confirm[strlen(confirm) - 1] = 0;
            if (strcasecmp(confirm, "n") == 0 || strcasecmp(confirm, "no") == 0||strcasecmp(confirm, "\n") == 0)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete the command '%s' be cancel!\n", shrtName);
                return;
            }
            if (delete_shrt(groupName, shrtName) == CB_SUCCESS)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete shrt '%s' is successed!\n", shrtName);
                return;
            }
            else
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete shrt '%s' is failed!\n", shrtName);
                return;
            }
        }
        if (cbCycle->group)
        {
            printf("confirm delete the %s group?(YES/NO)\n", groupName);
            fgets(confirm, 3, stdin);
            confirm[strlen(confirm) - 1] = 0;
            if (strcasecmp(confirm, "n") == 0 || strcasecmp(confirm, "no") == 0)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete group '%s' be cancel!\n", groupName);
                return;
            }
            if (delete_group(groupName) == CB_SUCCESS)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete group '%s' is successed!\n", groupName);
                return;
            }
            else
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete group '%s' is failed!\n", groupName);
                return;
            }
        }
    }
    /* if no match parameter */
    open_curses_terminal();
    return;
}

bool has_shrt(char *groupName, char *shortName)
{
    struct HashSetNodeP *group = hash_get_group(cbCycle->hashSet, groupName);
    struct HashSetNodeC *child = group->child;
    while (child)
    {
        if (strcasecmp(child->shrt, shortName) == 0)
        {
            return true;
        }
        child = child->next;
    }
    return false;
}
/* It's only to detele if have the same. */
int delete_shrt(char *groupName, char *shortName)
{
    struct HashSetNodeP *group = hash_get_group(cbCycle->hashSet, groupName);
    struct HashSetNodeC *child = group->child;
    if (!child)
        return CB_SUCCESS;
    struct HashSetNodeC *temp = group->child->next;
    if (group->child && strcasecmp(group->child->shrt, shortName) == 0)
    {
        temp = group->child;
        group->child = temp->next;
        cb_free(temp);
        return CB_SUCCESS;
    }
    while (temp)
    {
        if (strcasecmp(temp->shrt, shortName) == 0)
        {
            child->next = temp->next;
            cb_free(temp);
            temp = child->next;
            break;
        }
        else
        {
            child = child->next;
            temp = temp->next;
        }
    }
    return CB_SUCCESS;
}

int delete_group(char *groupName)
{
    int hash = hash_code(groupName);
    struct HashSetNodeP *group = cbCycle->hashSet->lists[hash];
    struct HashSetNodeC *child;
    if (group)
    {
        while (group->child)
        {
            child = group->child;
            group->child = group->child->next;
            cb_free(child);
        }
        cb_free(group);
        cbCycle->hashSet->lists[hash] = NULL;
    }
    return CB_SUCCESS;
}

int modify_group(char *newGroupName, char *oldGroupName, char *comment)
{
    int oldGroupHash = hash_code(oldGroupName);
    int newGroupHash = hash_code(newGroupName);
    if (strcasecmp(newGroupName, oldGroupName) == 0)
    {
        cbCycle->hashSet->lists[oldGroupHash]->value = comment;
        return CB_SUCCESS;
    }
    if (hashset_push(cbCycle->hashSet, newGroupName, comment) != CB_SUCCESS)
    {
        return CB_FAIL;
    }
    cbCycle->hashSet->lists[newGroupHash]->child = cbCycle->hashSet->lists[oldGroupHash]->child;
    cb_free(cbCycle->hashSet->lists[oldGroupHash]);
    cbCycle->hashSet->lists[oldGroupHash] = NULL;
    return CB_SUCCESS;
}
int _add_group(char *groupName, char *comment)
{
    return hashset_push(cbCycle->hashSet, groupName, comment);
}
int add_shrt(char *groupName, char *shrt, char *command, char *comment)
{
    struct HashSetNodeP *group = hash_get_group(cbCycle->hashSet, groupName);
    if (!group)
    {
        return CB_FAIL;
    }
    struct HashSetNodeC *child = group->child;
    struct HashSetNodeC *newChild = (struct HashSetNodeC *)cb_malloc(sizeof(struct HashSetNodeC));
    newChild->shrt = shrt;
    newChild->command = command;
    newChild->comment = comment;
    if(child){
        while (child->next)
        {
            child = child->next;
        }
        child->next = newChild;
    }else{
        group->child=newChild;
    }
    return CB_SUCCESS;
}

/* exit safely*/
void destory_cycle()
{
    /* export cb command */
    if (export_cb() != CB_SUCCESS)
    {
        return;
    }
    /* free cycle */
    struct HashSetNodeP *group;
    struct HashSetNodeC *child;
    for (int i = 0; i < HASH_MAP_SIZE; i++)
    {
        group = cbCycle->hashSet->lists[i];
        if (group)
        {
            while (group->child)
            {
                child = group->child;
                group->child = group->child->next;
                cb_free(child);
            }
            cb_free(group);
            cbCycle->hashSet->lists[i] = NULL;
        }
    }
    cb_free(cbCycle->hashSet);
    cb_free(cbCycle);
}

int export_cb()
{
    json object;
    json rootArray;
    json groupArray;
    struct stat statBuffer;
    if (stat(get_file_path(CB_FILE_NAME), &statBuffer) != 0)
    {
        exitExec = true;
        snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "get save file info failed,Please check whether the file exists!\n");
        return CB_FAIL;
    }
    char *jsonString = cb_malloc(statBuffer.st_size * 2 * sizeof(char));
    object = init_object();
    rootArray = init_root(object, cbCycle->rootComment);
    struct HashSetNodeP *group;
    struct HashSetNodeC *child;
    for (int i = 0; i < HASH_MAP_SIZE; i++)
    {
        group = cbCycle->hashSet->lists[i];
        if (group)
        {
            groupArray = add_group(rootArray, group->key, group->value);
            child = group->child;
            while (child)
            {
                add_group_child(groupArray, child->shrt, child->command, child->comment);
                child = child->next;
            }
            cb_free(group);
            cbCycle->hashSet->lists[i] = NULL;
        }
    }
    jsonString = get_json(object);
    rename(get_file_path(CB_FILE_NAME), get_file_path(CB_FILE_BAK_NAME));
    FILE *file = fopen(get_file_path(CB_FILE_NAME), "w");
    if (file == NULL)
    {
        exitExec = true;
        snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "Create save file error!Please check Whether the directory exists,and have promission!\n");
        return CB_FAIL;
    }
    fwrite(jsonString, sizeof(char), strlen(jsonString), file);
    fclose(file);
    return CB_SUCCESS;
}
int init_cycle()
{
    cbCycle = (cycle *)cb_malloc(sizeof(cycle));
    hashMap = (HashSet *)cb_malloc(sizeof(HashSet));
    cbCycle->list = 0;
    cbCycle->group = 0;
    cbCycle->add = 0;
    cbCycle->shrt = 0;
    cbCycle->delete = 0;
    cbCycle->deleteC = 0;
    cbCycle->run = 0;
    cbCycle->groupCount = 0;
    cbCycle->rootComment = "";
    cbCycle->helpY = 1;
    cbCycle->helpX = 0;
    cbCycle->titleY = 2;
    cbCycle->titleX = 0;
    cbCycle->maxY = 0;
    cbCycle->maxX = 0;
    cbCycle->selectCursorPosition = 0;
    cbCycle->selectShrtCursorPosition = 0;
    /* init memory */
    groupName = (char*)cb_calloc(MAX_GROUP_NAME_LEN,sizeof(char));
    shrtName = (char*)cb_calloc(MAX_SHORT_NAME_LEN,sizeof(char));
    command = (char*)cb_calloc(MAX_CMDLINE_LEN,sizeof(char));
    comment = (char*)cb_calloc(MAX_COMMENT_LEN,sizeof(char));
    int resCode;
    hashset_init(hashMap);
    json object;
    if (parse_init(&object)!=CB_SUCCESS)
    {
        return CB_FAIL;
    }
    
    resCode = create_hashset(cbCycle, object, hashMap);
    if (resCode != CB_SUCCESS)
        return resCode;
    cbCycle->hashSet = hashMap;
    return CB_SUCCESS;
}

int create_hashset(cycle *cbCycle, json object, HashSet *hashMap)
{
    json root = parse_obj_item(object, CB_ROOT_NAME_STRING);
    json rootIntr = parse_obj_item(object, CB_ROOT_INTRO_STRING);
    cbCycle->rootComment = rootIntr->valuestring;
    int size = get_array_size(root);
    if (rootIntr == NULL || size == 0)
    {
        return CB_SUCCESS;
    }
    json group;
    json groupName;
    json groupComment;
    int resCode;
    for (int i = 0; i < size; i++)
    {
        group = parse_array_item(root, i);
        groupName = parse_obj_item(group, CB_GROUP_NAME_STRING);
        groupComment = parse_obj_item(group, CB_GROUP_COMMENT_STRING);
        int resCode = hashset_push(hashMap, groupName->valuestring, groupComment->valuestring);
        if (resCode != CB_SUCCESS)
        {
            return resCode;
        }
        resCode = generate_child(group, hashMap->lists[hash_code(groupName->valuestring)]);
        if (resCode != CB_SUCCESS)
        {
            return resCode;
        }
    }
    return CB_SUCCESS;
}

int generate_child(json group, struct HashSetNodeP *p)
{
    json childArray;
    json child;
    json childShrt;
    json childComamnd;
    json childComment;
    struct HashSetNodeC *childSet;
    struct HashSetNodeC *temp;
    struct HashSetNodeC *head;
    childArray = parse_obj_item(group, CB_GROUP_CHILD_STRING);
    int size = get_array_size(childArray);
    if (size == 0)
    {
        return CB_SUCCESS;
    }
    childSet = (struct HashSetNodeC *)cb_malloc(sizeof(struct HashSetNodeC));
    childSet->next = NULL;
    p->child = childSet;
    for (int i = 0; i < size; i++)
    {
        child = parse_array_item(childArray, i);
        childShrt = parse_obj_item(child, CB_CHILD_NAME_STRING);
        childComamnd = parse_obj_item(child, CB_CHILD_COMMAND_STRING);
        childComment = parse_obj_item(child, CB_CHILD_COMMENT_STRING);
        temp = (struct HashSetNodeC *)cb_malloc(sizeof(struct HashSetNodeC));

        temp->shrt = childShrt->valuestring ? childShrt->valuestring : "";
        temp->command = childComamnd->valuestring ? childComamnd->valuestring : "";
        temp->comment = childComment->valuestring ? childComment->valuestring : "";
        childSet->next = temp;
        childSet = childSet->next;
    }
    childSet->next = NULL;
    /* delete head */
    head = p->child;
    p->child = head->next;
    cb_free(head);
    return CB_SUCCESS;
}

/* courses */
u_int promptItemStart = 0;

#define HOSTNAME_BUFFER_SIZE 128
void open_curses_terminal()
{
    signal(SIGINT, signal_callback_handler_ctrl_c);
    cb_curses_start();
    init_pair(CB_COLOR_NORMAL, COLOR_WHITE, COLOR_GREEN);
    attron(COLOR_PAIR(CB_COLOR_NORMAL));
    attroff(COLOR_PAIR(CB_COLOR_NORMAL));
    /*  */
    init_page(PRINT_TYPE_GROUP);

    int printGroupIndex[cbCycle->maxY - cbCycle->titleY + 1];
    cbCycle->printGroupIndex = printGroupIndex;
    /*  */
    bool done = false;
    bool promptInt = true;
    bool item = true;
    u_int operateCode = 0;
    char *cmd = "";
    bool execCommand = false;
    cbCycle->cmdline[0] = 0;
    /*  */
    while (!done)
    {
        if (promptInt)
        {
            attron(A_BOLD);
            print_prompt();
            attroff(A_BOLD);
            print_help_label();
            promptInt = false;
        }
        else
        {
            operateCode = wgetch(stdscr);
        }
        if (item)
        {
            print_group();
            item = false;
        }
        switch (operateCode)
        {
        case KEY_DOWN:
            if (cbCycle->delete){
                reset_delete();
                break;
            }
            if(cbCycle->add){
                switch_input(DIRECTION_DOWN);
                break;
            }
            if (cbCycle->printType == PRINT_TYPE_GROUP) //group select
            {
                if (cbCycle->selectCursorPosition < cbCycle->printItems)
                {
                    highline_selection(cbCycle->selectCursorPosition + 1, cbCycle->selectCursorPosition);
                    cbCycle->selectCursorPosition++;
                }
                else if (cbCycle->selectCursorPosition == cbCycle->printItems && cbCycle->printItems != 0)
                {
                    highline_selection(1, cbCycle->selectCursorPosition);
                    cbCycle->selectCursorPosition = 1;
                }
            }
            else //short command select
            {
                if (cbCycle->selectShrtCursorPosition < cbCycle->printItems)
                {
                    highline_selection(cbCycle->selectShrtCursorPosition + 1, cbCycle->selectShrtCursorPosition);
                    cbCycle->selectShrtCursorPosition++;
                }
                else if (cbCycle->selectShrtCursorPosition == cbCycle->printItems && cbCycle->printItems != 0)
                {
                    highline_selection(1, cbCycle->selectShrtCursorPosition);
                    cbCycle->selectShrtCursorPosition = 1;
                }
            }
            break;
        case K_ESC:
            if (cbCycle->delete){
                reset_delete();
                break;
            }
            if (cbCycle->add)
            {
                reset_add();
                break;
            }
            if(cbCycle->printType == PRINT_TYPE_COMMAND){
                cbCycle->printType = PRINT_TYPE_GROUP;
                reprint(true);
                break;
            }
            done=true;
            break;
        case KEY_UP:
            if (cbCycle->delete)
            {
                reset_delete();
                break;
            }
            if (cbCycle->add)
            {
                switch_input(DIRECTION_UP);
                break;
            }
            if (cbCycle->printType == PRINT_TYPE_GROUP) //group select
            {
                if (cbCycle->selectCursorPosition > 1)
                {
                    highline_selection(cbCycle->selectCursorPosition - 1, cbCycle->selectCursorPosition);
                    cbCycle->selectCursorPosition--;
                }
                else if (cbCycle->selectCursorPosition == 1)
                {
                    highline_selection(cbCycle->printItems, cbCycle->selectCursorPosition);
                    cbCycle->selectCursorPosition = cbCycle->printItems;
                }
            }
            else //short command select
            {
                if (cbCycle->selectShrtCursorPosition > 1)
                {
                    highline_selection(cbCycle->selectShrtCursorPosition - 1, cbCycle->selectShrtCursorPosition);
                    cbCycle->selectShrtCursorPosition--;
                }
                else if (cbCycle->selectShrtCursorPosition == 1)
                {
                    highline_selection(cbCycle->printItems, cbCycle->selectShrtCursorPosition);
                    cbCycle->selectShrtCursorPosition = cbCycle->printItems;
                }
            }
            break;
        case K_CTRL_A:
            if (cbCycle->delete||cbCycle->add)
                break;
            if (!cbCycle->add&&cbCycle->printType == PRINT_TYPE_COMMAND && cbCycle->selectCursorPosition != 0)
            {
                cbCycle->shrt = 1;
                cbCycle->add = 1;
                add_shrt_form();
            }
            if (!cbCycle->add&&cbCycle->printType == PRINT_TYPE_GROUP)
            {
                cbCycle->group = 1;
                cbCycle->add = 1;
                add_group_form();
            }
            break;
        case K_ENTER:
        case KEY_ENTER:
            if(cbCycle->add){
                if(cbCycle->group){
                    cbCycle->addParamsNum+=1;
                    add_group_form();
                }
                if(cbCycle->shrt){
                    cbCycle->addParamsNum+=1;
                    add_shrt_form();
                }
                break;
            }
            if (cbCycle->delete)
            {
                if (cbCycle->deleteC)
                {
                    if (cbCycle->printType == PRINT_TYPE_COMMAND && cbCycle->selectShrtCursorPosition != 0)
                    {
                        struct HashSetNodeP *group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[cbCycle->selectCursorPosition - 1]];
                        if (!group)
                            break;
                        struct HashSetNodeC *child = group->child;
                        for (int i = 1; (i < cbCycle->selectShrtCursorPosition && i < cbCycle->printItems); i++)
                        {
                            child = child->next;
                        }
                        delete_shrt(group->key, child->shrt);
                    }
                    else if (cbCycle->printType == PRINT_TYPE_GROUP && cbCycle->selectCursorPosition != 0)
                    {
                        struct HashSetNodeP *group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[cbCycle->selectCursorPosition - 1]];
                        if (!group)
                            break;
                        delete_group(group->key);
                    }
                    cbCycle->delete = 0;
                    cbCycle->deleteC = 0;
                    reprint(false);
                }
                else
                {
                    reset_delete();
                }
                break;
            }
            if (cbCycle->printType == PRINT_TYPE_GROUP && cbCycle->selectCursorPosition != 0)
            {
                if (strlen(cbCycle->cmdline))
                {
                    memset(cbCycle->cmdline, 0, sizeof(char) * strlen(cbCycle->cmdline));
                    mvprintw(cbCycle->promptY, cbCycle->promptX, "%s", cbCycle->cmdline);
                    clrtoeol();
                }
                cbCycle->selectShrtCursorPosition = 0;
                print_shrt();
            }
            else if (cbCycle->printType == PRINT_TYPE_COMMAND && cbCycle->selectShrtCursorPosition != 0)
            {
                struct HashSetNodeP *group;
                struct HashSetNodeC *child;
                group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[cbCycle->selectCursorPosition - 1]];
                child = group->child;
                for (int i = 1; (i < cbCycle->selectShrtCursorPosition && i < cbCycle->printItems); i++)
                {
                    child = child->next;
                }
                cmd = child->command;
                done = TRUE;
                execCommand = true;
            }
            break;
        case KEY_DC:
            if (cbCycle->delete)
                break;
            if (cbCycle->printType == PRINT_TYPE_COMMAND && cbCycle->selectShrtCursorPosition == 0)
            {
                break;
            }
            if (cbCycle->printType == PRINT_TYPE_GROUP && cbCycle->selectCursorPosition == 0)
            {
                break;
            }
            cbCycle->delete = 1;
            confirm_delete();
            break;
        case KEY_LEFT:
            if (cbCycle->delete)
            {
                switch_delete_option();
                break;
            }
            if (cbCycle->printType == PRINT_TYPE_COMMAND)
            {
                cbCycle->printType = PRINT_TYPE_GROUP;
                reprint(true);
            }
            break;
        case KEY_RIGHT:
            if (cbCycle->delete)
            {
                switch_delete_option();
                break;
            }
            if (cbCycle->printType == PRINT_TYPE_GROUP && cbCycle->selectCursorPosition != 0)
            {
                cbCycle->printType = PRINT_TYPE_COMMAND;
                cbCycle->selectShrtCursorPosition = 0;
                reprint(false);
            }
            break;
        case K_BACKSPACE:
        case KEY_BACKSPACE:
            if (cbCycle->delete){
                reset_delete();
                break;
            }
            if (cbCycle->add){
                if (cbCycle->group)
                    {
                        if(cbCycle->addParamsNum==0){
                            memset(groupName + strlen(groupName) - 1, 0, sizeof(char));
                        }
                        if(cbCycle->addParamsNum==1){
                            memset(comment + strlen(comment) - 1, 0, sizeof(char));
                        }
                        add_group_form();
                    }
                    if (cbCycle->shrt)
                    {
                        if(cbCycle->addParamsNum==0){
                            memset(shrtName + strlen(shrtName) - 1, 0, sizeof(char));
                        }else if(cbCycle->addParamsNum==1){
                            memset(command + strlen(command) - 1, 0, sizeof(char));
                        }else if(cbCycle->addParamsNum==2){
                            memset(comment + strlen(comment) - 1, 0, sizeof(char));
                        }
                        add_shrt_form();
                    }
                    break;
            }
            if (strlen(cbCycle->cmdline))
            {
                memset(cbCycle->cmdline + strlen(cbCycle->cmdline) - 1, 0, sizeof(char));
                mvprintw(cbCycle->promptY, cbCycle->promptX, "%s", cbCycle->cmdline);
                clrtoeol();
                reprint(false);
            }
            break;
        case KEY_RESIZE:
            if (cbCycle->delete)
            {
                reset_delete();
            }
            else
            {
                reprint(true);
                print_help_label();
            }
            break;
        default:
            if (operateCode >= K_SPACE && strlen(cbCycle->cmdline) < (cbCycle->maxX - cbCycle->promptX - 1))
            {
                if (cbCycle->add)
                {
                    if (cbCycle->group && strlen(groupName) < MAX_GROUP_NAME_LEN)
                    {
                        if(cbCycle->addParamsNum==0){
                            strncat(groupName, (char *)(&operateCode),MAX_GROUP_NAME_LEN-strlen(groupName));
                        }
                        if(cbCycle->addParamsNum==1){
                            strncat(comment, (char *)(&operateCode),MAX_COMMENT_LEN-strlen(comment));
                        }
                        add_group_form();
                    }
                    if (cbCycle->shrt && strlen(shrtName) < MAX_SHORT_NAME_LEN)
                    {
                        if(cbCycle->addParamsNum==0){
                            strncat(shrtName, (char *)(&operateCode),MAX_SHORT_NAME_LEN-strlen(shrtName));
                        }else if(cbCycle->addParamsNum==1){
                            strncat(command, (char *)(&operateCode),MAX_COMMENT_LEN-strlen(command));
                        }else if(cbCycle->addParamsNum==2){
                            strncat(comment, (char *)(&operateCode),MAX_COMMENT_LEN-strlen(comment));
                        }
                        add_shrt_form();
                    }
                    break;
                }
                if (cbCycle->printType == PRINT_TYPE_GROUP)
                {
                    cbCycle->selectCursorPosition = 0;
                }
                cbCycle->selectShrtCursorPosition = 0;
                strcat(cbCycle->cmdline, (char *)(&operateCode));
                mvprintw(cbCycle->promptY, cbCycle->promptX, "%s", cbCycle->cmdline);
                clrtoeol();
                reprint(false);
                refresh();
            }
            break;
        }
    }
    exit_curses();
    if (cmd != "")
    {
        terminal_input(cmd);
    }
    if (execCommand)
    {
        terminal_input("\n");
    }
}

void switch_input(int direction)
{
    if (cbCycle->group)
    {
        if (direction == DIRECTION_UP)
        {
            cbCycle->addParamsNum = cbCycle->addParamsNum == 0 ? 1 : cbCycle->addParamsNum-1;
        }
        else
        {
            cbCycle->addParamsNum = cbCycle->addParamsNum == 1 ? 0 : cbCycle->addParamsNum+1;
        }
        add_group_form();
    }
    else if (cbCycle->shrt)
    {
        if (direction == DIRECTION_UP)
        {
            cbCycle->addParamsNum = cbCycle->addParamsNum == 0 ? 2 : cbCycle->addParamsNum-1;
        }
        else
        {
            cbCycle->addParamsNum = cbCycle->addParamsNum == 2 ? 0 : cbCycle->addParamsNum+1;
        }
        add_shrt_form();
    }
}

void reset_add()
{
    if(cbCycle->group){
        groupName = (char*)cb_calloc(MAX_GROUP_NAME_LEN,sizeof(char));
        comment=(char*)cb_calloc(MAX_COMMENT_LEN,sizeof(char));
    }else{
        shrtName = (char*)cb_calloc(MAX_SHORT_NAME_LEN,sizeof(char));
        command = (char*)cb_calloc(MAX_CMDLINE_LEN,sizeof(char));
        comment = (char*)cb_calloc(MAX_COMMENT_LEN,sizeof(char));
    }
    cbCycle->group=0;
    cbCycle->shrt=0;
    cbCycle->add=0;
    cbCycle->addParamsNum=0;
    reprint(true);
}

void add_group_form()
{
    
    int thirdMaxY = cbCycle->maxY / 3;
    int thirdMaxX = cbCycle->maxX / 3;
    move(thirdMaxY, 0);
    clrtobot();
    attron(A_BOLD);
    attron(A_UNDERLINE);
    mvprintw(thirdMaxY, thirdMaxX, "**Add group**");
    attroff(A_UNDERLINE);
    attroff(A_BOLD);
    mvprintw(thirdMaxY + 1, thirdMaxX, "group name:%s",groupName);
    mvprintw(thirdMaxY + 2, thirdMaxX, "group comment:%s",comment);
    if(cbCycle->addParamsNum==0){
        move(thirdMaxY + 1, thirdMaxX + 11+strlen(groupName));
    }else if(cbCycle->addParamsNum==1){
        move(thirdMaxY + 2, thirdMaxX + 14+strlen(comment));
    }else{
        _add_group(groupName, comment);
        reset_add();
    }
    refresh();
}

void add_shrt_form()
{
    int thirdMaxY = cbCycle->maxY / 3;
    int thirdMaxX = cbCycle->maxX / 3;
    move(thirdMaxY, 0);
    clrtobot();
    attron(A_BOLD);
    attron(A_UNDERLINE);
    mvprintw(thirdMaxY, thirdMaxX, "**Add short command**");
    attroff(A_UNDERLINE);
    attroff(A_BOLD);
    mvprintw(thirdMaxY + 1, thirdMaxX, "short name:%s",shrtName);
    mvprintw(thirdMaxY + 2, thirdMaxX, "command string:%s",command);
    mvprintw(thirdMaxY + 3, thirdMaxX, "short comment:%s",comment);
    if(cbCycle->addParamsNum==0){
        move(thirdMaxY + 1, thirdMaxX + 11+strlen(shrtName));
    }else if(cbCycle->addParamsNum==1){
        move(thirdMaxY + 2, thirdMaxX + 15+strlen(command));
    }else if(cbCycle->addParamsNum==2){
        move(thirdMaxY + 3, thirdMaxX + 14+strlen(comment));
    }else{
        struct HashSetNodeP *group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[cbCycle->selectCursorPosition - 1]];
        printw("%s",group->key);
        refresh();
        if(group)add_shrt(group->key, shrtName,command,comment);
        reset_add();
    }
    refresh();
}

void switch_delete_option()
{
    cbCycle->deleteC = !cbCycle->deleteC;
    confirm_delete();
}

void reset_delete()
{
    cbCycle->delete = 0;
    cbCycle->deleteC = 0;
    reprint(true);
}

void confirm_delete()
{
    int thirdMaxY = cbCycle->maxY / 3;
    int halfMaxX = cbCycle->maxX / 2;
    if (cbCycle->delete)
    {
        move(thirdMaxY - 2, 0);
        clrtobot();
        attron(A_BOLD);
        mvprintw(thirdMaxY - 2, halfMaxX - 10, "——————————————————————");
        mvprintw(thirdMaxY, halfMaxX - 10, "——————————————————————");
        if (cbCycle->deleteC == 0)
        {
            attron(A_UNDERLINE);
            attron(A_REVERSE);
            attron(A_BLINK);
            mvprintw(thirdMaxY - 1, halfMaxX - 5, "NO");
            attroff(A_BLINK);
            attroff(A_REVERSE);
            attroff(A_UNDERLINE);
            mvprintw(thirdMaxY - 1, halfMaxX + 5, "YES");
        }
        else
        {
            mvprintw(thirdMaxY - 1, halfMaxX - 5, "NO");
            attron(A_UNDERLINE);
            attron(A_REVERSE);
            attron(A_BLINK);
            mvprintw(thirdMaxY - 1, halfMaxX + 5, "YES");
            attroff(A_BLINK);
            attroff(A_REVERSE);
            attroff(A_UNDERLINE);
        }
        attroff(A_BOLD);
        clrtoeol();
        move(cbCycle->promptY, cbCycle->promptX + strlen(cbCycle->cmdline));
        refresh();
    }
}

void terminal_input(char *cmd)
{
    if (cmd && strlen(cmd) > 0)
    {
        /* simulate terminal input */
        size_t size = strlen(cmd);
        char *c;
        for (int i = 0; i < size; i++)
        {
            c = (cmd + i);
            ioctl(0, TIOCSTI, c);
        }
    }
}

void reprint(bool highline)
{
    init_page(cbCycle->printType);
    if (cbCycle->printType == PRINT_TYPE_GROUP)
    {
        print_group();
        if (highline)
        {
            highline_selection(cbCycle->selectCursorPosition, cbCycle->selectCursorPosition - 1);
        }
        else
        {
            cbCycle->selectCursorPosition = 0;
        }
    }
    else if (cbCycle->printType == PRINT_TYPE_COMMAND)
    {
        print_shrt();
        if (highline)
        {
            highline_selection(cbCycle->selectShrtCursorPosition, cbCycle->selectShrtCursorPosition - 1);
        }
        else
        {
            cbCycle->selectShrtCursorPosition = 0;
        }
    }
    move(cbCycle->promptY, cbCycle->promptX + strlen(cbCycle->cmdline));
}

/*short command in the current selected group  */
void print_shrt()
{
    struct HashSetNodeP *group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[cbCycle->selectCursorPosition - 1]];
    if (!group)
        return;
    init_page(PRINT_TYPE_COMMAND);
    print_shrt_title();
    struct HashSetNodeC *child = group->child;
    char childBuffer[cbCycle->maxX];
    while (child)
    {
        if (strcasestr(child->shrt, cbCycle->cmdline) || strcasestr(child->command, cbCycle->cmdline) || strcasestr(child->comment, cbCycle->cmdline))
        {
            generate_shrt_str(childBuffer, cbCycle->maxX, child->shrt, child->command, child->comment);
            print_row(childBuffer, cbCycle->pageY, cbCycle->pageX);
            cbCycle->printItems++;
            cbCycle->pageY++;
            cbCycle->pageX = 0;
            child = child->next;
        }
        else
        {
            child = NULL;
        }
    }
    move(cbCycle->promptY, cbCycle->promptX + strlen(cbCycle->cmdline));
    refresh();
}

void init_page(int printType)
{
    cbCycle->pageY = cbCycle->titleY + 1;
    cbCycle->pageX = 0;
    cbCycle->titleX = 0;
    cbCycle->printType = printType;
    cbCycle->maxY = getmaxy(stdscr);
    cbCycle->maxX = getmaxx(stdscr);
    cbCycle->printItems = 0;
}

/* 高亮和A_REVERSE选中行 */
void highline_selection(int selectionPos, int preSelectionPos)
{
    if(selectionPos<=0)return;
    int preSelectY = cbCycle->titleY + preSelectionPos;
    int selectY = cbCycle->titleY + selectionPos;
    struct HashSetNodeP *group;
    struct HashSetNodeC *child;
    char preBuffer[cbCycle->maxX];
    char buffer[cbCycle->maxX];
    if (cbCycle->printType == PRINT_TYPE_GROUP) //group select
    {
        if (preSelectionPos > 0)
        {
            group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[preSelectionPos - 1]];
            generate_group_str(preBuffer,cbCycle->maxX, group->key, group->value);
            print_row(preBuffer, preSelectY, 0);
        }
        group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[selectionPos - 1]];
        generate_group_str(buffer,cbCycle->maxX, group->key, group->value);
    }
    else
    {
        group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[cbCycle->selectCursorPosition - 1]];
        if (preSelectionPos > 0)
        {
            child = group->child;
            for (int i = 1; (i < preSelectionPos && i < cbCycle->printItems); i++)
            {
                child = child->next;
            }
            generate_shrt_str(preBuffer, cbCycle->maxX, child->shrt, child->command, child->comment);
            print_row(preBuffer, preSelectY, 0);
        }
        child = group->child;
        for (int i = 1; (i < selectionPos && i < cbCycle->printItems); i++)
        {
            child = child->next;
        }
        generate_shrt_str(buffer, cbCycle->maxX, child->shrt, child->command, child->comment);
    }
    attron(A_REVERSE);
    print_highlighted_row(buffer, selectY, 0);
    attroff(A_REVERSE);
    refresh();
    move(cbCycle->promptY, cbCycle->promptX + strlen(cbCycle->cmdline));
}

void generate_group_str(char *buffer,int maxX, char *key, char *value)
{
    int namePos = maxX/3;
    snprintf(buffer,namePos,"%s",key);
    str_repeat(buffer," ",namePos-strlen(buffer));
    if(strlen(value)<maxX-strlen(buffer)){
        strncat(buffer,value,maxX-strlen(buffer)-1);
        str_repeat(buffer," ",maxX-strlen(buffer)-1);
    }else{
        strncat(buffer,value,maxX-strlen(buffer)-4);
        strcat(buffer,"...");
    }
}

void generate_shrt_str(char *buffer, int maxX, char *shrt, char *command, char *comment)
{
    int shrtPos = maxX/4;
    int commandPos = maxX/4*3;
    snprintf(buffer,shrtPos,"%s",shrt);
    str_repeat(buffer," ",shrtPos-strlen(buffer));
    if(strlen(command)<commandPos-strlen(buffer)){
        strncat(buffer,command,commandPos-strlen(buffer));
        str_repeat(buffer," ",commandPos-strlen(buffer));
    }else{
        strncat(buffer,command,commandPos-strlen(buffer)-3);
        strcat(buffer,"...");
    }
    if(strlen(comment)<maxX-strlen(buffer)){
        strncat(buffer,comment,maxX-strlen(buffer)-1);
        str_repeat(buffer," ",maxX-strlen(buffer)-1);
    }else{
        strncat(buffer,comment,maxX-strlen(buffer)-4);
        strcat(buffer,"...");
    }
}

void print_group_title()
{
    char buffer[cbCycle->maxX];
    snprintf(buffer, cbCycle->maxX,"%5s","NAME");
    str_repeat(buffer," ",cbCycle->maxX/3-strlen(buffer));
    strcat(buffer,"COMMENT");
    str_repeat(buffer," ",cbCycle->maxX-strlen(buffer)-1);
    print_title(buffer);
}

void print_shrt_title()
{
    char buffer[cbCycle->maxX];
    snprintf(buffer, cbCycle->maxX, "%5s","SHORT");
    str_repeat(buffer," ",cbCycle->maxX/4-strlen(buffer));
    strcat(buffer,"COMMAND");
    str_repeat(buffer," ",cbCycle->maxX/4*3-strlen(buffer));
    strcat(buffer,"COMMEND");
    str_repeat(buffer," ",cbCycle->maxX-strlen(buffer)-1);
    print_title(buffer);
}

void print_title(char *buffer)
{
    move(cbCycle->titleY, cbCycle->titleX);
    clrtobot();
    attron(A_REVERSE);
    print_highlighted_row(buffer, cbCycle->titleY, cbCycle->titleX);
    attroff(A_REVERSE);
    refresh();
    move(cbCycle->promptY, cbCycle->promptX + strlen(cbCycle->cmdline));
}

char *str_repeat(char *buffer, char *c, int num)
{
    int i = 0;
    for (; i < num; i++)
    {
        strcat(buffer, c);
    }
}

void print_highlighted_row(char *buffer, u_int y, u_int x)
{
    attron(A_BOLD);
    print_row(buffer, y, x);
    attroff(A_BOLD);
}
void print_row(char *buffer, u_int y, u_int x)
{
    mvprintw(y, x, "%s", buffer);
    clrtoeol();
}
/* print group */
void print_group()
{
    struct HashSetNodeP *group;
    int printGroupIndexIndex = 0;
    char buffer[cbCycle->maxX];
    print_group_title();
    for (u_int i = 0; i < HASH_MAP_SIZE; i++)
    {
        group = cbCycle->hashSet->lists[i];
        if (group && cbCycle->pageY < cbCycle->maxY && (strcasestr(group->key, cbCycle->cmdline) || strcasestr(group->value, cbCycle->cmdline)))
        {
            generate_group_str(buffer,cbCycle->maxX, group->key, group->value);
            print_row(buffer, cbCycle->pageY, cbCycle->pageX);
            cbCycle->printItems++;
            cbCycle->printGroupIndex[printGroupIndexIndex] = i;
            printGroupIndexIndex++;
            cbCycle->pageY++;
            cbCycle->pageX = 0;
        }
    }
    move(cbCycle->promptY, cbCycle->promptX + strlen(cbCycle->cmdline));
    refresh();
}

/* 提示栏 */
void print_prompt()
{
    char *user = getenv(ENV_VAR_USER);
    char *hostname = (char *)malloc(HOSTNAME_BUFFER_SIZE);
    user = user ? user : "me";
    get_hostname(HOSTNAME_BUFFER_SIZE, hostname);
    mvprintw(cbCycle->promptY, cbCycle->promptX, "%s@%s$ ", user, hostname);
    cbCycle->promptX += (strlen(user) + strlen(hostname) + 3);
    free(hostname);
    refresh();
}

void print_help_label()
{
    const char *helpStr=
    "Type to filter,UP/DOWN move,ENTER select,CTRL+A add item,DEL delete item,CTRL+C exit,ESC back";
    char printBuffer[cbCycle->maxX];
    if(strlen(helpStr)<cbCycle->maxX-1){
        strncpy(printBuffer,helpStr,cbCycle->maxX-1);
    }else{
        strncpy(printBuffer,helpStr,cbCycle->maxX-4);
        strcat(printBuffer,"...");
    }
    mvprintw(cbCycle->helpY,cbCycle->helpX,printBuffer);
    move(cbCycle->promptY, cbCycle->promptX);
    refresh();
}

void signal_callback_handler_ctrl_c(int signNum)
{
    if (signNum == SIGINT)
    {
        exit_curses();
        destory_cycle();
        exit(signNum);
    }
}
