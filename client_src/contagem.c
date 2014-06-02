#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

static int received_signal = 0;

/*
 * Returns the number of digits of a given number
 */
static int nDigits(unsigned n) {
    int size = 0;

    do {
        size++;
    } while(n /= 10);

    return size;
}

/*
 * Signal handler
 */
static void sig_handler(int sig) {
    switch (sig) {
        case (SIGUSR1):
            received_signal = 1;
            break;
        case (SIGUSR2):
            received_signal = 2;
            break;
    }

    return;
}

/*
 * Increments a counter by a given value
 */
int incrementar(char *nome[], unsigned valor) {
    int fd;
    int i; 
    unsigned int offset; /* Variable used to keep track of the string length */
    int valDigits;
    char str[1024];

    valDigits = nDigits(valor);
    offset = 4; /* The first 4 bytes are reserved */
    fd = open("count_fifo", O_WRONLY);
    
    for (i = 0; nome[i]; i++) {
        sprintf(str + offset, "%s:", nome[i]);
        offset += strlen(nome[i]) + 1;
    }
    
    sprintf(str + offset, "%u", valor);
    offset += valDigits;
    str[2] = 1; /* function id */
    str[3] = 1 + i; /* number of arguments */
    /* message size is divided by to chars */
    str[0] = (offset - 2) / 256; 
    str[1] = (offset - 2) % 256;
    write(fd, str, offset);
    close(fd);
    
    return 0;
}

/*
 * Write counters to a file.
 * Returns -1 if there aren't any to print
 */
int agregar(char *prefixo[], unsigned nivel, char *path) {
    sigset_t mask;
    int fd;
    int i;
    unsigned int offset; /* keep track of the string length */
    int levelDigits;
    char str[1024];

    received_signal = 0;

    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);
    levelDigits = nDigits(nivel);
    offset = 4; /* first 4 bytes are reserved */
    fd = open("count_fifo", O_WRONLY);

    for(i = 0; prefixo[i]; i++) {
        sprintf(str + offset, "%s:", prefixo[i]);
        offset += strlen(prefixo[i]) + 1;
    }

    sprintf(str + offset, "%u:", nivel);
    offset += levelDigits + 1;
    sprintf(str + offset, "%s:", path);
    offset += strlen(path) + 1;
    str[offset++] = getpid() / 256;
    str[offset++] = getpid() % 256;

#ifdef DEBUG
    printf("%u\n", 255 & str[offset - 1]);
    printf("%u\n", 255 & str[offset - 2]);

    printf("%d\n", getpid());
#endif

    str[2] = 2; /* function id */
    str[3] = 3 + i; /* number of arguments */
    /* message size */
    str[0] = (offset - 2) / 256;
    str[1] = (offset - 2) % 256;
    write(fd, str, offset);

    close(fd);

    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    /* wait for SIGUSR1 or SIGUSR2 */
    sigsuspend(&mask);

    if (received_signal == 1)
        return 0;
    else
        return 1;    
}
