#include "st_reactor.h"

#define PORT "9034"
#define MAX_CLIENTS 100
#define BUFF_SIZE 256

// void sigHandler(int sig); // will stop reactor and close the program
int setup_server_socket(void);
void process_connection(preactor_t, void*);
void handle_client(preactor_t, int, void*);
void *get_in_addr(struct sockaddr *sa);


