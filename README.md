# 🔄 UBUS ↔ RPC BRIDGE

A bidirectional communication bridge between OpenWrt's **ubus** message bus and a custom **JSON-RPC** server over Unix Domain Sockets.

<p align="center">
  <img src="https://img.shields.io/badge/version-1.0.0-blue.svg" alt="Version">
  <img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/OpenWrt-ubus-orange.svg" alt="OpenWrt">
  <img src="https://img.shields.io/badge/language-C-00599C.svg" alt="Language">
  <img src="https://img.shields.io/badge/platform-Linux%2FWSL-yellow.svg" alt="Platform">
</p>

---

## 📋 TABLE OF CONTENTS
- [Project Overview](#project-overview)
- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building the Project](#building-the-project)
- [Running the System](#running-the-system)
- [Testing Both Directions](#testing-both-directions)
- [The Debugging Journey](#the-debugging-journey)
- [Root Cause Analysis](#root-cause-analysis)
- [Protocol Specification](#protocol-specification)
- [Code Structure](#code-structure)
- [Troubleshooting](#troubleshooting)


---

## 🎯 PROJECT OVERVIEW

### What is this?
This project implements a **2-way bridge** between two different IPC systems:
1. **ubus** - OpenWrt's message bus (used in routers/embedded devices)
2. **JSON-RPC over Unix Domain Sockets** - Custom RPC implementation

### Why?
- Connect embedded systems (routers) with modern applications
- Demonstrate protocol translation between binary (ubus) and JSON
- Enable ubus-based applications to communicate with RPC clients

### Features
- 🔄 **Bidirectional** - Both sides can initiate communication
- 🔀 **Protocol Translation** - ubus ↔ JSON-RPC
- ⚡ **Unix Sockets** - Fast local IPC
- 📝 **JSON Messages** - Human-readable format
- 🎨 **Colored Logging** - Beautiful terminal output
- 🛡️ **Error Handling** - Comprehensive error management

---

## 🏗️ ARCHITECTURE
```text
┌───────────────────────────────────────────────────────────────────────────────┐
│                     BIDIRECTIONAL UBUS ↔ RPC BRIDGE                          │
└───────────────────────────────────────────────────────────────────────────────┘


                        ┌──────────────────────┐
                        │      RPC CLIENT      │
                        │   /tmp/rpc_client    │
                        └──────────┬───────────┘
                                   │
                     JSON-RPC      │
               {method:"welcome"}  │
                                   ▼
                        ┌──────────────────────┐
                        │      RPC SERVER      │
                        │ /tmp/greet_rpc.sock  │
                        └──────────┬───────────┘
                                   │
                                   ▼
═══════════════════════════════════════════════════════════════════════════════
                        UBUS ↔ RPC BRIDGE (ubus_rpc_bridge.c)
═══════════════════════════════════════════════════════════════════════════════
        ┌──────────────────────┐        ┌──────────────────────┐
        │   ubus Client Side   │  ↔↔↔  │   RPC Client Side    │
        │ (handles ubus calls) │        │ (handles RPC calls)  │
        └──────────┬───────────┘        └──────────┬───────────┘
                   │                                 │
                   ▼                                 ▼
        ┌──────────────────────┐        ┌──────────────────────┐
        │        ubusd         │        │      RPC SERVER      │
        │  /tmp/ubus.sock      │        │  (external service)  │
        └──────────┬───────────┘        └──────────┬───────────┘
                   │
                   ▼
        ┌───────────────────────────────────────────┐
        │              GREET PROVIDER               │
        │     (greet_ubus_provider.c)               │
        │-------------------------------------------│
        │ ubus Object : "greet"                     │
        │ Method      : "welcome"                   │
        │ Policy      : name (string)               │
        │ Response    : { "message": "Hello X" }    │
        └───────────────────────────────────────────┘


======================== COMMUNICATION FLOW ========================

RPC → ubus:
RPC Client → RPC Server → Bridge → ubusd → Greet Provider

ubus → RPC:
ubus Client → Bridge → RPC Server → RPC Client

====================================================================
```
```text
═══════════════════════════════════════════════════════════════════════════════
                               DATA FLOW
═══════════════════════════════════════════════════════════════════════════════

COMMUNICATION FLOW
───────────────────────────────────────────────────────────────────────────────
Direction      Path
───────────────────────────────────────────────────────────────────────────────
(1,2)  RPC → ubus
       RPC Client → RPC Server → Bridge → ubusd → Provider

(3,4,5) ubus → RPC
       ubus Client → Bridge → RPC Server → RPC Client
───────────────────────────────────────────────────────────────────────────────



═══════════════════════════════════════════════════════════════════════════════
                              COMPONENT DETAILS
═══════════════════════════════════════════════════════════════════════════════

Component      File/Source                     Purpose
───────────────────────────────────────────────────────────────────────────────
ubusd          System daemon                   OpenWrt message bus core
               (/usr/local/sbin/ubusd)         Manages ubus objects & routing

Provider       src/greet_ubus_provider.c       Registers "greet" object
                                               Handles "welcome" method

RPC Server     src/rpc_server.c                Listens on Unix socket
                                               Processes JSON-RPC requests

Bridge         src/ubus_rpc_bridge.c           Core connector
                                               Registers "rpc_greet"
                                               Forwards requests both ways

Test Client    /tmp/rpc_client.c               Test tool for RPC → ubus
               (not in repo)                   Sends length-prefixed JSON



═══════════════════════════════════════════════════════════════════════════════
                              PROTOCOL STACK
═══════════════════════════════════════════════════════════════════════════════

RPC ↔ Bridge
───────────────────────────────────────────────────────────────────────────────
Application : JSON-RPC
Format      : { "id":1, "method":"greet.welcome", "params":{...} }

Framing     : 4-byte length prefix + JSON payload
Example     : [00 00 00 48][JSON string]

Transport   : Unix Domain Socket (SOCK_STREAM)
Socket      : /tmp/greet_rpc.sock
───────────────────────────────────────────────────────────────────────────────


Bridge ↔ ubus
───────────────────────────────────────────────────────────────────────────────
Protocol    : ubus binary protocol
Connection  : ubus_connect(NULL) or explicit socket path
Operations  : ubus_add_object()
              ubus_invoke()
              ubus_send_reply()
───────────────────────────────────────────────────────────────────────────────



═══════════════════════════════════════════════════════════════════════════════
                    MESSAGE SEQUENCE A : RPC → ubus
═══════════════════════════════════════════════════════════════════════════════

RPC Client      RPC Server        Bridge          ubusd         Provider
    │                │                │               │               │
    │──JSON Req─────>│                │               │               │
    │ {greet.welcome}│                │               │               │
    │                │──Forward──────>│               │               │
    │                │                │──ubus call───>│               │
    │                │                │ "greet.welcome"               │
    │                │                │               │──invoke──────>│
    │                │                │               │<──response────│
    │                │                │<──ubus resp───│               │
    │                │<──RPC resp─────│               │               │
    │<──Result───────│                │               │               │



═══════════════════════════════════════════════════════════════════════════════
                    MESSAGE SEQUENCE B : ubus → RPC
═══════════════════════════════════════════════════════════════════════════════

ubus Client      Bridge         RPC Server        RPC Client
     │               │               │               │
     │──ubus call───>│               │               │
     │ "rpc_greet"   │               │               │
     │               │──RPC Req─────>│               │
     │               │ {greet.welcome}              │
     │               │               │──response────>│
     │               │<──RPC resp────│               │
     │<──ubus resp───│               │               │

═══════════════════════════════════════════════════════════════════════════════
```

## 📦 PREREQUISITES

### System Requirements
- Ubuntu 20.04/22.04/24.04 (or any Debian-based Linux)
- WSL (Windows Subsystem for Linux) works
- 100MB free disk space
- Root/sudo access

### Required Packages
```bash
# Build essentials
sudo apt update
sudo apt install -y build-essential cmake git pkg-config

# Libraries & tools
sudo apt install -y libjson-c-dev socat



🔧 INSTALLATION
Method 1: Install ubus from Ubuntu Repos (if available)
bash
sudo apt install -y ubus ubusd libubus-dev libubox-dev

Method 2: Build ubus from Source (Recommended)
bash
# Build libubox
cd /tmp
git clone https://git.openwrt.org/project/libubox.git
cd libubox
cmake -DBUILD_LUA=OFF -DCMAKE_INSTALL_PREFIX=/usr/local .
make
sudo make install

# Build ubus
cd /tmp
git clone https://git.openwrt.org/project/ubus.git
cd ubus
cmake -DBUILD_LUA=OFF -DCMAKE_INSTALL_PREFIX=/usr/local .
make
sudo make install

# Update library cache
sudo ldconfig
Verify Installation
bash
which ubusd      # Should show /usr/local/sbin/ubusd
ubus -v          # Show version


🏗️ BUILDING THE PROJECT
bash
# Clone or navigate to project
cd /path/to/ubus_to_rpc_bridge

# Build everything
make clean
make all

# Verify binaries were created
ls -la bin/
# Should show:
# - greet_ubus_provider
# - rpc_server
# - ubus_rpc_bridge


🚀 RUNNING THE SYSTEM
You need 5 terminals to run everything.


TERMINAL 1: Start ubusd
bash
cd "/path/to/ubus_to_rpc_bridge"
sudo pkill ubusd
sudo ubusd -s /tmp/ubus.sock &
sleep 2
pgrep ubusd && echo "✅ ubusd running" || echo "❌ ubusd not running"


TERMINAL 2: Start greet-ubus-provider
bash
cd "/path/to/ubus_to_rpc_bridge"
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
sudo ./bin/greet_ubus_provider
# Expected: "Registered ubus object: greet"


TERMINAL 3: Start rpc-server
bash
cd "/path/to/ubus_to_rpc_bridge"
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
sudo rm -f /tmp/greet_rpc.sock
sudo ./bin/rpc_server
# Expected: "Unix server listening on: /tmp/greet_rpc.sock"


TERMINAL 4: Start ubus-rpc-bridge
bash
cd "/path/to/ubus_to_rpc_bridge"
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
sudo ./bin/ubus_rpc_bridge
# Expected: "Registered bridge ubus object: rpc_greet"


TERMINAL 5: Test Client (create once)
bash
cd "/path/to/ubus_to_rpc_bridge"

# Create the test client
cat > /tmp/rpc_client.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/greet_rpc.sock");
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }
    printf("✅ Connected to RPC server\n");
    
    char *json = "{ \"id\": 1, \"method\": \"greet.welcome\", \"params\": { \"name\": \"Shripad\" } }";
    int len = strlen(json);
    
    printf("📤 Sending: %s\n", json);
    write(sock, &len, sizeof(len));
    write(sock, json, len);
    
    int resp_len;
    read(sock, &resp_len, sizeof(resp_len));
    char buffer[4096] = {0};
    read(sock, buffer, resp_len);
    printf("📥 Response (%d bytes):\n%s\n", resp_len, buffer);
    
    close(sock);
    return 0;
}
EOF

gcc -o /tmp/rpc_client /tmp/rpc_client.c
sudo chmod 777 /tmp/greet_rpc.sock


🧪 TESTING BOTH DIRECTIONS
In TERMINAL 5, run these tests:
bash
echo "=================================="
echo "🔵 DIRECTION 1: ubus → RPC"
echo "=================================="
sudo ubus -s /tmp/ubus.sock call rpc_greet welcome '{"name":"Shripad"}'

echo -e "\n=================================="
echo "🔴 DIRECTION 2: RPC → ubus" 
echo "=================================="
sudo /tmp/rpc_client
Expected Output:

==================================
🔵 DIRECTION 1: ubus → RPC
==================================
{
  "message": "Hello Shripad, Welcome to XYZ Company (from RPC)"
}

==================================
🔴 DIRECTION 2: RPC → ubus
==================================
✅ Connected to RPC server
📤 Sending: { "id": 1, "method": "greet.welcome", "params": { "name": "Shripad" } }
📥 Response (103 bytes):
{ "id": 1, "result": { "message": "Hello Shripad, Welcome to XYZ Company" }, "error": null }


Stop Everything (When Done)
bash
sudo pkill ubusd
sudo pkill -f greet_ubus_provider
sudo pkill -f rpc_server
sudo pkill -f ubus_rpc_bridge
rm -f /tmp/ubus.sock /tmp/greet_rpc.sock
echo "✅ All components stopped"


🔍 THE DEBUGGING JOURNEY
This project took over 10 hours of debugging. Here's what went wrong and how it was fixed:


Issue 1: "Failed to add object: Invalid argument"
Attempt	What We Tried	Result
❌	Using UBUS_METHOD macro	Failed
❌	Using UBUS_METHOD_NOARG	Failed
❌	Using UBUS_METHOD with empty policy	Failed
❌	Manual struct initialization with policy	Failed
✅	Ultra-simple struct with no policy	WORKED!


Issue 2: RPC → ubus "Connection reset by peer"
Attempt	What We Tried	Result
❌	Using socat with raw JSON	Failed
❌	Using socat with sudo	Failed
❌	Changing socket permissions	Failed
✅	Custom C client with length prefix	WORKED!


Issue 3: Missing bridge object
Attempt	What We Tried	Result
❌	Starting components in wrong order	Failed
✅	Starting in correct order: ubusd → provider → rpc-server → bridge	WORKED!
🎯 ROOT CAUSE ANALYSIS
The Main Problem: Method Definition Format Mismatch
Your ubus version expected method definitions in a specific format that the macros didn't provide:

What DIDN'T work (using macros):

c
static const struct ubus_method greet_methods[] = {
    UBUS_METHOD("welcome", handle_welcome, welcome_policy),
};
What FINALLY worked (direct struct initialization):

c
static struct ubus_method greet_methods[] = {
    {
        .name = "welcome",
        .handler = handle_welcome
        /* NO policy field - let it default to NULL */
    },
};


Other Critical Issues:
Issue	Root Cause	Solution
Macro Mismatch	ubus version didn't support macros	Direct struct initialization
Policy Field	Including policy caused "Invalid argument"	Omitted policy field entirely
const Qualifier	static const caused issues	Used non-const static
Socket Path	ubusd used /tmp/ubus.sock not default	Explicit -s /tmp/ubus.sock
RPC Protocol	socat sends raw JSON without length	Custom client with length prefix
WSL Issues	Working in /mnt/d/ caused corruption	(Identified but worked around)


The Breakthrough Test:
c
// This worked! (object with no methods)
obj.name = "test_object";
obj.methods = NULL;
obj.n_methods = 0;

// This failed! (object with methods) 
// → Problem was in method definitions, not ubus itself


📡 PROTOCOL SPECIFICATION
Wire Format
text
[4-byte length][JSON message]
Example: 00 00 00 48 7b 22 69 64 22 3a 31 2c ...

Request Format
json
{
  "id": 1,
  "method": "greet.welcome",
  "params": {
    "name": "Shripad"
  }
}

Success Response
json
{
  "id": 1,
  "result": {
    "message": "Hello Shripad, Welcome to XYZ Company"
  },
  "error": null
}

Error Response
json
{
  "id": 1,
  "result": null,
  "error": {
    "code": 400,
    "message": "invalid request"
  }
}


Error Codes
Code	Meaning	Description
400	Bad Request	Invalid JSON format
404	Not Found	Method doesn't exist
500	Internal Error	ubus call failed


📁 CODE STRUCTURE
text
📦 ubus_to_rpc_bridge
├── 📂 bin/                 # Compiled binaries (after build)
│   ├── greet_ubus_provider
│   ├── rpc_server
│   └── ubus_rpc_bridge
├── 📂 include/             # Header files
│   ├── logging.h           # Colored logging macros
│   ├── protocol.h          # Protocol definitions
│   └── utils.h             # Utility declarations
├── 📂 src/                 # Source files
│   ├── greet_ubus_provider.c  # ubus provider with "greet" object
│   ├── rpc_server.c           # JSON-RPC server over Unix socket
│   ├── ubus_rpc_bridge.c      # Main bridge (connects ubus and RPC)
│   └── utils.c                # Shared utilities (socket, JSON)
├── 📄 Makefile             # Build automation
└── 📄 README.md            # This file



Key Files Explained
File	Purpose	Key Functions
greet_ubus_provider.c	Registers "greet" on ubus	handle_welcome(), main()
rpc_server.c	Listens for RPC requests	handle_client(), handle_greet()
ubus_rpc_bridge.c	Bridges both worlds	handle_bridge_welcome(), ubus_to_rpc()
utils.c	Socket/JSON helpers	create_unix_server(), send_json()


🔧 TROUBLESHOOTING
Common Issues and Fixes
Error	Probable Cause	Solution
Failed to add object: Invalid argument	Wrong method definition	Use direct struct init (no macros, no policy)
Connection reset by peer	Wrong message format	Use C client with length prefix
Permission denied on socket	Wrong permissions	sudo chmod 777 /tmp/greet_rpc.sock
Command failed: Not found	Bridge not running	Start bridge in Terminal 4
Cannot find -lubus	Library path not set	export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
socat: command not found	socat not installed	sudo apt install socat


WSL-Specific Issues
If you get file corruption errors in /mnt/d/:
bash
# Move project to Linux home directory
cd ~
mkdir -p projects
cp -r "/mnt/d/c backup/Desktop/NeSecure/ubus_to_rpc_bridge" ~/projects/
cd ~/projects/ubus_to_rpc_bridge
make clean && make


Quick Status Check
bash
make status
# Shows: ubusd✅, provider✅, RPC✅, bridge✅, socket✅


📚 WHAT I LEARNED
Test incrementally - Start with simplest possible case (object with no methods)

Don't trust macros blindly - They can hide the actual structure

Protocol matters - RPC needs proper framing (length prefix)

Order matters - Start ubusd → provider → rpc-server → bridge

Check logs thoroughly - "Invalid argument" was consistent clue

Work in Linux filesystem - Avoid WSL mounted drives for development




