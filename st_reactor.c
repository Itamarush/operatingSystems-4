#include "st_reactor.h"

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
        pthread_create(&reactor->thread, NULL, reactorRun, reactor);
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
