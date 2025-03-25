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
#include <time.h>
#include "Parse.h"

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

int starts_with(const char* str, const char* prefix);

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

    printf("Server connected. Socket fd: %d\n", curr_server.sock);

    return 0;
}

void disconnect_server(const int i, int* connected_servers, struct pollfd** servers_pollfd, server** servers)
{
    printf("Server with socket fd %d and title %s disconnected\n", (*servers_pollfd)[i].fd, (*servers)[i].server_name);

    close((*servers_pollfd)[i].fd);

    free((*servers)[i].server_name);
    (*connected_servers)--;

    for (int j = i; j < *connected_servers - 1; j++)
    {
        (*servers_pollfd)[j] = (*servers_pollfd)[j + 1];
        (*servers)[j] = (*servers)[j + 1];
    }

    struct pollfd* tmp_pollfd = realloc(*servers_pollfd, sizeof(struct pollfd) * *connected_servers);
    if (tmp_pollfd == NULL)
        error("failed to allocate memory");
    *servers_pollfd = tmp_pollfd;

    server* tmp_servers = realloc(*servers, sizeof(server) * *connected_servers);
    if (tmp_servers == NULL)
        error("failed to allocate memory");
    *servers = tmp_servers;
}

void receive_message(const int i, int* connected_servers, server** servers)
{
    char buffer[BUFFER_SIZE] = "";

    read((*servers)[i].sock, buffer, BUFFER_SIZE - 1);
    if ((*servers)[i].server_name == NULL)
    {
        if (buffer[strlen(buffer) - 2] == '\r')
            buffer[strlen(buffer) - 2] = '\0';
        else if (buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';

        (*servers)[i].server_name = malloc(strlen(buffer) + 1);
        strcpy((*servers)[i].server_name, buffer);
        printf("Received server name from server with sock id %d: %s\n", (*servers)[i].sock, buffer);

        return;
    }

    if (starts_with(buffer, "@get") == 0)
    {
        // @get <user-to>
        char user_name[BUFFER_SIZE] = "";

        get_parse_user_name(buffer, user_name);

        char file_server_name[BUFFER_SIZE] = "";
        char file_user_name[BUFFER_SIZE] = "";
        FILE* file = fopen("log.txt", "r");
        char file_buffer[BUFFER_SIZE] = "";

        if (file == NULL)
        {
            write((*servers)[i].sock, "@end\n", 5);
            return;
        }
        while (fgets(file_buffer, BUFFER_SIZE - 1, file))
        {
            bzero(file_server_name, BUFFER_SIZE);
            bzero(file_user_name, BUFFER_SIZE);
            log_parse_server_name_to(file_buffer, file_server_name);
            log_parse_user_name_to(file_buffer, file_user_name);

            if (strcmp((*servers)[i].server_name, file_server_name) == 0 && strcmp(user_name, file_user_name) == 0)
            {
                write((*servers)[i].sock, file_buffer, strlen(file_buffer));
                bzero(file_buffer, BUFFER_SIZE);
                read((*servers)[i].sock, file_buffer, BUFFER_SIZE - 1);

                if (starts_with(file_buffer, "@received") != 0)
                    return;
            }

            bzero(file_buffer, BUFFER_SIZE);
        }
        fclose(file);

        write((*servers)[i].sock, "@end\n", 5);
    }
    else if (starts_with(buffer, "@send") == 0)
    {
        // @send <server-name> <user-to> <user-from>: msg
        char server_name_to[BUFFER_SIZE] = "";
        char user_name_to[BUFFER_SIZE] = "";
        char user_name_from[BUFFER_SIZE] = "";
        char msg[BUFFER_SIZE] = "";

        send_parse_server_name(buffer, server_name_to);
        send_parse_user_name_to(buffer, user_name_to);
        send_parse_user_name_from(buffer, user_name_from);
        parse_msg(buffer, msg);

        FILE* file = fopen("log.txt", "a");
        if (file == NULL)
            error("failed to open file");

        const time_t t = time(NULL);
        const struct tm tm = *localtime(&t);
        fprintf(file, "[%d-%02d-%02d %02d:%02d:%02d] <%s> <%s> -> <%s> <%s>: %s\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            (*servers)[i].server_name, user_name_from, server_name_to, user_name_to, msg);
        fclose(file);
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

int starts_with(const char* str, const char* prefix)
{
    return strncmp(str, prefix, strlen(prefix));
}

void signal_handler(int)
{
    keep_running = 0;
    printf("\nServer closing...\n");

    exit(0);
}
