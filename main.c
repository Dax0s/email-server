#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>

typedef struct client
{
    int sock;
    char* username;
} client;

void error(const char *msg)
{
    fprintf(stderr, "[ERROR]: %s", msg);
    exit(1);
}

void signal_handler(int);

volatile int keep_running = 1;

int main(const int argc, const char** argv)
{
    struct sigaction act;
    act.sa_handler = signal_handler;
    sigaction(SIGINT, &act, NULL);

    if (argc != 4)
    {
        char* tmp = malloc(44 + strlen(argv[0]) + 1);
        sprintf(tmp, "Usage: %s <port> <server1-port> <server2-port>", argv[0]);

        error(tmp);

        free(tmp);
    }
    return 0;
}

void signal_handler(int)
{
    keep_running = 0;
    printf("\nServer closing...\n");

    exit(0);
}
