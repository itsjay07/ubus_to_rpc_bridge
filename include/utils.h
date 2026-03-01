#ifndef UTILS_H
#define UTILS_H
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <json-c/json.h>
int create_unix_client(const char* path);
int create_unix_server(const char* path);
int send_json(int sockfd, json_object* json);
char* recv_json(int sockfd);
void close_socket(int* sockfd);
#endif
