#include "Parse.h"

#include <stdio.h>
#include <string.h>

void get_parse_user_name(const char* str, char* user_name)
{
    bzero(user_name, sizeof(user_name));

    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>')
        {
            break;
        }
        if (write == 1)
        {
            user_name[len++] = str[i];
        }
    }
}

void send_parse_server_name(const char* str, char* server_name)
{
    bzero(server_name, sizeof(server_name));
    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>' && write == 2)
        {
            break;
        }
        if (write == 2)
        {
            server_name[len++] = str[i];
        }
    }
}

void send_parse_user_name_to(const char* str, char* user_name)
{
    bzero(user_name, sizeof(user_name));

    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>' && write == 3)
        {
            break;
        }
        if (write == 3)
        {
            user_name[len++] = str[i];
        }
    }
}

void send_parse_user_name_from(const char* str, char* user_name)
{
    bzero(user_name, sizeof(user_name));

    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>')
        {
            break;
        }
        if (write == 1)
        {
            user_name[len++] = str[i];
        }
    }
}

void log_parse_server_name_to(const char* str, char* server_name)
{
    bzero(server_name, sizeof(server_name));

    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>' && write == 3)
        {
            break;
        }
        if (write == 3)
        {
            server_name[len++] = str[i];
        }
    }
}

void log_parse_server_name_from(const char* str, char* server_name)
{
    bzero(server_name, sizeof(server_name));

    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>' && write == 1)
        {
            break;
        }
        if (write == 1)
        {
            server_name[len++] = str[i];
        }
    }
}

void log_parse_user_name_to(const char* str, char* user_name)
{
    bzero(user_name, sizeof(user_name));

    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>' && write == 4)
        {
            break;
        }
        if (write == 4)
        {
            user_name[len++] = str[i];
        }
    }
}

void log_parse_user_name_from(const char* str, char* user_name)
{
    bzero(user_name, sizeof(user_name));

    int write = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '<')
        {
            write++;
            continue;
        }
        if (str[i] == '>' && write == 2)
        {
            break;
        }
        if (write == 2)
        {
            user_name[len++] = str[i];
        }
    }
}

void parse_msg(const char* str, char* msg)
{
    bzero(msg, sizeof(msg));

    int write_msg = 0;
    int len = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ':')
        {
            write_msg++;
            continue;
        }
        if (write_msg == 1)
        {
            write_msg++;
            continue;
        }
        if (str[i] == '\r' || str[i] == '\n')
        {
            break;
        }
        if (write_msg == 2)
        {
            msg[len++] = str[i];
        }
    }
}
