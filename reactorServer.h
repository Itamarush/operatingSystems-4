#include "st_reactor.h"
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

#define PORT "9034"
#define MAX_CLIENTS 50
#define BUFF_SIZE 256

void sigHandler(int sig); // will stop reactor and close the program
int setup_server_socket(void);  // get listener socket from beej's guide
void connectionHandler(preactor_t reactor,void *arg); // handle new connections 
void clientHandler(preactor_t reactor,int client_fd,void *arg); // recieve and send messages to clients
