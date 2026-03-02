#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libubus.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>
#include "../include/logging.h"

static struct ubus_context *ctx;
static struct ubus_object obj;
static struct blob_buf b;

static int handle_welcome(struct ubus_context *ctx, struct ubus_object *obj,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg) {
    struct blob_attr *tb[1];
    const char *name;

    log_debug("Handling welcome request");
    
    static const struct blobmsg_policy policy = { "name", BLOBMSG_TYPE_STRING };
    blobmsg_parse(&policy, 1, tb, blob_data(msg), blob_len(msg));

    if (!tb[0]) {
        log_warn("Missing name argument");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    name = blobmsg_data(tb[0]);
    log_info("Greeting requested for: %s", name);

    char response[256];
    snprintf(response, sizeof(response), "Hello %s, Welcome to XYZ Company", name);

    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "message", response);
    ubus_send_reply(ctx, req, b.head);
    log_debug("Sent response: %s", response);
    return UBUS_STATUS_OK;
}

/* SIMPLE method definition - exactly like your working test */
static struct ubus_method greet_methods[] = {
    {
        .name = "welcome",
        .handler = handle_welcome
    },
};

static struct ubus_object_type greet_object_type = {
    .name = "greet",
    .methods = greet_methods,
    .n_methods = 1
};

static void ubus_connection_lost(struct ubus_context *ctx) {
    log_error("Connection to ubus lost");
    exit(1);
}

int main(int argc, char **argv) {
    log_info("Starting Greet ubus provider");
    
    ctx = ubus_connect("/tmp/ubus.sock");
    if (!ctx) {
        log_error("Failed to connect to ubus");
        return 1;
    }
    log_info("Connected to ubus");
    
    ctx->connection_lost = ubus_connection_lost;

    memset(&obj, 0, sizeof(obj));
    obj.name = "greet";
    obj.type = &greet_object_type;
    obj.methods = greet_methods;
    obj.n_methods = 1;

    int ret = ubus_add_object(ctx, &obj);
    if (ret) {
        log_error("Failed to add object: %s", ubus_strerror(ret));
        ubus_free(ctx);
        return 1;
    }
    log_info("Registered ubus object: greet");

    ubus_add_uloop(ctx);
    uloop_init();
    uloop_run();
    
    ubus_free(ctx);
    return 0;
}