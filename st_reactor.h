#include <sys/poll.h>
#include <pthread.h>
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


typedef struct reactorStruct* pReactor;
typedef struct {
    void (*handler)(pReactor, int, void*);
    void* arg;
} handlerStruct, *pHandler;

typedef struct reactorStruct
{
    pHandler* handlers;
    struct pollfd* fds;
    int count; 
    int size;
    int isRunning;
    int currentlyListen;
    pthread_t thread;
}reactorStruct, *pReactor;

pReactor createReactor(int, int);
void stopReactor(pReactor);
void startReactor(pReactor);
void* runReactor(void*);
void addFd(pReactor, int, handlerStruct);
void waitFor(pReactor);
void deleteReactor(pReactor);
void deleteFd(pReactor, int);
int findFd(pReactor, int);
void handleRealloc(pReactor);
void addHandlerAndFd(pReactor, int, handlerStruct);
void shiftElementsDown(pReactor, int);
void freeFds(pReactor);
void freeHandlers(pReactor);

