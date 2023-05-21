#include <sys/poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

typedef struct reactor_t *preactor_t;

typedef void (*handler_func_t)(preactor_t, int, void*);

typedef struct {
    handler_func_t handler;
    void* arg;
} handler_t, *phandler_t;

typedef struct event_handler_t {
    int fd;
    void* arg;
    handler_func_t handler;
} event_handler_t;

typedef struct reactor_t {
    struct pollfd* fds;
    event_handler_t** handlers;
    int counter;
    int isActive;
    int size;
    int currentlyListen;
    pthread_t thread;
} reactor_t; // preactor_t is already defined, no need to redefine it here

preactor_t createReactor(int size,int listenerFd);
void stopReactor(preactor_t reactor);
void startReactor(preactor_t reactor);
void *runReactor(void *arg);
void addFd(preactor_t reactor, int fd, handler_func_t handler);
void waitFor(preactor_t reactor);
void deleteReactor(preactor_t reactor);
void deleteFd(preactor_t reactor, int fd);
int findFd(preactor_t reactor, int fd);
