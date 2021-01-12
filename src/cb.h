#include <getopt.h>
#include "cb_store.h"
#include "hashset.h"
#include "cb_curses.h"
#include <signal.h>

extern char* strcasestr();

/* command str init length */
#define COMMAND_STR_LEN_DEFAULT 1 << 3
typedef unsigned char u_char;
#define PRINT_TYPE_GROUP 0
#define PRINT_TYPE_COMMAND 1
#define PRINT_BUFFER_SIZE 1080
#define MAX_CMDLINE_LEN 512

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
    char cmdline[MAX_CMDLINE_LEN];           /* print item num */
    HashSet *hashSet;
};



#define K_CTRL_A 1
#define K_CTRL_E 5
#define K_CTRL_F 6
#define K_CTRL_G 7
#define K_CTRL_H 8
#define K_CTRL_L 12
#define K_CTRL_J 10
#define K_CTRL_K 11

#define K_CTRL_N 14
#define K_CTRL_P 16

#define K_CTRL_R 18
#define K_CTRL_T 20
#define K_CTRL_U 21
#define K_CTRL_W 23
#define K_CTRL_X 24
#define K_CTRL_Z 26

#define K_CTRL_SLASH 31

#define K_ESC 27
#define K_TAB 9
#define K_BACKSPACE 127
#define K_ENTER 13

#define K_NUM_ZERO 48

int remalloc_size(int oldSize, int min);
void get_options(int argc, char *argv[]);
int init_cb(int argc, char *argv[]);
int create_hashset(cycle *cbCycle, json object, HashSet *hashMap);
int generate_child(json group, struct HashSetNodeP *p);
int init_cycle();
void open_curses_terminal();
void signal_callback_handler_ctrl_c(int signNum);
void terminal_input(char *cmd);

/* common */
void print_highlighted_row(char *buffer, u_int y, u_int x);
void print_row(char *buffer, u_int y, u_int x);
char *str_repeat(char *buffer, char *c, int num);
void print_help_label(void);
void print_prompt();
void print_title(char *buffer);
void init_page(int printType);
void highline_selection(int selectionPos, int preSelectionPos);
void reprint(bool highline);
/* group */
void print_group();
void print_group_title();
void generate_group_str(char *buffer, int maxX, char *key, char *value);
/* short */
void print_shrt();
void print_shrt_title();
void generate_shrt_str(char *buffer, int maxX, char *shrt, char *command, char *comment);
