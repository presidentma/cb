/* main.c Copyright presidentma */
#include "cb.h"

#define GETOPT_NO_ARGUMENT           0
#define GETOPT_REQUIRED_ARGUMENT     1
#define GETOPT_OPTIONAL_ARGUMENT     2
static const struct option longOptions[]={
    {"group",GETOPT_REQUIRED_ARGUMENT,NULL,'g'},
    {"list",GETOPT_NO_ARGUMENT,NULL,'l'},
    {"add",GETOPT_REQUIRED_ARGUMENT,NULL,'a'},
    {"short",GETOPT_REQUIRED_ARGUMENT,NULL,'s'}
};
static cycle *cbCycle;
int main(int argc,char* argv[])
{
    setlocale(LC_ALL, "");
    init_cb();
    get_options(argc,argv);
    open_curses_terminal();
    return 0;
}

void get_options(int argc,char* argv[])
{
    char *optstring = "g:a:s:l";
    
    int optionIndex = 0;
    int opt=getopt_long(argc,argv,optstring,longOptions,&optionIndex);
    /* opt optarg optind */
    if(opt!=-1){
        switch (opt)
        {
            case 'l':
                cbCycle->list=1;
                break;
            case 'g':
                cbCycle->group=1;
                break;
            case 'a':
                cbCycle->add=1;
                break;
            case 's':
                cbCycle->shrt=1;
                break;
            default:
                break;
        }
    }
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
    cbCycle = (cycle*)cb_malloc(sizeof(cycle));
    return init_cycle(cbCycle);
}

int init_cycle(cycle* cbCycle)
{
    json object;
    if(parse_init(&object)){
        return CB_FAIL;
    }
    cbCycle->list = 0;
    cbCycle->group = 0;
    cbCycle->add = 0;
    cbCycle->shrt = 0;
    cbCycle->groupCount = 0;
    cbCycle->rootComment = "";
    cbCycle->promptY = 0;
    cbCycle->promptX = 0;
    cbCycle->maxY = 0;
    cbCycle->maxX = 0;
    cbCycle->pageY = 0;
    cbCycle->pageX = 0;
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
    u_int selectionCount = 2;
    u_int height = getmaxy(stdscr);
    u_int width = getmaxx(stdscr);
    /*  */
    bool done = false;
    bool promptInt = true;
    u_int operateCode = 0;
    /*  */
    while(!done){
        if(promptInt){
            attron(A_BOLD);
            print_prompt();
            attroff(A_BOLD);
            promptInt=false;
            continue;
        }else{
            operateCode = wgetch(stdscr);
            mvprintw(cbCycle->promptY, cbCycle->promptX,"%c", (char)operateCode);
            cbCycle->promptX++;
            refresh();
        }
    }
        
        /* exit */
        exit_curses();
}

void print_prompt()
{
    char *user = getenv(ENV_VAR_USER);
    char *hostname=(char *)malloc(HOSTNAME_BUFFER_SIZE);
    user=user?user:"me";
    get_hostname(HOSTNAME_BUFFER_SIZE, hostname);
    mvprintw(cbCycle->promptY, cbCycle->promptX, "%s@%s$ ", user, hostname);
    cbCycle->promptX+=(strlen(user)+strlen(hostname)+3);
    cbCycle->pageY++;
    free(hostname);
    refresh();
}

void print_help_label(void)
{

    int cursorX=getcurx(stdscr);
    int cursorY=getcury(stdscr);

    char screenLine[1024];
    snprintf(screenLine, getmaxx(stdscr), "%s", "help command");
    mvprintw(2, 0, "%s", screenLine); clrtoeol();
    
    refresh();

    move(cursorY, cursorX);
    cursorX=getcurx(stdscr);
    cursorY=getcury(stdscr);
    printw("%d,%d",cursorY, cursorX);
    refresh();
}

void signal_callback_handler_ctrl_c(int signNum)
{
    if(signNum==SIGINT) {
        exit_curses();
        exit(signNum);
    }
}

void exit_curses()
{
        clear();
        refresh();
        doupdate();
        endwin();
}