#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <json-c/json.h>
#include <errno.h>
#include "../include/logging.h"
#include "../include/protocol.h"
#include "../include/utils.h"

static int server_fd = -1;
static int keep_running = 1;

void handle_signal(int sig) {
    log_info("Received signal %d, shutting down...", sig);
    keep_running = 0;
}

char* handle_greet(const char* name) {
    static char response[256];
    snprintf(response, sizeof(response), "Hello %s, Welcome to XYZ Company (from RPC)", name);
    return response;
}

void handle_client(int client_fd) {
    log_debug("New client connected");
    char* json_str = recv_json(client_fd);
    if (!json_str) { close(client_fd); return; }
    
    rpc_request_t req;
    memset(&req, 0, sizeof(req));
    
    if (parse_rpc_request(json_str, &req) < 0) {
        log_error("Invalid request format");
        json_object* error = build_rpc_error(0, 400, "invalid request");
        send_json(client_fd, error);
        json_object_put(error);
        free(json_str);
        close(client_fd);
        return;
    }
    
    log_info("RPC request: id=%d, method=%s, name=%s", req.id, req.method, req.name);
    
    json_object* response;
    if (strcmp(req.method, "greet.welcome") == 0) {
        char* greeting = handle_greet(req.name);
        response = build_rpc_response(req.id, greeting);
    } else {
        response = build_rpc_error(req.id, 404, "method not found");
    }
    
    send_json(client_fd, response);
    json_object_put(response);
    free(json_str);
    close(client_fd);
    log_debug("Client disconnected");
}

int main(int argc, char **argv) {
    log_info("Starting RPC server");
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    server_fd = create_unix_server(SOCKET_PATH);
    if (server_fd < 0) {
        log_error("Failed to create server");
        return 1;
    }
    
    while (keep_running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        struct timeval tv = {1, 0};
        int activity = select(server_fd + 1, &readfds, NULL, NULL, &tv);
        
        if (activity < 0) {
            if (errno != EINTR) log_error("Select error: %s", strerror(errno));
            continue;
        }
        
        if (activity > 0 && FD_ISSET(server_fd, &readfds)) {
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd < 0) {
                log_error("Accept failed: %s", strerror(errno));
                continue;
            }
            handle_client(client_fd);
        }
    }
    
    close_socket(&server_fd);
    unlink(SOCKET_PATH);
    log_info("RPC server stopped");
    return 0;
}
