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

#define BUFFER_SIZE 1024

typedef struct server
{
    int sock;
    char* server_name;
} server;

void error(const char *msg)
{
    fprintf(stderr, "[ERROR]: %s", msg);
    exit(1);
}

int bind_to_port(const char *port, int *sock, struct addrinfo **info);
int connect_server(int sock, struct addrinfo *server_addr, int *connected_servers, server** servers, struct pollfd** servers_pollfd);
void receive_data(int *connected_servers, struct pollfd** servers_pollfd, server** servers);
void signal_handler(int);

volatile int keep_running = 1;

int main(const int argc, const char** argv)
{
    struct sigaction act;
    act.sa_handler = signal_handler;
    sigaction(SIGINT, &act, NULL);

    if (argc != 2)
    {
        char* tmp = malloc(14 + strlen(argv[0]) + 1);
        sprintf(tmp, "Usage: %s <port>", argv[0]);

        error(tmp);

        free(tmp);
    }

    int sock;
    struct addrinfo *server_addr;
    if (bind_to_port(argv[1], &sock, &server_addr) != 0)
        error("failed to bind");

    printf("Server running on port %s\n", argv[1]);

    listen(sock, 5);
    struct pollfd sock_pollfd;
    sock_pollfd.fd = sock;
    sock_pollfd.events = POLLIN;

    int connected_servers = 0;
    server* servers = NULL;
    struct pollfd* servers_pollfd = NULL;

    while (keep_running)
    {
        if (poll(&sock_pollfd, 1, 0))
        {
            if (connect_server(sock, server_addr, &connected_servers, &servers, &servers_pollfd) != 0)
            {
                error("failed to connect");
            }
        }

        if (poll(servers_pollfd, connected_servers, 0))
        {
            receive_data(&connected_servers, &servers_pollfd, &servers);
        }
    }

    return 0;
}

int bind_to_port(const char *port, int *sock, struct addrinfo **info)
{
    struct addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &p) != 0)
        error("getaddrinfo error");

    *sock = -1;
    for (; p != NULL; p = p->ai_next)
    {
        if ((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(*sock, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(*sock);
            continue;
        }

        break;
    }

    if (p == NULL || *sock == -1)
        return 1;

    *info = p;

    return 0;
}

int connect_server(const int sock, struct addrinfo* server_addr, int* connected_servers, server** servers, struct pollfd** servers_pollfd)
{
    const int server_sock = accept(sock, server_addr->ai_addr, &server_addr->ai_addrlen);
    if (server_sock < 0)
        return 1;

    (*connected_servers)++;
    server* tmp = realloc(*servers, (*connected_servers) * sizeof(server));
    if (tmp == NULL)
        error("failed to allocate memory");
    *servers = tmp;

    server curr_server;
    curr_server.sock = server_sock;
    curr_server.server_name = NULL;
    (*servers)[*connected_servers - 1] = curr_server;

    struct pollfd* tmp_servers_pollfd = realloc(*servers_pollfd, sizeof(struct pollfd) * (*connected_servers));
    if (tmp_servers_pollfd == NULL)
        error("failed to allocate memory");
    *servers_pollfd = tmp_servers_pollfd;

    struct pollfd tmp_pollfd;
    tmp_pollfd.fd = server_sock;
    tmp_pollfd.events = POLLIN;
    (*servers_pollfd)[*connected_servers - 1] = tmp_pollfd;

    write(curr_server.sock, "ATSIUSKPAVADINIMA\n", 18);
    printf("Server connected. Socket fd: %d\n", curr_server.sock);

    return 0;
}

void disconnect_server(const int i, int* connected_clients, struct pollfd** servers_pollfd, server** servers)
{
    printf("Server with socket fd %d and title %s disconnected", (*servers_pollfd)[i].fd, servers[i]->server_name);

    close((*servers_pollfd)[i].fd);

    free((*servers)[i].server_name);

    for (int j = i; j < *connected_clients - 1; j++)
    {
        (*servers_pollfd)[j] = (*servers_pollfd)[j + 1];
        (*servers)[j] = (*servers)[j + 1];

        (*connected_clients)--;

        struct pollfd* tmp_pollfd = realloc(*servers_pollfd, sizeof(struct pollfd) * *connected_clients);
        if (tmp_pollfd == NULL)
            error("failed to allocate memory");
        *servers_pollfd = tmp_pollfd;

        server* tmp_servers = realloc(*servers, sizeof(server) * *connected_clients);
        if (tmp_servers == NULL)
            error("failed to allocate memory");
        *servers = tmp_servers;
    }
}

void receive_message(const int i, int* connected_servers, server** servers)
{
    char buffer[BUFFER_SIZE];

    read(servers[i]->sock, buffer, BUFFER_SIZE - 1);
    if (servers[i]->server_name == NULL)
    {
        if (buffer[strlen(buffer) - 2] == '\r')
            buffer[strlen(buffer) - 2] = '\0';
        else
            buffer[strlen(buffer) - 1] = '\0';

        servers[i]->server_name = malloc(strlen(buffer) + 1);
        strcpy(servers[i]->server_name, buffer);
        printf("Received server name from server with sock id %d: %s\n", servers[i]->sock, buffer);
        write(servers[i]->sock, "PAVADINIMASOK\n", 14);

        return;
    }
}

void receive_data(int *connected_servers, struct pollfd** servers_pollfd, server** servers)
{
    for (int i = 0; i < *connected_servers; i++)
    {
        if ((*servers_pollfd)[i].revents & POLLHUP)
        {
            disconnect_server(i, connected_servers, servers_pollfd, servers);
        }
        else if ((*servers_pollfd)[i].revents & POLLIN)
        {
            receive_message(i, connected_servers, servers);
        }
    }
}

void signal_handler(int)
{
    keep_running = 0;
    printf("\nServer closing...\n");

    exit(0);
}
