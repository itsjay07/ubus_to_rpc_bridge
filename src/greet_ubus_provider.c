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

enum {
    WELCOME_ATTR_NAME,
    __WELCOME_ATTR_MAX
};

static const struct blobmsg_policy welcome_policy[__WELCOME_ATTR_MAX] = {
    [WELCOME_ATTR_NAME] = { "name", BLOBMSG_TYPE_STRING },
};

static int handle_welcome(struct ubus_context *ctx, struct ubus_object *obj,
                          struct ubus_request_data *req, const char *method,
                          struct blob_attr *msg) {
    struct blob_attr *tb[__WELCOME_ATTR_MAX];
    const char *name;

    log_debug("Handling welcome request");
    blobmsg_parse(welcome_policy, __WELCOME_ATTR_MAX, tb, blob_data(msg), blob_len(msg));
    if (!tb[WELCOME_ATTR_NAME]) {
        log_warn("Missing name argument");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    name = blobmsg_data(tb[WELCOME_ATTR_NAME]);
    log_info("Greeting requested for: %s", name);

    char response[256];
    snprintf(response, sizeof(response), "Hello %s, Welcome to XYZ Company", name);

    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "message", response);
    ubus_send_reply(ctx, req, b.head);
    log_debug("Sent response: %s", response);
    return UBUS_STATUS_OK;
}

/* ✅ Use UBUS_METHOD macro – this is the critical fix */
static const struct ubus_method greet_methods[] = {
    UBUS_METHOD("welcome", handle_welcome, welcome_policy),
};

static struct ubus_object_type greet_object_type =
    UBUS_OBJECT_TYPE("greet", greet_methods);

static void ubus_connection_lost(struct ubus_context *ctx) {
    log_error("Connection to ubus lost");
    exit(1);
}

int main(int argc, char **argv) {
    log_info("Starting Greet ubus provider");
    ctx = ubus_connect(NULL);
    if (!ctx) { log_error("Failed to connect to ubus"); return 1; }
    log_info("Connected to ubus");
    ctx->connection_lost = ubus_connection_lost;

    memset(&obj, 0, sizeof(obj));   // ensure zeroed
    obj.name = "greet";
    obj.type = &greet_object_type;
    obj.methods = greet_methods;
    obj.n_methods = ARRAY_SIZE(greet_methods);

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