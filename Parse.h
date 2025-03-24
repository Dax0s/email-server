#ifndef PARSE_H
#define PARSE_H

void get_parse_user_name(const char* str, char* user_name);

void send_parse_server_name(const char* str, char* server_name);
void send_parse_user_name_to(const char* str, char* user_name);
void send_parse_user_name_from(const char* str, char* user_name);

void log_parse_server_name_to(const char* str, char* server_name);
void log_parse_server_name_from(const char* str, char* server_name);
void log_parse_user_name_to(const char* str, char* user_name);
void log_parse_user_name_from(const char* str, char* user_name);

void parse_msg(const char* str, char* msg);

#endif
