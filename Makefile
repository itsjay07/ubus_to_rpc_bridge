CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include -I/usr/local/include -D_GNU_SOURCE
LDFLAGS = -L/usr/local/lib -ljson-c -lpthread -lubox -lubus
LIBRARY_PATH = /usr/local/lib

SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source files
UTILS_SRC = $(SRCDIR)/utils.c
PROVIDER_SRC = $(SRCDIR)/greet_ubus_provider.c
RPC_SERVER_SRC = $(SRCDIR)/rpc_server.c
BRIDGE_SRC = $(SRCDIR)/ubus_rpc_bridge.c

# Object files
UTILS_OBJ = $(OBJDIR)/utils.o
PROVIDER_OBJ = $(OBJDIR)/greet_ubus_provider.o
RPC_SERVER_OBJ = $(OBJDIR)/rpc_server.o
BRIDGE_OBJ = $(OBJDIR)/ubus_rpc_bridge.o

# Targets
TARGETS = $(BINDIR)/greet_ubus_provider $(BINDIR)/rpc_server $(BINDIR)/ubus_rpc_bridge

all: directories $(TARGETS)
	@echo "✅ Build complete!"
	@ls -la $(BINDIR)/

directories:
	@mkdir -p $(OBJDIR) $(BINDIR)
	@echo "📁 Directories created"

# Compile rules
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "🔨 Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "   ✅ Created $@"

# Link rules
$(BINDIR)/greet_ubus_provider: $(PROVIDER_OBJ) $(UTILS_OBJ)
	@echo "🔗 Linking greet_ubus_provider..."
	@$(CC) $^ -o $@ $(LDFLAGS)
	@echo "   ✅ Created $@"

$(BINDIR)/rpc_server: $(RPC_SERVER_OBJ) $(UTILS_OBJ)
	@echo "🔗 Linking rpc_server..."
	@$(CC) $^ -o $@ $(LDFLAGS)
	@echo "   ✅ Created $@"

$(BINDIR)/ubus_rpc_bridge: $(BRIDGE_OBJ) $(UTILS_OBJ)
	@echo "🔗 Linking ubus_rpc_bridge..."
	@$(CC) $^ -o $@ $(LDFLAGS)
	@echo "   ✅ Created $@"

clean:
	@echo "🧹 Cleaning..."
	@rm -rf $(OBJDIR) $(BINDIR) /tmp/greet_rpc.sock /tmp/bridge_listener.sock
	@echo "   ✅ Clean complete"

run-all:
	@echo "🚀 Starting all components..."
	@-sudo pkill ubusd 2>/dev/null || true
	@-pkill -f greet_ubus_provider 2>/dev/null || true
	@-pkill -f rpc_server 2>/dev/null || true
	@-pkill -f ubus_rpc_bridge 2>/dev/null || true
	@echo "1️⃣  Starting ubusd..."
	@sudo ubusd &
	@sleep 2
	@echo "2️⃣  Starting greet provider..."
	@$(BINDIR)/greet_ubus_provider &
	@sleep 1
	@echo "3️⃣  Starting RPC server..."
	@$(BINDIR)/rpc_server &
	@sleep 1
	@echo "4️⃣  Starting bridge..."
	@$(BINDIR)/ubus_rpc_bridge &
	@sleep 1
	@echo "✅ All components started!"
	@echo ""
	@echo "Run './test.sh' to test the bridge"

stop-all:
	@echo "🛑 Stopping all components..."
	@-sudo pkill ubusd 2>/dev/null || true
	@-pkill -f greet_ubus_provider 2>/dev/null || true
	@-pkill -f rpc_server 2>/dev/null || true
	@-pkill -f ubus_rpc_bridge 2>/dev/null || true
	@rm -f /tmp/greet_rpc.sock /tmp/bridge_listener.sock
	@echo "✅ All components stopped"

check:
	@echo "🔍 Checking ubus installation..."
	@which ubusd || echo "❌ ubusd not found"
	@ls -la /usr/local/lib/libubus* 2>/dev/null || echo "❌ libubus not found"
	@ls -la /usr/local/lib/libubox* 2>/dev/null || echo "❌ libubox not found"
	@echo ""
	@echo "📚 Library paths:"
	@echo "LD_LIBRARY_PATH=$(LD_LIBRARY_PATH)"
	@echo ""
	@echo "To fix library path, run: export LD_LIBRARY_PATH=/usr/local/lib:\$$LD_LIBRARY_PATH"

.PHONY: all clean directories run-all stop-all check
