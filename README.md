# ubus ↔ RPC Bridge System

A bidirectional bridge between OpenWrt's ubus and a custom RPC server over Unix domain sockets.

## Components

1. **greet_ubus_provider**: C program that registers a "greet" object with "welcome" method on ubus
2. **rpc_server**: C program that listens on Unix socket for JSON-RPC requests
3. **ubus_rpc_bridge**: C program that bridges between ubus and the RPC server

## Prerequisites

- Ubuntu system (tested on 20.04/22.04)
- Root/sudo access for installing dependencies

## Building

```bash
# Install dependencies
make install-deps

# Build all components
make clean all




Running the System
Terminal 1: Start ubus daemon
bash
make start-ubusd


Terminal 2: Start the greet ubus provider
bash
./bin/greet_ubus_provider


Terminal 3: Start the RPC server
bash
./bin/rpc_server


Terminal 4: Start the bridge
bash
./bin/ubus_rpc_bridge


/////////////////////////////////////////////
Testing
Test 1: ubus → RPC direction
bash
ubus call rpc_greet welcome '{"name": "Shripad"}'
Expected response:

json
{
  "message": "Hello Shripad, Welcome to XYZ Company (from RPC)"
}
Test 2: RPC → ubus direction
Using socat:

bash
echo '{"id":1,"method":"greet.welcome","params":{"name":"Shripad"}}' | socat - UNIX-CONNECT:/tmp/greet_rpc.sock
Expected response:

json
{
  "id": 1,
  "result": {
    "message": "Hello Shripad, Welcome to XYZ Company"
  },
  "error": null
}