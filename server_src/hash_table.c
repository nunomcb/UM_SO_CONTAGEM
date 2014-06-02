#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

typedef struct Data_s {
    char* key;
    void* value;
    struct Data_s *next;
} *Data;

struct HashTable_s {
    int size;
    Data* content;
};

static unsigned long hash(char *str) {
    /*
     * DJB2 hash algorithm
     */
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; 
    
    return hash;
}

HashTable htNew(int size) {
    HashTable ht;
    int i;

    ht = (HashTable)malloc(sizeof(struct HashTable_s));
    ht->content = (Data*)malloc(sizeof(Data) * size);
    ht->size = size;

    for (i = 0; i < size; i++) {
        ht->content[i] = NULL;
    }

    return ht;
}

void* htGet(HashTable ht, char* key) {
    unsigned long val;
    Data d;

    val = hash(key) % ht->size;
    d = ht->content[val];
    
    while (d != NULL) {
        if (strcmp(d->key, key) == 0) {
            return d->value;
        }
        else {
            d = d->next;
        }
    }

    return NULL;
}

int htContainsKey(HashTable ht, char* key) {
    unsigned long val;
    Data d;

    val = hash(key) % ht->size;
    d = ht->content[val];

    while (d != NULL) {
        if (strcmp(d->key, key) == 0) {
            return 1;
        }
        else {
            d = d->next;
        }
    }

    return 0;
}

void htInsert(HashTable ht, char* key, void* value) {
    unsigned long val;
    Data d;

    val = hash(key) % ht->size;
    d = ht->content[val];

    if (d) {
        while (d->next != NULL) {
            if (strcmp(d->key, key) == 0) {
                d->value = value;
                return;
            }

            d = d->next;
        }
        
        d->next = (Data)malloc(sizeof(struct Data_s));
        d = d->next;
    }
    else {
        ht->content[val] = (Data)malloc(sizeof(struct Data_s));
        d = ht->content[val];
    }

    d->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(d->key, key);
    d->value = value;
    d->next = NULL;

    return;
}

void htRemove(HashTable ht, char* key) {
    unsigned long val;
    Data d;

    val = hash(key) % ht->size;
    d = ht->content[val];
    
    while (d != NULL) {
        if (strcmp(d->key, key) == 0) {
            free(d->key);
            free(d);
            d = NULL;
            return;
        }
        else {
            d = d->next;
        }
    }

    return;
}

char* htFind(HashTable ht, void* term, int (*compare)(void*, void*)) {
    Data d;
    char* ret;
    int i;

    for (i = 0; i < ht->size; i++) {
        d = ht->content[i];

        while (d) {
            if (compare(term, d->value) == 0) {
                ret = malloc(sizeof(char) * (strlen(d->key) + 1));
                strcpy(ret, d->key);
                return ret;
            }
            else {
                d = d->next;
            } 
        }
    }

    return NULL;
}

void htDestroy(HashTable ht, void (*destroyValue)(void*)){
    Data temp1, temp2;
    int i;

    for (i = 0; i < ht->size; i++) {
        temp1 = ht->content[i];
        
        while (temp1) {
            temp2 = temp1->next;
            free(temp1->key);
            if (destroyValue)
                destroyValue(temp1->value);
            else
                free(temp1->value);
            free(temp1);
            temp1 = temp2;
        }
    }

    free(ht->content);
    free(ht);
}


