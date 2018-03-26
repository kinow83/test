#ifndef __REQUEST_COMMAND__
#define __REQUEST_COMMAND__

#include <pthread.h>
#include <sys/socket.h> 
#include <time.h>
#include <stdint.h>

#define _COMMAND_PORT       8991
    
int make_command_socket(int *sock, uint16_t port, int secs);
int send_command_request(int sock, const char *command, int require_return, char **return_val);
#endif
