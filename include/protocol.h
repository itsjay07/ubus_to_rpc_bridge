#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <json-c/json.h>
#include <stdint.h>
#define SOCKET_PATH "/tmp/greet_rpc.sock"
#define MAX_MSG_SIZE 4096
typedef struct { int id; char method[64]; char name[256]; } rpc_request_t;
typedef struct { int id; char message[512]; int error_code; char error_msg[256]; } rpc_response_t;
json_object* build_rpc_request(int id, const char* method, const char* name);
json_object* build_rpc_response(int id, const char* message);
json_object* build_rpc_error(int id, int code, const char* msg);
int parse_rpc_request(const char* json_str, rpc_request_t* req);
int parse_rpc_response(const char* json_str, rpc_response_t* resp);
#endif
