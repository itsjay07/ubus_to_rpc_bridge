#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <json-c/json.h>
#include "../include/logging.h"
#include "../include/protocol.h"
#include "../include/utils.h"

static struct ubus_context *ctx;
static struct ubus_object bridge_obj;
static int rpc_sock = -1;
static int keep_running = 1;
static pthread_mutex_t rpc_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct blob_buf b;

enum {
    BRIDGE_ATTR_NAME,
    __BRIDGE_ATTR_MAX
};

static const struct blobmsg_policy bridge_policy[__BRIDGE_ATTR_MAX] = {
    [BRIDGE_ATTR_NAME] = { "name", BLOBMSG_TYPE_STRING },
};

static int ubus_to_rpc(const char* name, char* response_buf, size_t buf_size) {
    pthread_mutex_lock(&rpc_mutex);
    if (rpc_sock < 0) {
        rpc_sock = create_unix_client(SOCKET_PATH);
        if (rpc_sock < 0) { pthread_mutex_unlock(&rpc_mutex); return -1; }
    }
    json_object* request = build_rpc_request(1, "greet.welcome", name);
    int ret = send_json(rpc_sock, request);
    json_object_put(request);
    if (ret < 0) { close_socket(&rpc_sock); pthread_mutex_unlock(&rpc_mutex); return -1; }
    char* resp_str = recv_json(rpc_sock);
    if (!resp_str) { close_socket(&rpc_sock); pthread_mutex_unlock(&rpc_mutex); return -1; }
    rpc_response_t resp;
    memset(&resp, 0, sizeof(resp));
    if (parse_rpc_response(resp_str, &resp) < 0) {
        free(resp_str); close_socket(&rpc_sock); pthread_mutex_unlock(&rpc_mutex); return -1;
    }
    if (resp.error_code != 0) {
        log_error("RPC error: %s", resp.error_msg);
        free(resp_str); pthread_mutex_unlock(&rpc_mutex); return -1;
    }
    strncpy(response_buf, resp.message, buf_size - 1);
    response_buf[buf_size - 1] = '\0';
    free(resp_str);
    pthread_mutex_unlock(&rpc_mutex);
    return 0;
}

static int handle_bridge_welcome(struct ubus_context *ctx, struct ubus_object *obj,
                                 struct ubus_request_data *req, const char *method,
                                 struct blob_attr *msg) {
    struct blob_attr *tb[__BRIDGE_ATTR_MAX];
    const char *name;

    log_debug("Bridge received ubus welcome request");
    blobmsg_parse(bridge_policy, __BRIDGE_ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[BRIDGE_ATTR_NAME]) {
        log_warn("Missing name argument in ubus call");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    name = blobmsg_data(tb[BRIDGE_ATTR_NAME]);
    log_info("Bridge forwarding to RPC: name=%s", name);

    char response[512];
    if (ubus_to_rpc(name, response, sizeof(response)) < 0) {
        log_error("Failed to get response from RPC server");
        return UBUS_STATUS_UNKNOWN_ERROR;
    }
    log_info("Bridge received from RPC: %s", response);

    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "message", response);
    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}

/* ✅ Use UBUS_METHOD macro */
static const struct ubus_method bridge_methods[] = {
    UBUS_METHOD("welcome", handle_bridge_welcome, bridge_policy),
};

static struct ubus_object_type bridge_object_type =
    UBUS_OBJECT_TYPE("rpc_greet", bridge_methods);

static void* rpc_listener_thread(void* arg) {
    int listen_sock = -1;
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    log_info("RPC listener thread started");
    while (keep_running) {
        if (listen_sock < 0) {
            listen_sock = create_unix_server("/tmp/bridge_listener.sock");
            if (listen_sock < 0) { sleep(1); continue; }
        }
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_sock, &readfds);
        struct timeval tv = {1, 0};
        int activity = select(listen_sock + 1, &readfds, NULL, NULL, &tv);
        if (activity < 0) { if (errno != EINTR) log_error("Select error: %s", strerror(errno)); continue; }
        if (activity > 0 && FD_ISSET(listen_sock, &readfds)) {
            int client_fd = accept(listen_sock, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) { log_error("Accept failed: %s", strerror(errno)); continue; }
            char* req_str = recv_json(client_fd);
            if (!req_str) { close(client_fd); continue; }
            rpc_request_t req;
            memset(&req, 0, sizeof(req));
            if (parse_rpc_request(req_str, &req) < 0) {
                log_error("Invalid RPC request format");
                json_object* error = build_rpc_error(0, 400, "invalid request");
                send_json(client_fd, error); json_object_put(error);
                free(req_str); close(client_fd); continue;
            }
            log_info("RPC -> ubus: id=%d, method=%s, name=%s", req.id, req.method, req.name);
            if (strcmp(req.method, "greet.welcome") == 0) {
                // Simulated response for now (you can replace with real ubus call later)
                char ubus_resp[512];
                snprintf(ubus_resp, sizeof(ubus_resp), "Hello %s, Welcome to XYZ Company", req.name);
                json_object* response = build_rpc_response(req.id, ubus_resp);
                send_json(client_fd, response); json_object_put(response);
                log_info("RPC -> ubus: sent simulated response");
            } else {
                json_object* error = build_rpc_error(req.id, 404, "method not found");
                send_json(client_fd, error); json_object_put(error);
            }
            free(req_str); close(client_fd);
        }
    }
    if (listen_sock >= 0) { close(listen_sock); unlink("/tmp/bridge_listener.sock"); }
    return NULL;
}

static void ubus_connection_lost(struct ubus_context *ctx) {
    log_error("Connection to ubus lost");
    keep_running = 0;
}

void handle_signal(int sig) {
    log_info("Received signal %d, shutting down...", sig);
    keep_running = 0;
}

int main(int argc, char **argv) {
    log_info("Starting ubus-rpc bridge");
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    ctx = ubus_connect(NULL);
    if (!ctx) { log_error("Failed to connect to ubus"); return 1; }
    log_info("Connected to ubus");
    ctx->connection_lost = ubus_connection_lost;

    memset(&bridge_obj, 0, sizeof(bridge_obj));
    bridge_obj.name = "rpc_greet";
    bridge_obj.type = &bridge_object_type;
    bridge_obj.methods = bridge_methods;
    bridge_obj.n_methods = ARRAY_SIZE(bridge_methods);

    int ret = ubus_add_object(ctx, &bridge_obj);
    if (ret) {
        log_error("Failed to add bridge object: %s", ubus_strerror(ret));
        ubus_free(ctx);
        return 1;
    }
    log_info("Registered bridge ubus object: rpc_greet");

    pthread_t listener_thread;
    if (pthread_create(&listener_thread, NULL, rpc_listener_thread, NULL) != 0) {
        log_error("Failed to create listener thread");
        ubus_free(ctx);
        return 1;
    }

    ubus_add_uloop(ctx);
    uloop_init();
    while (keep_running) uloop_run_timeout(1);
    uloop_done();
    pthread_join(listener_thread, NULL);
    close_socket(&rpc_sock);
    ubus_free(ctx);
    log_info("Bridge stopped");
    return 0;
}