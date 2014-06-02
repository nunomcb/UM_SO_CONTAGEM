#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct HashTable_s *HashTable;

HashTable htNew(int size);
void htDestroy(HashTable ht, void (*destroyValue)(void*));
void* htGet(HashTable ht, char* key);
char* htFind(HashTable ht, void* searchTerm, int (*compareFunction)(void*, void*));
int htContainsKey(HashTable ht, char* key);
void htInsert(HashTable ht, char* key, void* value);
void htRemove(HashTable ht, char* key);

#endif
