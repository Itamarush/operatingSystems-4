#include "server_reactor.h"

pReactor p_reactor;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int get_listener_socket(void)
{
    int listener;
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai);

    if (p == NULL)
    {
        return -1;
    }

    if (listen(listener, 500 == -1))
    {
        return -1;
    }

    return listener;
}

void connectionHandler(pReactor reactor,int fd, void *arg)
{
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    int newfd;

    addrlen = sizeof remoteaddr;
    newfd = accept(reactor->currentlyListen, (struct sockaddr *)&remoteaddr, &addrlen);
    if (newfd == -1)
    {
        perror("accept");
    }
    else
    {
        printf("server: new connection from %s on "
               "socket %d\n",
               inet_ntop(remoteaddr.ss_family,
                         get_in_addr((struct sockaddr *)&remoteaddr),
                         remoteIP, INET6_ADDRSTRLEN),
               newfd);
        handlerStruct cHandler;
        cHandler.arg = NULL;
        cHandler.handler = &handleClient;
        addFd(reactor, newfd, cHandler);
    }
}

void sendToOtherClients(pReactor reactor, int sender_fd, char *message, int length)
{
    printf("Attempting to send message to all clients\n");
    for (int i = 0; i < reactor->count; i++)
    {
        int target_fd = reactor->fds[i].fd;
        if (target_fd != sender_fd && target_fd != reactor->currentlyListen)
        {
            if (send(target_fd, message, length, 0) == -1)
            {
                perror("Failed to send message");
            }
        }
    }
    printf("Message successfully sent to all clients\n");
}

void handleClient(pReactor reactor, int clientSocket, void *arg)
{
    char messageBuffer[BUFF_SIZE] = {0};
    int recvBytes;

    recvBytes = recv(clientSocket, messageBuffer, BUFF_SIZE, 0);
    if (recvBytes <= 0)
    {
        if (recvBytes == 0)
        {
            printf("Server log: client at socket %d disconnected\n", clientSocket);
        }
        else
        {
            perror("Failed to receive message");
        }
        close(clientSocket);
        deleteFd(reactor, clientSocket);
    }
    else
    {
        printf("Server log: client at socket %d sent message: %s", clientSocket, messageBuffer);
        sendToOtherClients(reactor, clientSocket, messageBuffer, recvBytes);
    }
}


int main(void)
{
    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;


    char remoteIP[INET6_ADDRSTRLEN];

    printf("listener: waiting for connections...\n");
    listener = get_listener_socket();

    if (listener == -1)
    {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    p_reactor = createReactor(10, listener);

    handlerStruct cHandler;
    cHandler.arg = NULL;
    cHandler.handler = &connectionHandler;
    addFd(p_reactor, listener, cHandler);

    startReactor(p_reactor);
    printf("Reactor is running to stop the programm use CTRL+C\n");


    while (p_reactor->isRunning)
    {
        sleep(1);
    }

    return 0;
}