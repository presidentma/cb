/* main.c Copyright presidentma */
#include "cb.h"

#define GETOPT_NO_ARGUMENT 0
#define GETOPT_REQUIRED_ARGUMENT 1
#define GETOPT_OPTIONAL_ARGUMENT 2
static const struct option longOptions[] = {
    {"group", GETOPT_REQUIRED_ARGUMENT, NULL, 'g'},
    {"list", GETOPT_NO_ARGUMENT, NULL, 'l'},
    {"add", GETOPT_REQUIRED_ARGUMENT, NULL, 'a'},
    {"short", GETOPT_REQUIRED_ARGUMENT, NULL, 's'}};
static cycle *cbCycle;
HashSet *hashMap;
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    init_cb(argc, argv);
    return 0;
}
/* 解析参数 */
void get_options(int argc, char *argv[])
{
    char *optstring = "g:a:s:l";

    int optionIndex = 0;
    int opt = getopt_long(argc, argv, optstring, longOptions, &optionIndex);
    /* opt optarg optind */
    if (opt != -1)
    {
        switch (opt)
        {
        case 'l':
            cbCycle->list = 1;
            break;
        case 'g':
            cbCycle->group = 1;
            break;
        case 'a':
            cbCycle->add = 1;
            break;
        case 's':
            cbCycle->shrt = 1;
            break;
        default:
            break;
        }
    }
}

int remalloc_size(int oldSize, int min)
{
    return oldSize << 1 > min ? oldSize << 1 : remalloc_size(oldSize << 1, min);
}

int init_cb(int argc, char *argv[])
{
    init_cycle();
    get_options(argc, argv);
    open_curses_terminal();
    return 0;
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
        case KEY_LEFT:
            break;
        case KEY_RIGHT:
            break;
        default:
            if (operateCode >= K_NUM_ZERO && strlen(cbCycle->cmdline) < (cbCycle->maxX - cbCycle->promptX - 1))
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
