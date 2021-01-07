#include <getopt.h>
#include "cb_store.h"
#include "hashset.h"
#include "cb_curses.h"
#include <signal.h>

/* command str init length */
#define COMMAND_STR_LEN_DEFAULT 1<<3
typedef struct command_s command_t;
typedef unsigned char u_char;
struct command_s{
    u_char *str;  /* command str */
    u_char *last; /* command str last pos */
    int len;    /* command str length */
    command_t *prev;    /* prev command */
    command_t *next;    /* next command */
};

/* cb cycle */

typedef struct{
    u_char   *rootComment;
    u_int   groupCount;
    u_int add:1;        /* add option */
    u_int group:1;      /* group option */
    u_int shrt:1;       /* short command option */
    u_int list:1;       /* show list option */
    u_int promptY;      /* curses Y position */
    u_int promptX;      /* curses X position */
    u_int maxY;      /* curses max Y position */
    u_int maxX;      /* curses max X position */
    u_int pageY;      /* current print Y position */
    u_int pageX;      /* current print X position */
    HashSet  *hashSet;
}cycle;

int remalloc_size(int oldSize,int min);
void get_options(int argc,char* argv[]);
int init_cb();
int create_hashset(cycle* cbCycle,json object,HashSet* hashMap);
int generate_child(json group,struct HashSetNodeP **p);
int init_cycle(cycle* cycle);
void open_curses_terminal();
void signal_callback_handler_ctrl_c(int signNum);


/*  */
void print_help_label(void);
void exit_curses();
void print_prompt();


