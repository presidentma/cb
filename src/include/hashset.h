#include "cb_utils.h"

#define HASH_MAP_SIZE 2047
#define HASH_BASE 8564

struct HashSetNodeP {
    u_char* key;
    u_char* value;
    struct HashSetNodeP* next;
    struct HashSetNodeC* child; 
};

struct HashSetNodeC {
    u_char* shrt;
    u_char* comment;
    u_char* command;
    struct HashSetNodeC* next;
};

typedef struct {
    struct HashSetNodeP* lists[HASH_MAP_SIZE];
    int currentSize;
} HashSet;

unsigned int hash_code(const char *str);
void hashset_init(HashSet *hs);
void *hash_get_group(const HashSet *hs,const char *key);

int hasGroup(const HashSet *hs,const char *key);
int hashset_push(HashSet *hs, char *key,void* value);
int hashset_modify(const HashSet *hs,const char *key,void* value);
char** hashset_keys(const HashSet *hs);
int hashset_size(const HashSet *hs);
void hashset_destroy(HashSet *hs);