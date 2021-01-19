#include <getopt.h>
#include "cb_store.h"
#include "hashset.h"
#include "cb_curses.h"
#include <signal.h>

extern char *strcasestr();

/* command str init length */
#define COMMAND_STR_LEN_DEFAULT 1 << 3
#define PRINT_TYPE_GROUP 0
#define PRINT_TYPE_COMMAND 1
#define PRINT_BUFFER_SIZE 1080
#define MAX_CMDLINE_LEN 513
#define MAX_GROUP_NAME_LEN 17
#define MAX_SHORT_NAME_LEN 17
#define MAX_COMMENT_LEN 65
#define MAX_EXIT_PRINT_LEN 1028

typedef unsigned char u_char;
/* cb cycle */
typedef struct cycle_s cycle;
struct cycle_s
{
    u_char *rootComment;
    u_int groupCount;
    u_int add : 1;                  /* add option */
    u_int group : 1;                /* group option */
    u_int shrt : 1;                 /* short command option */
    u_int list : 1;                 /* show list option */
    u_int delete : 1;               /* delete option */
    u_int deleteC : 1;              /*confirm delete status 0 no 1 yes */
    u_int run : 2;                  /* run input comamnd */
    u_int promptY;                  /* curses Y position */
    u_int promptX;                  /* curses X position */
    u_int helpY;                    /* help Y position */
    u_int helpX;                    /* help X position */
    u_int titleY;                   /* help Y position */
    u_int titleX;                   /* help X position */
    u_int maxY;                     /* curses max Y position */
    u_int maxX;                     /* curses max X position */
    u_int pageY;                    /* current print Y position */
    u_int pageX;                    /* current print X position */
    u_int printType;                /* print group or commands */
    u_int printItems;               /* print item num */
    u_int selectCursorPosition;     /* selected group position */
    u_int selectShrtCursorPosition; /* selected short command position */
    int *printGroupIndex;           /* print item num */
    char cmdline[MAX_CMDLINE_LEN];  /* cmd */
    int addParamsNum;               /* window add form params num */
    HashSet *hashSet;
};

#define DIRECTION_UP 1
#define DIRECTION_DOWN 2
#define K_CTRL_A 1
#define K_ESC 27
#define K_BACKSPACE 127
#define K_ENTER 13

#define K_NUM_ZERO 48
#define K_SPACE 32

int remalloc_size(int oldSize, int min);
void get_options(int argc, char *argv[]);
int init_cb(int argc, char *argv[]);
int create_hashset(cycle *cbCycle, json object, HashSet *hashMap);
int generate_child(json group, struct HashSetNodeP *p);
int init_cycle();
void open_curses_terminal();
void signal_callback_handler_ctrl_c(int signNum);
void terminal_input(char *cmd);
void print_help();
void exec_option();
void destory_cycle();
int export_cb();

/* common */
void print_highlighted_row(char *buffer, u_int y, u_int x);
void print_row(char *buffer, u_int y, u_int x);
char *str_repeat(char *buffer, char *c, int num);
void print_help_label();
void print_prompt();
void print_title(char *buffer);
void init_page(int printType);
void highline_selection(int selectionPos, int preSelectionPos);
void reprint(bool highline);
void confirm_delete();
void switch_delete_option();
void reset_delete();
void reset_add();
void switch_input(int direction);
/* group */
void print_group();
void print_group_title();
void generate_group_str(char *buffer, int maxX, char *key, char *value);
int _add_group(char *groupName, char *comment);
int modify_group(char *newGroupName, char *oldGroupName, char *comment);
int delete_group(char *groupName);
void add_group_form();
/* short */
void print_shrt();
void print_shrt_title();
void generate_shrt_str(char *buffer, int maxX, char *shrt, char *command, char *comment);
int add_shrt(char *groupName, char *shrt, char *command, char *comment);
bool has_shrt(char *groupName, char *shortName);
int delete_shrt(char *groupName, char *shortName);
void add_shrt_form();
