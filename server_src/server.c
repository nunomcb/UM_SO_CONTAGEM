#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "estrutura_contagem.h"
#include "hash_table.h"

/*
 * Some values are represented by 2 bytes. This macro is used to read them
 */
#define GET_VALUE(str) (255 & str[0]) * 256 + (255 & str[1])

/*
 * Table that associates a name to a child
 */
HashTable childsTable;

/*
 * Stores counters...
 */
CountStructure counters;

/*
 * Structure used to store information about a child.
 * pd is pipe descriptor used to communicate with the child
 * pid is the child's pid
 */
typedef struct ChildInfo_s {
    int pd;
    int pid;
} *ChildInfo;

/*
 * Structure used to hold information about a request.
 */
typedef struct {
    int function;
    int n_args;
    char **args;
} Request;

/*
 * Utility structure for strings
 */
typedef struct Str_s {
   char* str;
   int len;
   int max;
} Str;


/*
 * Function used as an argument for the function htFind, used to find a given pid in the hash table 
 */
int comparePID(void* pid1, void* pid2) {
    int* pid = (int*)pid1;
    ChildInfo c = (ChildInfo)pid2;

    if (*pid == c->pid) return 0;
    else return -1;
}

/*
 * Function used as an argument for the function htDestroy, used to free all the memory allocated and close pipes, process that will result in the death of every child
 */
void destroyChildInfo(void *info) {
    ChildInfo i = (ChildInfo)info;

    close(i->pd);
    free(i);
}

/*
 * Parses a string into a Request
 */
Request parse(char *buf) {
    Request d;
    int i;
   
    d.function = (int)buf[0];
    d.n_args = (int)buf[1];
    d.args = (char**)malloc(sizeof(char*) * d.n_args);
    
    d.args[0] = strtok(buf + 2, ":");

    for (i = 1; i < d.n_args; i++) {
        d.args[i] = strtok(NULL, ":");
    }

    return d;
}

/*
 * Returns the first argument of a request. Used to search for the child process that should handle the request
 */ 
int firstArg(char *buf, char** ret) {
    char* str;
    int size;
    int i;

    size = 1024;
    i = 0;
    str = (char*)malloc(sizeof(char) * (size + 1));
    buf += 2;

    while (buf[i] != ':') {
        if (i == size) {
            size *= 2;
            str = (char*)realloc(str, sizeof(char) * (size + 1));
        }
        str[i] = buf[i];
        i++;
    }

    str[i] = '\0';

    *ret = str;

    return i;
}

/*
 * Increments a given counter
 */
void incrementar(int n_args, char *args[]) {
    int val = atoi(args[n_args - 1]);
    args[n_args - 1] = NULL;

    csInsert(counters, args, val);
}

/*
 * Recursive function to print a CounterInfo variable to a file.
 * Returns -1 if nothing is written.
 */
int __printToFile(int fd, CounterInfo info, Str prefix) {
    int i;
    int len;
    int ret;
    int total;
    int n_subs;
    char *str;

    ret = -1;
    total = ciGetTotal(info);
    str = ciGetStr(info);
    n_subs = ciGetNumberOfSubs(info);

    if (ciGetNumberOfSubs(info) == -1) {
        /*
         * If it's on the requested level, print
         */
        sprintf(prefix.str + prefix.len, "%s:%d\n", str, total);
        write(fd, prefix.str, strlen(prefix.str));

        ret = 0;
    }
    else {
        /*
         * Append name to the prefix and call this function with the sub-counters
         */
        len = prefix.len;
        if (len + (int)strlen(str) + 1 >= prefix.max) {
            /*
             * String memory reallocation
             */
            prefix.max *= 2;
            prefix.str = realloc(prefix.str, sizeof(char) * (prefix.max + 1));
        }
        sprintf(prefix.str + len, "%s:", str);
        prefix.len += strlen(str) + 1;
        len = prefix.len;

        for (i = 0; i < n_subs; i++) {
            prefix.str[len] = '\0';
            /*
             * ret is initialized as -1 and it becomes 0 when something is written in the file, 
             * therefore, if a recursive call writes in the file, this will also return 0
             */
            ret &= __printToFile(fd, ciGetSub(info, i), prefix);
        }
    }

    return ret;
}

/*
 * Opens the requested file and prints the info parameter with the given prefix.
 * Returns -1 if the file couldn't be opened or if there isn't anything to write in it
 */
int printToFile(char* filename, CounterInfo info) {
    Str prefix;
    int fd;
    int ret;

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    if (fd == -1) {
        return -1;
    }

    prefix.str = malloc(sizeof(char) * (1024 + 1));
    prefix.max = 1024;
    prefix.len= 0;

    ret = __printToFile(fd, info, prefix);
    free(prefix.str);

    close(fd);

    return ret;
}

/*
 * Prints counters to a file. 
 */
void agregar(int n_args, char *args[]) {
    CounterInfo ci;
    char *path = args[n_args - 2];
    int level = atoi(args[n_args - 3]);
    unsigned int pid = GET_VALUE(args[n_args - 1]);

    args[n_args - 1] = args[n_args - 2] = args[n_args - 3] = NULL;

    ci = csCountByLevel(counters, args, level);
    
    if (!ci) {
        /*
         * if the prefix doesn't exist, send SIGUSR2 to the client
         */
        kill(pid, SIGUSR2);
#ifdef DEBUG
        printf("SENDING SIGUSR2 TO PID %d\n", pid);
#endif
    }
    else{ 
        if (printToFile(path, ci)) {
            /*
             * if something goes wrong
             */
            kill(pid, SIGUSR2);
#ifdef DEBUG
            printf("SENDING SIGUSR2 TO PID %d\n", pid);
#endif
        }
        else {
            /*
             * everythin'z goooood
             */
            kill(pid, SIGUSR1);
#ifdef DEBUG
            printf("SENDING SIGUSR1 TO PID %d\n", pid);
#endif
        }

        ciDestroy(ci);
    }

    return;
}

/*
 * Read increments from a file and resume to a previous state
 */
void resume(int fd) {
    char buf[2048];
    int n;
    Request d;

    while ((n = read(fd, buf, 2))) {
        n = GET_VALUE(buf);
        n = read(fd, buf, n);
        buf[n] = '\0';

        d = parse(buf);
        incrementar(d.n_args, d.args);
        free(d.args);
    }

    return;
}

/*
 * Child's loop
 */
void mainLoopChild() {
    Request d;
    char buf[2048];
    char *filename;
    int logfile;
    int n;

    counters = csNew();

    /*
     * Waits for the parent to write the counter name
     */
    n = read(0, buf, 2);
    n = read(0, buf, GET_VALUE(buf));
    buf[n]= '\0';

    filename = (char*)malloc(sizeof(char) * (n + 10));
    strcpy(filename, "logs/");
    strcpy(filename + 5, buf);
    strcpy(filename + n + 5, ".log");

    do {
        logfile = open(filename, O_RDWR | O_APPEND | O_CREAT, 0666);
    } while(logfile == -1);

    resume(logfile);
    free(filename);

    while ((n = read(0, buf, 2)) > 0) {
        n = GET_VALUE(buf);
        n = read(0, buf + 2, n);
        buf[n + 2] = '\0';
        
#ifdef DEBUG
        printf("CHILD WITH PID %d RECEIVED REQUEST:\n\t", getpid());
        if (buf[2] == 2)
            printf("AGREGAR %.*s\n", n - 6, buf + 4);
        else
            printf("INCREMENTAR %.*s\n", n - 2, buf + 4);
#endif  

        if (buf[2] == 1) {
            /*
             * Append the increment to the log file
             */
            write(logfile, buf, n + 2);
        }

        d = parse(buf + 2);

        if (d.function == 1) {
            incrementar(d.n_args, d.args);
        }
        else if (d.function == 2) {
            agregar(d.n_args, d.args);
        }

        free(d.args);
    }

    close(logfile);
    csDestroy(counters);
    _exit(0);
}

/*
 * Creates a new child. Returns a ChildInfo var with the child's pid and pipe descriptor
 */
ChildInfo newChild(char* str) {
    int pd[2];
    int pid;
    int len;
    char clen[2];
    ChildInfo cinfo;

    cinfo = (ChildInfo)malloc(sizeof(struct ChildInfo_s));
    pipe(pd);
    cinfo->pd = pd[1];
    htInsert(childsTable, str, cinfo);

    if ((pid = fork())) {
        cinfo->pid = pid;
        close(pd[0]);
        len = strlen(str);
        clen[0] = len / 256;
        clen[1] = len % 256;
        write(pd[1], clen, 2);
        write(pd[1], str, len);
    }
    else {
        free(str);
        /*
         * Destroy the childsTable... It isn't needed in the child process
         */
        htDestroy(childsTable, &destroyChildInfo);
        /*
         * Ignore SIGINTs... The parent will replace this process, so why bother?
         */
        signal(SIGINT, SIG_IGN);
        close(pd[1]);
        dup2(pd[0], 0);
        close(pd[0]);
        mainLoopChild();
    }

    return cinfo;
}

/*
 * Initializes the known counters
 */
void initializeChilds(int fd) {
    int n;
    Str buffer;
    char* arg;
    char* name;

    buffer.len = 0;
    buffer.max = 2048;
    buffer.str = (char*)malloc(sizeof(char) * (2048 + 1));

    while ((n = read(fd, buffer.str, 2048)) > 0) {
        buffer.len += n;
        if (buffer.len == buffer.max) {
            buffer.max += 2048;
            buffer.str = realloc(buffer.str, sizeof(char) * (buffer.max + 1));
        }
    }
    
    buffer.str[buffer.len] = '\0';

    arg = strtok(buffer.str, "\n");

    while (arg) {
#ifdef DEBUG
        printf("INITIALIZING %s!\n", arg);
#endif
        name = malloc(sizeof(char) * (strlen(arg) + 1));
        strcpy(name, arg);
        newChild(name);
        free(name);
        arg = strtok(NULL, "\n");
    }

    free(buffer.str);
}

/*
 * Parent's loop
 */
void mainLoopParent() {
    ChildInfo cinfo;
    char buf[2048];
    char* fst;
    int size;
    int n;
    int pid;
    int counters_list;
    struct stat st = {0};

    if (stat("logs", &st) == -1) {
        /*
         * If the logs directory doesn't exist... create one!
         */
        mkdir("logs", 0777);
    }

    counters_list = open("logs/counters", O_RDWR | O_CREAT, 0666);
    
    if (counters_list == -1) {
        /*
         * I can't really work without my files... Good bye cruel world
         */
        write(2, "CAN'T OPEN logs/counters! EXITING\n", 34);
        raise(SIGINT);
        return;
    }

    initializeChilds(counters_list);

    for(;;) {
        while (!(n = read(0, buf, 2)));
        n = GET_VALUE(buf);
        read(0, buf + 2, n);
        
        size = firstArg(buf + 2, &fst);

#ifdef DEBUG
        printf("RECEIVED REQUEST:\n\t");
        if (buf[2] == 2)
            printf("AGREGAR %.*s\n", n - 6, buf + 4);
        else
            printf("INCREMENTAR %.*s\n", n - 2, buf + 4);
#endif  

        if (!(cinfo = (ChildInfo)htGet(childsTable, fst))) {
            /*
             * If there isn't any child process to handle the given counter
             */
            if (buf[2] == 2) {
                /*
                 * Well... It isn't really worth creating a child to aggregate something that doesn't exist
                 */
                pid = GET_VALUE((buf + n));
                kill(pid, SIGUSR2);
#ifdef DEBUG
                printf("%s NOT FOUND. SENDING SIGUSR2 TO PID %d\n", fst, pid);
#endif
                free(fst);
                continue;
            }
#ifdef DEBUG
            printf("%s NOT FOUND. FORKING\n", fst);
#endif
            /*
             * Create a new child to handle the new counter
             */
            fst[size] = '\n';
            write(counters_list, fst, size + 1);
            fst[size] = '\0';
            cinfo = newChild(fst);
        }

#ifdef DEBUG
        printf("SENDING REQUEST TO PID %d\n", cinfo->pid);
#endif

        free(fst);

        write(cinfo->pd, buf, n + 2);
    }

    htDestroy(childsTable, &destroyChildInfo);
    close(counters_list);
}

void destroyStructures() {
    /*
     * Since killing every child will generate a lot of useless SIGCHLDs, we should just ignore them
     */
    signal(SIGCHLD, SIG_IGN);
    htDestroy(childsTable, &destroyChildInfo);
    unlink("count_fifo");
    _exit(0);
}

void sig_handler(int sig) {
    int pid;
    int status;
    char* str;
    ChildInfo deadChild;

    switch (sig) {
        case (SIGCHLD):
            pid = waitpid(-1, &status, 0);
            
            if (WEXITSTATUS(status)) {
                /*
                 * Find the counter name and replace the dead child.
                 */
                str = htFind(childsTable, (void*)&pid, comparePID);
                deadChild = htGet(childsTable, str);
                close(deadChild->pd);
                htRemove(childsTable, str);

                newChild(str);
            }

            break;
        case (SIGINT):
            /*
             * FREE ALL THE ALLOCATED MEMORY! And kill the children <3
             */
            destroyStructures();
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    int fd;

    childsTable = htNew(100);
    counters = NULL;

    signal(SIGCHLD, sig_handler);
    signal(SIGINT, sig_handler);

    mkfifo("count_fifo", 0666);
    fd = open("count_fifo", O_RDONLY);
    dup2(fd, 0);
    close(fd);

    mainLoopParent();

    return 0;
}
