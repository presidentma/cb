/* main.c Copyright presidentma */
#include "cb.h"

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
char groupName[MAX_GROUP_NAME_LEN];
char shrtName[MAX_SHORT_NAME_LEN];
char comment[MAX_COMMENT_LEN];
bool exitExec = false;
char exitPrint[MAX_EXIT_PRINT_LEN];
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    init_cb(argc, argv);
    return 0;
}

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
        destory_cycle();
        printf("%s", exitPrint);
        return 0;
    }
    exec_option();
    if (exitExec)
    {
        destory_cycle();
        printf("%s", exitPrint);
        return 0;
    }
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
        command[strlen(command)-1]=0;
        if (strlen(command) == 0)
        {
            exitExec = true;
            snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "command str length must between 1-%d!\n", MAX_CMDLINE_LEN - 1);
            return;
        }
        printf("Please input the short command comment(can be empty):\n");
        fgets(comment, MAX_COMMENT_LEN, stdin);
        comment[strlen(comment)-1]=0;
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
            confirm[strlen(confirm)-1]=0;
            if (strcasecmp(confirm, "n") == 0 || strcasecmp(confirm, "no") == 0)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "group %s already existed!Cover be cancel!\n", groupName);
                return;
            }
            printf("Please input the new group name:\n");
            char newGroupName[MAX_GROUP_NAME_LEN];
            fgets(newGroupName, MAX_GROUP_NAME_LEN, stdin);
            newGroupName[strlen(newGroupName)-1]=0;
            printf("Please input the new group comment(can be empty):\n");
            fgets(comment, MAX_COMMENT_LEN, stdin);
            comment[strlen(comment)-1]=0;
            if (modify_group(newGroupName, groupName, comment) == CB_SUCCESS)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "modify group %s to %s is successed!\n", groupName, newGroupName);
                return;
            }else{
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "modify group %s to %s is failed!\n", groupName, newGroupName);
                return;
            }
        }
        printf("Please input the new group comment(can be empty):\n");
        fgets(comment, MAX_COMMENT_LEN, stdin);
        comment[strlen(comment)-1]=0;
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
        if (cbCycle->shrt)
        {
            if (!has_shrt(groupName, shrtName))
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "shrt is not exist!\n");
                return;
            }
            if (delete_shrt(groupName, shrtName) == CB_SUCCESS)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete shrt is successed!\n");
                return;
            }
            else
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete shrt is failed!\n");
                return;
            }
        }
        if (cbCycle->group)
        {
            if (delete_group(groupName) == CB_SUCCESS)
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete group is successed!\n");
                return;
            }
            else
            {
                exitExec = true;
                snprintf(exitPrint, MAX_EXIT_PRINT_LEN, "delete group is failed!\n");
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

int delete_shrt(char *groupName, char *shortName)
{
    struct HashSetNodeP *group = hash_get_group(cbCycle->hashSet, groupName);
    struct HashSetNodeC *child = group->child;
    while (child)
    {
        if (strcasecmp(child->shrt, shortName) == 0)
        {
            cb_free(child);
        }
        child = child->next;
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
    while (child->next)
    {
        child = child->next;
    }
    struct HashSetNodeC *newChild = (struct HashSetNodeC *)cb_malloc(sizeof(struct HashSetNodeC));
    newChild->shrt = shrt;
    newChild->command = command;
    newChild->comment = comment;
    child->next = newChild;
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
    if (stat(CB_FILE_PATH, &statBuffer) != 0)
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
    rename(CB_FILE_PATH, CB_FILE_BAK_PATH);
    FILE *file = fopen(CB_FILE_PATH, "w");
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
    json object;
    if (parse_init(&object))
    {
        return CB_FAIL;
    }
    cbCycle->list = 0;
    cbCycle->group = 0;
    cbCycle->add = 0;
    cbCycle->shrt = 0;
    cbCycle->delete = 0;
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
    int resCode;
    hashset_init(hashMap);
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

    int size = get_array_size(root);
    if (rootIntr == NULL || size == 0)
    {
        return CB_FAIL;
    }
    cbCycle->rootComment = rootIntr->valuestring;
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
        return CB_FAIL;
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
                else if (cbCycle->selectShrtCursorPosition == cbCycle->printItems)
                {
                    highline_selection(1, cbCycle->selectShrtCursorPosition);
                    cbCycle->selectShrtCursorPosition = 1;
                }
            }
            break;
        case KEY_UP:
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
        case K_ENTER:
        case KEY_ENTER:
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

            break;
        case KEY_LEFT:
            if (cbCycle->printType == PRINT_TYPE_COMMAND && cbCycle->selectShrtCursorPosition != 0)
            {
                cbCycle->printType = PRINT_TYPE_GROUP;
                reprint(true);
            }
            break;
        case KEY_RIGHT:
            if (cbCycle->printType == PRINT_TYPE_GROUP && cbCycle->selectCursorPosition != 0)
            {
                cbCycle->printType = PRINT_TYPE_COMMAND;
                cbCycle->selectShrtCursorPosition = 0;
                reprint(false);
            }
            break;
        case K_BACKSPACE:
        case KEY_BACKSPACE:
            if (strlen(cbCycle->cmdline))
            {
                memset(cbCycle->cmdline + strlen(cbCycle->cmdline) - 1, 0, sizeof(char));
                mvprintw(cbCycle->promptY, cbCycle->promptX, "%s", cbCycle->cmdline);
                clrtoeol();
                reprint(false);
            }
            break;
        case KEY_RESIZE:
            reprint(true);
            break;

        default:
            if (operateCode >= K_SPACE && strlen(cbCycle->cmdline) < (cbCycle->maxX - cbCycle->promptX - 1))
            {
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
    }
    else if (cbCycle->printType == PRINT_TYPE_COMMAND)
    {
        print_shrt();
        if (highline)
        {
            highline_selection(cbCycle->selectShrtCursorPosition, cbCycle->selectShrtCursorPosition - 1);
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
            generate_group_str(preBuffer, cbCycle->maxX, group->key, group->value);
            print_row(preBuffer, preSelectY, 0);
        }
        group = cbCycle->hashSet->lists[cbCycle->printGroupIndex[selectionPos - 1]];
        generate_group_str(buffer, cbCycle->maxX, group->key, group->value);
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

void generate_group_str(char *buffer, int maxX, char *key, char *value)
{
    snprintf(buffer, maxX, "%15s%15s%-30s", key, " ", value);
    if (strlen(buffer) < maxX)
    {
        str_repeat(buffer, " ", maxX - strlen(buffer) - 1);
    }
}

void generate_shrt_str(char *buffer, int maxX, char *shrt, char *command, char *comment)
{
    snprintf(buffer, maxX, "%15s%25s%25s", shrt, command, comment);
    if (strlen(buffer) < maxX)
    {
        str_repeat(buffer, " ", maxX - strlen(buffer) - 1);
    }
}

void print_group_title()
{
    char buffer[cbCycle->maxX];
    snprintf(buffer, cbCycle->maxX, "%15s%15s%-30s", "NAME", " ", "COMMENT");
    if (strlen(buffer) < cbCycle->maxX)
    {
        str_repeat(buffer, " ", cbCycle->maxX - strlen(buffer) - 1);
    }
    print_title(buffer);
}

void print_shrt_title()
{
    char buffer[cbCycle->maxX];
    snprintf(buffer, cbCycle->maxX, "%15s%25s%25s", "SHORT", "COMMAND", "COMMENT");
    if (strlen(buffer) < cbCycle->maxX)
    {
        str_repeat(buffer, " ", cbCycle->maxX - strlen(buffer) - 1);
    }
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
            generate_group_str(buffer, cbCycle->maxX, group->key, group->value);
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

void print_help_label(void)
{
    int cursorX = getcurx(stdscr);
    int cursorY = getcury(stdscr);

    char screenLine[1024];
    snprintf(screenLine, getmaxx(stdscr), "%s", "help command");
    mvprintw(2, 0, "%s", screenLine);
    clrtoeol();
    refresh();
    move(cursorY, cursorX);
    cursorX = getcurx(stdscr);
    cursorY = getcury(stdscr);
    refresh();
}

void signal_callback_handler_ctrl_c(int signNum)
{
    if (signNum == SIGINT)
    {
        exit_curses();
        exit(signNum);
    }
}
