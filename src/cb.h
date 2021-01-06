#include "cb_store.h"
#include "hashset.h"
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
    HashSet  *hashSet;
    u_int   groupCount;
}cycle;

int remalloc_size(int oldSize,int min);
int init_cb();
int create_hashset(cycle* cbCycle,json object,HashSet* hashMap);
int generate_child(json group,struct HashSetNodeP **p);
int init_cycle(cycle* cycle);


