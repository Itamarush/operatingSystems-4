#include "reactorServer.h"
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

        // Successfully bound, stop looping
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

    // Everything is set up and we can return the server socket
    return server_socket;
}