#include "include/hashset.h"

u_int hash_code(const char *str)
{
    u_int hash = HASH_BASE;
    for(int i=0;str[i]!='\0';i++){
        hash=33*hash+str[i];
    }
    hash ^=(hash>>16);
    return hash%HASH_MAP_SIZE;
}

void hashset_init(HashSet *hs)
{
    hs->currentSize = 0;
    memset(hs->lists,0,sizeof(struct HashSetNode*)*HASH_MAP_SIZE);
}
/* group node struct */
void *hash_get_group(const HashSet *hs,const char *key)
{
    u_int hash = hash_code(key);
    struct HashSetNodeP *p = hs->lists[hash];
    while(p != NULL && strcmp(p->key, key)!= 0) {
        p = p->next;
    }
    return (p!=0?p:NULL);
}

int hasGroup(const HashSet *hs,const char *key)
{
    return (hash_get_group(hs, key) != NULL);
}

int hashset_push(HashSet *hs,char *key,void* value)
{
    if(hasGroup(hs, key)) {
        return hashset_modify(hs, key,value);
    } else {
        int hash = hash_code(key);
        struct HashSetNodeP *p=(struct HashSetNodeP *)cb_malloc(sizeof(struct HashSetNodeP));
        if(p == NULL) {
            fprintf(stderr,"Unable to allocate hashset entry!");
            return CB_FAIL;
        }

        p->key=key;
        p->value=value;
        p->child=NULL;
        p->next=hs->lists[hash];
        hs->lists[hash]=p;
        hs->currentSize++;

        return CB_SUCCESS;
    }
}

int hashset_modify(const HashSet *hs,const char *key,void* value)
{
    struct HashSetNodeP *oldNode;
    if(!(oldNode = hash_get_group(hs,key))){
        return CB_FAIL;
    }
    oldNode->value = value;
    return CB_SUCCESS;
}

char** hashset_keys(const HashSet *hs)
{
    if(hs->currentSize) {
        char **result=malloc(sizeof(char*) * hs->currentSize);
        int i=0, j=0;
        struct HashSetNodeP *p;
        for(i=0; i<HASH_MAP_SIZE; i++) {
            p=hs->lists[i];
            if(p) {
                result[j++]=strdup(p->key);
            }
            while(p && p->next) {
                result[j++]=strdup(p->key);
                p=p->next;
            }
        }
        return result;
    } else {
        return NULL;
    }
}
int hashset_size(const HashSet *hs)
{
    return hs->currentSize;
}
void hashset_destroy(HashSet *hs)
{
    if(hs && hs->currentSize) {
        int i=0;
        struct HashSetNodeP *p, *pp;
        for(i=0; i<HASH_MAP_SIZE; i++) {
            p=hs->lists[i];
            while(p && p->next) {
                if(p->key) {
                    free(p->key);
                    if(p->value) free(p->value);
                }
                pp=p;
                p=p->next;
                free(pp);
            }
            if(p && p->key) {
                free(p->key);
                if(p->value) free(p->value);
                free(p);
            }
        }
    }
}

