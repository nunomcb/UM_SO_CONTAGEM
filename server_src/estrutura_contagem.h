#ifndef ST_COUNT_H
#define ST_COUNT_H

typedef struct CounterInfo_s *CounterInfo;
typedef struct CountTree_s *CountStructure;

void                ciDestroy(CounterInfo info);
int                 ciGetNumberOfSubs(CounterInfo info);
char*               ciGetStr(CounterInfo info);
CounterInfo         ciGetSub(CounterInfo info, int index);
int                 ciGetTotal(CounterInfo info);

CounterInfo         csCountByLevel(CountStructure cs, char *prefixes[], int level);
void                csDestroy(CountStructure cs);
void                csInsert(CountStructure ct, char *nomes[], int val);
CountStructure      csNew();
CountStructure      csSubTree(CountStructure ct, char *prefixes[]);


#endif
