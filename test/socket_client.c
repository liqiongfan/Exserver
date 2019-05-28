/**
 * File:socket_client.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include <stdio.h>

#include "../kernel/exlist.h"
#include "../kernel/types.h"


#include "../kernel/net/socket.h"
#include <signal.h>

int client_fd;

void socket_term(int signo)
{
    close(client_fd);
    exit(0);
}

int main()
{
    client_fd = socket_create(AF_INET, SOCK_STREAM, 0);
    
    socket_connect(client_fd, "0.0.0.0", 8181);
    
    struct sigaction sig;
    memset(&sig, 0, sizeof(struct sigaction));
    sigemptyset(&sig.sa_mask);
    sig.sa_handler = socket_term;
    sigaddset(&sig.sa_mask, SIGUSR1);
    
    sigaction(SIGUSR1, &sig, NULL);
    
    char *str = "startaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaastartaaaaaaaaaaend";
    
    while (1)
    {
        // memset(str, 0, sizeof(str));
        // scanf("%s", str);
        int write_len = write(client_fd, str, sizeof(str));
        
        if ( strncmp(str, "close", 5) == 0)
        {
            kill(getpid(), SIGUSR1);
        }
        close(client_fd);
        break;
    }
    
    return 0;
}
