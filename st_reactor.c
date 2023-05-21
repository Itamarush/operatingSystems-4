#include "st_reactor.h"

void addFd(preactor_t reactor, int fd, handler_func_t handler)
{
    if (reactor->counter < reactor->size)
    {
        reactor->handlers[reactor->counter] = malloc(sizeof(event_handler_t));
        reactor->handlers[reactor->counter]->handler = handler;
        reactor->handlers[reactor->counter]->arg = handler;
        reactor->fds[reactor->counter].fd = fd;
        reactor->fds[reactor->counter].events = POLLIN;
        reactor->counter++;
    }
    else
    {
        reactor->handlers = (event_handler_t **)realloc(reactor->handlers, reactor->size * 2 * sizeof(event_handler_t *));
        reactor->fds = (struct pollfd *)realloc(reactor->fds, reactor->size * 2 * sizeof(struct pollfd));
        reactor->size *= 2;
        addFd(reactor, fd, handler);
    }
}

preactor_t createReactor(int size, int listenerFd) 
{
    preactor_t reactor = malloc(sizeof(reactor_t));
    reactor->fds = malloc(size * sizeof(struct pollfd));
    reactor->handlers = malloc(size * sizeof(phandler_t));
    reactor->counter = 0;
    reactor->size = size;
    reactor->isActive = 0;
    reactor->currentlyListen = listenerFd;
    return reactor;
}

void stopReactor(preactor_t reactor)
{
    if (reactor == NULL)
	{
		printf("stopReactor() failed: \n");
		return;
	}

    else if (reactor->isActive == 0)
	{
		printf("The reactor is already not running. \n");
		return;
	}

    reactor->isActive = 0;
    waitFor(reactor);
    return;
}

void waitFor(preactor_t reactor)
{
    if (reactor != NULL && !(reactor->isActive))
    {
        pthread_join(reactor->thread, NULL);
    }
    
}

void startReactor(preactor_t reactor)
{
    if (!(reactor->isActive) && (reactor))
    {
        reactor->isActive = 1;
        pthread_create(&reactor->thread, NULL, runReactor, reactor);
    }
}

void processEvent(preactor_t reactor, int i) {
    if (reactor->fds[i].revents & POLLIN) {
        reactor->handlers[i]->handler(reactor, reactor->fds[i].fd, reactor->handlers[i]->arg);
    }
}

void processEvents(preactor_t reactor, int numEvents) {
    int currCount = reactor->counter;
    for (int i = 0; i < currCount; i++) {
        processEvent(reactor, i);
    }
}

void *runReactor(void *arg)
{
    preactor_t reactor = (preactor_t)arg;

    while (reactor->isActive)
    {
        int numEvents = poll(reactor->fds, reactor->counter, -1);
        if (numEvents > 0) {
            processEvents(reactor, numEvents);
        }
    }

    return NULL;
}

void deleteFd(preactor_t reactor, int fd)
{
    int index = findFd(reactor, fd);
    if (index != -1)
    {
        free(reactor->handlers[index]);
        for (int i = index; i < reactor->counter - 1; i++)
        {
            reactor->handlers[i] = reactor->handlers[i + 1];
            reactor->fds[i] = reactor->fds[i + 1];
        }
        reactor->counter--;
    }
}

void deleteReactor(preactor_t reactor)
{
    if (reactor != NULL)
    {
        if (reactor->isActive)
        {
            stopReactor(reactor);
        }
        if (reactor->handlers != NULL)
        {
            for (int i = 0; i < reactor->counter; i++)
            {
                free(reactor->handlers[i]);
            }
            free(reactor->handlers);
        }
        if (reactor->fds != NULL)
        {
            free(reactor->fds);
        }
        free(reactor);
    }
}

int findFd(preactor_t reactor, int fd)
{
    for (int i = 0; i < reactor->counter; i++)
    {
        if (reactor->fds[i].fd == fd)
        {
            return i;
        }
    }
    return -1;
}