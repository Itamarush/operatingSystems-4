#include "reactorServer.h"

int main(void)
{
    preactor_t pReactor;
    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    printf("listener: waiting for connections...\n");
    listener = setup_server_socket();

    if (listener == -1)
    {
        printf("error getting listening socket\n");
        exit(1);
    }

    else if (listener > 0)
    {
        printf("listening...");
    }

    pReactor = createReactor(3, listener);

    handler_t clientHandler;
    clientHandler.arg = NULL;
    clientHandler.handler = &handle_client;
    addFd(pReactor, listener, clientHandler.handler);
    startReactor(pReactor);
    printf("Reactor is running to stop the programm use CTRL+C\n");

    while (pReactor->isActive)
    {
        sleep(1); // zzzzzzzz
    }

    return 0;
}

int setup_server_socket(void) {
    struct addrinfo hints, *addr_info_list, *addr_info;
    int server_socket;
    int reuse_addr_flag = 1;

    // Fill in the 'hints' structure
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get address information
    int getaddrinfo_status = getaddrinfo(NULL, PORT, &hints, &addr_info_list);
    if (getaddrinfo_status != 0) {
        fprintf(stderr, "setup_server_socket: %s\n", gai_strerror(getaddrinfo_status));
        exit(1);
    }

    // Iterate over the items in the linked list; stop when we successfully bind to one
    for (addr_info = addr_info_list; addr_info != NULL; addr_info = addr_info->ai_next) {
        // Try to get a socket
        server_socket = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
        if (server_socket == -1) {
            continue;
        }

        // Try to set the socket option to reuse address
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_flag, sizeof(int)) == -1) {
            close(server_socket);
            continue;
        }

        // Try to bind the socket to the address/port
        if (bind(server_socket, addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
            close(server_socket);
            continue;
        }

        printf("Successfully bind, stop looping");
        fflush(stdout);
        break;
    }

    freeaddrinfo(addr_info_list);

    if (addr_info == NULL) {
        // We didn't successfully bind any address
        return -1;
    }

    // Start listening
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        return -1;
    }

    printf("Everything is set up and we can return the server socket");
    fflush(stdout);
    return server_socket;
}

// void process_connection(preactor_t reactor, void *context)
// {
//     struct sockaddr_storage client_addr; 
//     socklen_t addr_len;
//     char client_IP[INET6_ADDRSTRLEN];
//     int client_socket;

//     addr_len = sizeof client_addr;
//     client_socket = accept(reactor->currentlyListen, (struct sockaddr *)&client_addr, &addr_len);
    
//     if (client_socket == -1)
//     {
//         perror("error during accept");
//     }
//     else
//     {
//         printf("server: established connection with %s on "
//                "socket %d\n",
        
//                inet_ntop(client_addr.ss_family,
//                 get_in_addr((struct sockaddr *)&client_addr),
//                 client_IP, INET6_ADDRSTRLEN),
//                 client_socket);
//         handler_t new_handler;
//         new_handler.arg = NULL;
//         new_handler.handler = &handle_client;
//         addFd(reactor, client_socket, new_handler.handler);
//     }
// }

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// void handle_client(preactor_t reactor, int socket_fd, void *context)
// {
//     char buffer[BUFF_SIZE] = {0};
//     int num_received;
//     printf("\nthe soc: %d", socket_fd);
//     fflush(stdout);
//     num_received = recv(socket_fd, buffer, BUFF_SIZE, 0);
//     if (num_received <= 0)
//     {
//         if (num_received == 0)
//         {
//             printf("server: socket %d disconnected\n", socket_fd);
//         }
//         else
//         {
//             printf("error during recv");
//         }
//         close(socket_fd);
//         deleteFd(reactor, socket_fd);
//     }
//     else
//     {
//         printf("server: socket %d received message: %s", socket_fd, buffer);
        
//         // Broadcast the message to all other clients
//         for (int i = 0; i < reactor->counter; i++)
//         {
//             if (reactor->fds[i].fd != socket_fd && reactor->fds[i].fd != reactor->currentlyListen)
//             {
//                 if (send(reactor->fds[i].fd, buffer, num_received, 0) == -1)
//                 {
//                     perror("error during send");
//                 }
//             }
//         }
//         printf("Message successfully broadcasted to all clients\n");
//     }
// }

void process_connection(preactor_t reactor, void *arg)
{
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    int newfd; // Newly accepted socket descriptor

    addrlen = sizeof remoteaddr;
    newfd = accept(reactor->currentlyListen, (struct sockaddr *)&remoteaddr, &addrlen); // Accept the incoming connection
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
        handler_t cHandler;
        cHandler.arg = NULL;
        cHandler.handler = &handle_client;
        addFd(reactor, newfd, &handle_client);
    }
}
void handle_client(preactor_t reactor, int client_fd, void *arg)
{
    char buf[BUFF_SIZE] = {0};
    int nbytes;

    nbytes = recv(client_fd, buf, BUFF_SIZE, 0);
    if (nbytes <= 0)
    {
        if (nbytes == 0)
        {
            printf("server: socket %d hung up\n", client_fd);
        }
        else
        {
            perror("recv");
        }
        close(client_fd);
        deleteFd(reactor, client_fd);
    }
    else
    {
        // send to all clients
        printf("server: socket %d got message: %s", client_fd, buf);
        for (int i = 0; i < reactor->counter; i++)
        {
            if (reactor->fds[i].fd != client_fd && reactor->fds[i].fd != reactor->currentlyListen)
            {
                if (send(reactor->fds[i].fd, buf, nbytes, 0) == -1)
                {
                    perror("send");
                }
            }
        }
        printf("message sent to all clients\n");
    }
}
