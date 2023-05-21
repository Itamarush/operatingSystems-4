#include "st_reactor.h"

#define PORT "9034"
#define BUFF_SIZE 256

void *get_in_addr(struct sockaddr*);
int get_listener_socket(void);
void connectionHandler(pReactor, int, void*);
void handleClient(pReactor, int, void*);
void sendToOtherClients(pReactor reactor, int sender_fd, char *message, int length);