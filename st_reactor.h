#include <sys/poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

typedef void (*handler_t)(preactor_t, int, void*);

typedef struct {
    void (*handler)(preactor_t, int, void*);
    void* arg;
} handler_t, *phandler_t;

typedef struct event_handler_t {
    int fd;
    void* arg;
    handler_t handler;
} event_handler_t;

typedef struct reactor_t {
    struct pollfd* fds;
    event_handler_t** handlers;
    int counter;
    int isActive;
    int size;
    int currentlyListen;
    pthread_t thread;
} reactor_t, *preactor_t;

preactor_t createReactor(int size,int listenerFd);
void stopReactor(preactor_t reactor);
void startReactor(preactor_t reactor);
void* reactorRun(void* arg);
void addFd(preactor_t reactor, int fd, handler_t handler);
void waitFor(preactor_t reactor);
void deleteReactor(preactor_t reactor);
void deleteFd(preactor_t reactor, int fd);
int findFd(preactor_t reactor, int fd);
