#include <string.h>
#include <stdio.h>
#include "estrutura_contagem.h"
#include "avl.h"

typedef char* String;

typedef struct CountTree_s CountTree;
typedef struct ctAVL_s *ctAVL;

struct CountTree_s {
    int n_casos;
    ctAVL subTree;
};

struct CounterInfo_s {
    char* str;
    int total;
    int n_subs;
    struct CounterInfo_s** subs;
};

AVL_DEF_HEADER(ct, String, CountTree);
AVL_DEF(ct, String, CountTree);


static int compareString(String s1, String s2) {
    int t = strcmp(s1, s2);

    if (t > 0)
        return 1;
    else if (t < 0)
        return -1;
    else
        return 0;
}

static CountTree collision(CountTree ct1, CountTree ct2) {
    int t;
    ctEntry *entry = NULL;

    ct1.n_casos += ct2.n_casos;

    if (ct2.subTree)
        do {
            t = ctYield(ct2.subTree, entry);
    
            if (t != -1) {
                ctInsert(ct1.subTree, (*entry)->key, (*entry)->value);
            }
        } while(!t);

    return ct1;
}

CountStructure csNew() {
    CountTree *tree;

    tree = (CountStructure)malloc(sizeof(CountTree));

    tree->n_casos = 0;
    tree->subTree = ctNew(&compareString, &collision);

    return tree;
}

static void freeString(String s) {
    free(s);
}

static void __csDestroy(CountTree ct) {
    ctDestroy(ct.subTree, &freeString, &__csDestroy);
}

void csDestroy(CountStructure cs) {
    __csDestroy(*cs);
    free(cs);
}

void csInsert(CountStructure ct, char *nomes[], int val) {
    int i = 0, size;
    char *clone;
    CountTree new;
    CountTree current;
    ctAVLNode node;

    current = *ct;
    
    while (nomes[i]) {
        new.n_casos = val;
        new.subTree = NULL;
        size = strlen(nomes[i]);
        clone = (char *)malloc(sizeof(char) * (size + 1));
        strncpy(clone, nomes[i], size);
        clone[size] = '\0';
        i++;
        if (!__ctInsertFind(current.subTree, clone, new, &node)) {
            (node->value).subTree = ctNew(&compareString, &collision);
            current = node->value;
            break;
        }
        free(clone);
        current = node->value;
    }
     
    while(nomes[i]) {
        new.n_casos = val;
        new.subTree = ctNew(&compareString, &collision);
        size = strlen(nomes[i]);
        clone = (char *)malloc(sizeof(char) * (size + 1));
        strncpy(clone, nomes[i], size);
        clone[size] = '\0';
        i++;
        __ctInsertFind(current.subTree, clone, new, &node);
        current = node->value;
    }
}

static CounterInfo buildCounterInfo(CountTree ct, CounterInfo cont, int level) {
    ctEntry entry;
    CounterInfo new;
    int n_subs;
    int size;
    int test = -1;

    cont->total = ct.n_casos;
    size = 10;

    if (level == 0) {
        cont->n_subs = -1;
        cont->subs = NULL;
    }
    else if (level > 0) {
        cont->n_subs = 0;
        cont->subs = malloc(sizeof(CounterInfo) * size);

        do {
            n_subs = cont->n_subs;
            test = ctYield(ct.subTree, &entry);

            if (test >= 0) {
                new = malloc(sizeof(struct CounterInfo_s));
                if (n_subs == size) {
                    size *= 2;
                    cont->subs = realloc(cont->subs, sizeof(CounterInfo) * size);
                }
                new->str = malloc(strlen(entry->key) + 1);
                strcpy(new->str, entry->key);
                new = buildCounterInfo(entry->value, new, level - 1);
                if (new) {
                    cont->subs[n_subs] = new;
                    cont->n_subs++;
                }
                free(entry);
            }
        } while(!test);

        if (cont->n_subs == 0) {
            free(cont->str);
            free(cont);
            cont = NULL;
        }
    }

    return cont;
}

CountStructure csSubTree(CountStructure ct, char *prefixes[]) {
    CountTree current = *ct;
    CountTree *newTree;
    int i = 0;

    while (prefixes[i]) {
        if (ctFind(current.subTree, prefixes[i++], &current)) {
            return NULL;
        }
    }

    newTree = (CountStructure)malloc(sizeof(CountTree));
    newTree->n_casos = current.n_casos;
    newTree->subTree = current.subTree;

    return newTree;
}

CounterInfo csCountByLevel(CountStructure cs, char *prefixes[], int level) {
    CountStructure sub;
    CounterInfo n;
    char *prefix;
    int offset, i;

    offset = 0;
    i = 0;
    sub = csSubTree(cs, prefixes);
    n = malloc(sizeof(struct CounterInfo_s));

    
    if (sub) {

        prefix = malloc(sizeof(char) * 2048);
        while (prefixes[i]) {
            sprintf(prefix + offset, "%s:", prefixes[i]);
            offset += strlen(prefixes[i]) + 1;
            i++;
        }
        prefix[offset - 1] = '\0';
        n->str = prefix;

        n = buildCounterInfo(*sub, n, level);
        
        free(sub);
    }
    else {
        free(n);
        n = NULL;
    }

    return n;
}

CounterInfo ciGetSub(CounterInfo info, int index) {
    if (index >= info->n_subs)
        return NULL;
    else
        return info->subs[index];
}

char* ciGetStr(CounterInfo info) {
    return info->str;
}

int ciGetTotal(CounterInfo info) {
    return info->total;
}

int ciGetNumberOfSubs(CounterInfo info) {
    return info->n_subs;
}

void ciDestroy(CounterInfo info) {
    int i;

    for (i = 0; i < info->n_subs; i++) {
        ciDestroy(info->subs[i]);
    }

    free(info->str);
    free(info->subs);
    free(info);
}

/*
int main(void) {
    CountStructure cs, cs2;
    CountTree current;
    char *nomes[] = {"A", "B", "C", "D", "E", NULL};
    char *nomes5[] = {"A", "B", "C", "J", "E", NULL};
    char *nomes2[] = {"A", "B", NULL};
    char *nomes3[] = {"A", "B", "C", "D", "E", "F", "G", NULL};
    char *nomes4[] = {"B", "A", "B", "C", "D", NULL};
    int i;
    CounterInfo n;

    cs = csNew();

    csInsert(cs, nomes, 15);
    csInsert(cs, nomes2, 10);
    csInsert(cs, nomes3, 20);
    csInsert(cs, nomes4, 50);
    csInsert(cs, nomes5, 70);
    cs2 = csSubTree(cs, nomes2);
    n = csCountByLevel(cs, nomes2, 3);

    current = *cs;

    for (i = 0; i < 6; i++) {
        printf("%s %d\n", current.subTree->root->key, current.subTree->root->value.n_casos);
        current = current.subTree->root->value;
    }

    current = *cs;

    printf("%s %d\n", current.subTree->root->right->key, current.subTree->root->right->value.n_casos);

    current = current.subTree->root->right->value;

    for (i = 0; i < 4; i++) {
        printf("%s %d\n", current.subTree->root->key, current.subTree->root->value.n_casos);
        current = current.subTree->root->value;
    }

    printf("\n\n\n");
    cntPrint(n, nomes2);

    csDestroy(cs);
    free(cs2);
    return 0;
}
*/
