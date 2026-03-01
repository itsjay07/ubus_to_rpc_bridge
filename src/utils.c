#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <json-c/json.h>
#include "../include/logging.h"
#include "../include/utils.h"
#include "../include/protocol.h"

int create_unix_client(const char* path) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) { log_error("Failed to create socket: %s", strerror(errno)); return -1; }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_debug("Failed to connect to %s: %s", path, strerror(errno));
        close(sock); return -1;
    }
    log_debug("Connected to Unix socket: %s", path);
    return sock;
}

int create_unix_server(const char* path) {
    unlink(path);
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) { log_error("Failed to create server socket: %s", strerror(errno)); return -1; }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error("Failed to bind to %s: %s", path, strerror(errno));
        close(sock); return -1;
    }
    if (listen(sock, 5) < 0) {
        log_error("Failed to listen on socket: %s", strerror(errno));
        close(sock); return -1;
    }
    log_info("Unix server listening on: %s", path);
    return sock;
}

int send_json(int sockfd, json_object* json) {
    const char* json_str = json_object_to_json_string(json);
    int len = strlen(json_str);
    if (write(sockfd, &len, sizeof(len)) != sizeof(len)) {
        log_error("Failed to send message length"); return -1;
    }
    if (write(sockfd, json_str, len) != len) {
        log_error("Failed to send message data"); return -1;
    }
    log_debug("Sent JSON: %s", json_str);
    return 0;
}

char* recv_json(int sockfd) {
    int len;
    if (read(sockfd, &len, sizeof(len)) != sizeof(len)) {
        if (errno != 0 && errno != ECONNRESET) {
            log_error("Failed to read message length: %s", strerror(errno));
        }
        return NULL;
    }
    if (len <= 0 || len > 4096) {
        log_error("Invalid message length: %d", len); return NULL;
    }
    char* buffer = malloc(len + 1);
    if (!buffer) { log_error("Failed to allocate buffer"); return NULL; }
    int total_read = 0;
    while (total_read < len) {
        int n = read(sockfd, buffer + total_read, len - total_read);
        if (n <= 0) {
            if (errno != 0) log_error("Failed to read message data: %s", strerror(errno));
            free(buffer); return NULL;
        }
        total_read += n;
    }
    buffer[len] = '\0';
    log_debug("Received JSON: %s", buffer);
    return buffer;
}

void close_socket(int* sockfd) {
    if (sockfd && *sockfd >= 0) { close(*sockfd); *sockfd = -1; }
}

json_object* build_rpc_request(int id, const char* method, const char* name) {
    json_object* request = json_object_new_object();
    json_object_object_add(request, "id", json_object_new_int(id));
    json_object_object_add(request, "method", json_object_new_string(method));
    json_object* params = json_object_new_object();
    json_object_object_add(params, "name", json_object_new_string(name));
    json_object_object_add(request, "params", params);
    return request;
}

json_object* build_rpc_response(int id, const char* message) {
    json_object* response = json_object_new_object();
    json_object_object_add(response, "id", json_object_new_int(id));
    json_object* result = json_object_new_object();
    json_object_object_add(result, "message", json_object_new_string(message));
    json_object_object_add(response, "result", result);
    json_object_object_add(response, "error", NULL);
    return response;
}

json_object* build_rpc_error(int id, int code, const char* msg) {
    json_object* response = json_object_new_object();
    json_object_object_add(response, "id", json_object_new_int(id));
    json_object_object_add(response, "result", NULL);
    json_object* error = json_object_new_object();
    json_object_object_add(error, "code", json_object_new_int(code));
    json_object_object_add(error, "message", json_object_new_string(msg));
    json_object_object_add(response, "error", error);
    return response;
}

int parse_rpc_request(const char* json_str, rpc_request_t* req) {
    json_object* root = json_tokener_parse(json_str);
    if (!root) return -1;
    json_object *id_obj, *method_obj, *params_obj, *name_obj;
    if (!json_object_object_get_ex(root, "id", &id_obj) ||
        !json_object_object_get_ex(root, "method", &method_obj) ||
        !json_object_object_get_ex(root, "params", &params_obj)) {
        json_object_put(root); return -1;
    }
    if (!json_object_object_get_ex(params_obj, "name", &name_obj)) {
        json_object_put(root); return -1;
    }
    req->id = json_object_get_int(id_obj);
    strncpy(req->method, json_object_get_string(method_obj), sizeof(req->method)-1);
    req->method[sizeof(req->method)-1] = '\0';
    strncpy(req->name, json_object_get_string(name_obj), sizeof(req->name)-1);
    req->name[sizeof(req->name)-1] = '\0';
    json_object_put(root);
    return 0;
}

int parse_rpc_response(const char* json_str, rpc_response_t* resp) {
    json_object* root = json_tokener_parse(json_str);
    if (!root) return -1;
    json_object *id_obj, *result_obj = NULL, *error_obj = NULL;
    if (!json_object_object_get_ex(root, "id", &id_obj)) {
        json_object_put(root); return -1;
    }
    resp->id = json_object_get_int(id_obj);
    resp->error_code = 0;
    resp->error_msg[0] = '\0';
    resp->message[0] = '\0';
    if (json_object_object_get_ex(root, "error", &error_obj) && error_obj != NULL) {
        json_object *code_obj, *msg_obj;
        if (json_object_object_get_ex(error_obj, "code", &code_obj) &&
            json_object_object_get_ex(error_obj, "message", &msg_obj)) {
            resp->error_code = json_object_get_int(code_obj);
            strncpy(resp->error_msg, json_object_get_string(msg_obj), sizeof(resp->error_msg)-1);
            resp->error_msg[sizeof(resp->error_msg)-1] = '\0';
        }
    } else if (json_object_object_get_ex(root, "result", &result_obj) && result_obj != NULL) {
        json_object *msg_obj;
        if (json_object_object_get_ex(result_obj, "message", &msg_obj)) {
            strncpy(resp->message, json_object_get_string(msg_obj), sizeof(resp->message)-1);
            resp->message[sizeof(resp->message)-1] = '\0';
        }
    }
    json_object_put(root);
    return 0;
}
