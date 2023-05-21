#include "st_reactor.h"
#include <stdlib.h>

pReactor createReactor(int size, int listen)
{
    pReactor reactor = (pReactor)malloc(sizeof(reactorStruct));
    reactor->handlers = (pHandler *)malloc(size * sizeof(pHandler));
    reactor->fds = (struct pollfd *)malloc(size * sizeof(struct pollfd));
    reactor->count = 0;
    reactor->size = size;
    reactor->isRunning = 0;
    reactor->currentlyListen = listen;
    return reactor;
}

void stopReactor(pReactor reactor)
{
    if (reactor != NULL)
    {
        reactor->isRunning = 0;
        waitFor(reactor);
    }
}

void startReactor(pReactor reactor)
{
    if (!reactor->isRunning)
    {
        reactor->isRunning = 1;
        pthread_create(&reactor->thread, NULL, runReactor, reactor);
    }
}

void *runReactor(void *arg)
{
    pReactor reactor = (pReactor)arg;
    pHandler handler;
    struct pollfd *fd;

    while (reactor->isRunning)
    {
        if (poll(reactor->fds, reactor->count, -1) <= 0) continue;

        for (int i = 0; i < reactor->count; i++)
        {
            fd = &(reactor->fds[i]);
            handler = reactor->handlers[i];

            if (fd->revents & POLLIN)
            {
                handler->handler(reactor, fd->fd, handler->arg);
            }
        }
    }
}


void handleRealloc(pReactor reactor)
{
    reactor->handlers = (pHandler *)realloc(reactor->handlers, reactor->size * 2 * sizeof(pHandler));
    reactor->fds = (struct pollfd *)realloc(reactor->fds, reactor->size * 2 * sizeof(struct pollfd));
    reactor->size *= 2;
}

void addHandlerAndFd(pReactor reactor, int fd, handlerStruct handler)
{
    int count = reactor->count;
    reactor->handlers[count] = (pHandler)malloc(sizeof(handlerStruct));
    reactor->handlers[count]->handler = handler.handler;
    reactor->handlers[count]->arg = handler.arg;
    reactor->fds[count].fd = fd;
    reactor->fds[count].events = POLLIN;
    reactor->count++;
}

void addFd(pReactor reactor, int fd, handlerStruct handler)
{
    if (reactor->count >= reactor->size)
    {
        handleRealloc(reactor);
    }
    
    addHandlerAndFd(reactor, fd, handler);
}


void waitFor(pReactor reactor)
{
    if(reactor && !reactor->isRunning)
        pthread_join(reactor->thread, NULL);
}

int findFd(pReactor reactor, int fd)
{
    for (int i = 0; i < reactor->count; i++)
    {
        if (reactor->fds[i].fd == fd)
        {
            return i;
        }
    }
    return -1;
}

void shiftElementsDown(pReactor reactor, int startIndex) 
{
    for (int i = startIndex; i < reactor->count - 1; i++)
    {
        reactor->handlers[i] = reactor->handlers[i + 1];
        reactor->fds[i] = reactor->fds[i + 1];
    }
    reactor->count--;
}

void deleteFd(pReactor reactor, int fd)
{
    int index = findFd(reactor, fd);
    if (index != -1)
    {
        free(reactor->handlers[index]);
        shiftElementsDown(reactor, index);
    }
}

void freeHandlers(pReactor reactor) 
{
    if (reactor->handlers != NULL)
    {
        for (int i = 0; i < reactor->count; i++)
        {
            free(reactor->handlers[i]);
        }
        free(reactor->handlers);
    }
}

void freeFds(pReactor reactor)
{
    if (reactor->fds != NULL)
    {
        free(reactor->fds);
    }
}

void deleteReactor(pReactor reactor)
{
    if (reactor == NULL) return;

    if (reactor->isRunning)
    {
        stopReactor(reactor);
    }

    freeHandlers(reactor);
    freeFds(reactor);
    
    free(reactor);
}