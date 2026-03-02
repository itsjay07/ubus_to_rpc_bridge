# ============================================================================
# 🚀 UBUS-RPC BRIDGE MAKEFILE - Professional Edition
# ============================================================================

# ████████╗ ██████╗  ██████╗ ██╗     ███████╗
# ╚══██╔══╝██╔═══██╗██╔═══██╗██║     ██╔════╝
#    ██║   ██║   ██║██║   ██║██║     ███████╗
#    ██║   ██║   ██║██║   ██║██║     ╚════██║
#    ██║   ╚██████╔╝╚██████╔╝███████╗███████║
#    ╚═╝    ╚═════╝  ╚═════╝ ╚══════╝╚══════╝
# ============================================================================

# ██╗   ██╗ █████╗ ██████╗ ██╗ █████╗ ██████╗ ██╗     ███████╗███████╗
# ██║   ██║██╔══██╗██╔══██╗██║██╔══██╗██╔══██╗██║     ██╔════╝██╔════╝
# ██║   ██║███████║██████╔╝██║███████║██████╔╝██║     █████╗  ███████╗
# ╚██╗ ██╔╝██╔══██║██╔══██╗██║██╔══██║██╔══██╗██║     ██╔══╝  ╚════██║
#  ╚████╔╝ ██║  ██║██║  ██║██║██║  ██║██████╔╝███████╗███████╗███████║
#   ╚══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝╚══════╝
# ============================================================================

# 🎨 Colors for beautiful output
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
RED = \033[0;31m
NC = \033[0m # No Color

# 🔧 Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./include -I/usr/local/include -D_GNU_SOURCE
LDFLAGS = -L/usr/local/lib -ljson-c -lpthread -lubox -lubus
LIBRARY_PATH = /usr/local/lib

# 📁 Directory structure
SRCDIR = src
OBJDIR = obj
BINDIR = bin
TESTDIR = tests

# 📝 Source files
UTILS_SRC = $(SRCDIR)/utils.c
PROVIDER_SRC = $(SRCDIR)/greet_ubus_provider.c
RPC_SERVER_SRC = $(SRCDIR)/rpc_server.c
BRIDGE_SRC = $(SRCDIR)/ubus_rpc_bridge.c

# 📦 Object files
UTILS_OBJ = $(OBJDIR)/utils.o
PROVIDER_OBJ = $(OBJDIR)/greet_ubus_provider.o
RPC_SERVER_OBJ = $(OBJDIR)/rpc_server.o
BRIDGE_OBJ = $(OBJDIR)/ubus_rpc_bridge.o

# 🎯 Targets
TARGETS = $(BINDIR)/greet_ubus_provider $(BINDIR)/rpc_server $(BINDIR)/ubus_rpc_bridge

# ============================================================================
# 🏗️  MAIN BUILD TARGETS
# ============================================================================

# Default target
all: banner directories $(TARGETS) size
	@printf "$(GREEN)╔════════════════════════════════════════════════════════════╗$(NC)\n"
	@printf "$(GREEN)║           ✅ BUILD COMPLETE - ALL SYSTEMS GO!            ║$(NC)\n"
	@printf "$(GREEN)╚════════════════════════════════════════════════════════════╝$(NC)\n"

# Banner
banner:
	@printf "$(PURPLE)╔════════════════════════════════════════════════════════════╗$(NC)\n"
	@printf "$(PURPLE)║$(YELLOW)            UBUS-RPC BRIDGE BUILDER v2.0                $(PURPLE)║$(NC)\n"
	@printf "$(PURPLE)║$(CYAN)            Building bidirectional bridge...               $(PURPLE)║$(NC)\n"
	@printf "$(PURPLE)╚════════════════════════════════════════════════════════════╝$(NC)\n"

# Create directories
directories:
	@mkdir -p $(OBJDIR) $(BINDIR) $(TESTDIR)
	@printf "$(BLUE)📁  Directories: $(GREEN)✅$(NC)\n"

# Show binary sizes
size:
	@printf "$(CYAN)📊  Binary sizes:$(NC)\n"
	@ls -lh $(BINDIR)/ | awk '{print "     $(YELLOW)" $$9 "$(NC): " $$5}'

# ============================================================================
# 🔨 COMPILATION RULES
# ============================================================================

# Generic compilation rule
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf "$(BLUE)🔨  Compiling $(YELLOW)$<$(NC)... "
	@$(CC) $(CFLAGS) -c $< -o $@ 2> $(OBJDIR)/$*.log || \
		(printf "$(RED)❌\n" && cat $(OBJDIR)/$*.log && exit 1)
	@printf "$(GREEN)✅$(NC)\n"

# ============================================================================
# 🔗 LINKING RULES
# ============================================================================

# Greet provider
$(BINDIR)/greet_ubus_provider: $(PROVIDER_OBJ) $(UTILS_OBJ)
	@printf "$(BLUE)🔗  Linking $(YELLOW)greet_ubus_provider$(NC)... "
	@$(CC) $^ -o $@ $(LDFLAGS) 2> $(OBJDIR)/provider_link.log || \
		(printf "$(RED)❌\n" && cat $(OBJDIR)/provider_link.log && exit 1)
	@printf "$(GREEN)✅$(NC)\n"

# RPC server
$(BINDIR)/rpc_server: $(RPC_SERVER_OBJ) $(UTILS_OBJ)
	@printf "$(BLUE)🔗  Linking $(YELLOW)rpc_server$(NC)... "
	@$(CC) $^ -o $@ $(LDFLAGS) 2> $(OBJDIR)/rpc_link.log || \
		(printf "$(RED)❌\n" && cat $(OBJDIR)/rpc_link.log && exit 1)
	@printf "$(GREEN)✅$(NC)\n"

# Bridge
$(BINDIR)/ubus_rpc_bridge: $(BRIDGE_OBJ) $(UTILS_OBJ)
	@printf "$(BLUE)🔗  Linking $(YELLOW)ubus_rpc_bridge$(NC)... "
	@$(CC) $^ -o $@ $(LDFLAGS) 2> $(OBJDIR)/bridge_link.log || \
		(printf "$(RED)❌\n" && cat $(OBJDIR)/bridge_link.log && exit 1)
	@printf "$(GREEN)✅$(NC)\n"

# ============================================================================
# 🧹 CLEANING TARGETS
# ============================================================================

# Clean everything
clean:
	@printf "$(YELLOW)🧹  Cleaning...$(NC)\n"
	@rm -rf $(OBJDIR) $(BINDIR) $(TESTDIR) /tmp/greet_rpc.sock /tmp/bridge_listener.sock *.log
	@printf "$(GREEN)   ✅ Clean complete$(NC)\n"

# Deep clean (removes everything including dependencies)
distclean: clean
	@printf "$(YELLOW)🗑️   Deep cleaning...$(NC)\n"
	@rm -rf $(SRCDIR)/*~ include/*~ *.bak *.backup
	@printf "$(GREEN)   ✅ Deep clean complete$(NC)\n"

# ============================================================================
# 🚀 RUNTIME TARGETS
# ============================================================================

# Run all components
run-all: check-deps
	@printf "$(CYAN)╔════════════════════════════════════════════════════════════╗$(NC)\n"
	@printf "$(CYAN)║$(YELLOW)              STARTING ALL COMPONENTS                   $(CYAN)║$(NC)\n"
	@printf "$(CYAN)╚════════════════════════════════════════════════════════════╝$(NC)\n"
	@-sudo pkill ubusd 2>/dev/null || true
	@-pkill -f greet_ubus_provider 2>/dev/null || true
	@-pkill -f rpc_server 2>/dev/null || true
	@-pkill -f ubus_rpc_bridge 2>/dev/null || true
	@printf "$(BLUE)1️⃣  Starting ubusd...$(NC) "
	@sudo ubusd > /dev/null 2>&1 &
	@sleep 2
	@pgrep -x ubusd > /dev/null && printf "$(GREEN)✅$(NC)\n" || printf "$(RED)❌$(NC)\n"
	@printf "$(BLUE)2️⃣  Starting greet provider...$(NC) "
	@$(BINDIR)/greet_ubus_provider > /dev/null 2>&1 &
	@sleep 1
	@pgrep -f greet_ubus_provider > /dev/null && printf "$(GREEN)✅$(NC)\n" || printf "$(RED)❌$(NC)\n"
	@printf "$(BLUE)3️⃣  Starting RPC server...$(NC) "
	@$(BINDIR)/rpc_server > /dev/null 2>&1 &
	@sleep 1
	@pgrep -f rpc_server > /dev/null && printf "$(GREEN)✅$(NC)\n" || printf "$(RED)❌$(NC)\n"
	@printf "$(BLUE)4️⃣  Starting bridge...$(NC) "
	@$(BINDIR)/ubus_rpc_bridge > /dev/null 2>&1 &
	@sleep 1
	@pgrep -f ubus_rpc_bridge > /dev/null && printf "$(GREEN)✅$(NC)\n" || printf "$(RED)❌$(NC)\n"
	@printf "$(GREEN)╔════════════════════════════════════════════════════════════╗$(NC)\n"
	@printf "$(GREEN)║           ✅ ALL COMPONENTS STARTED!                       ║$(NC)\n"
	@printf "$(GREEN)║                                                            ║$(NC)\n"
	@printf "$(GREEN)║  Run '$(YELLOW)make status$(GREEN)' to check status              ║$(NC)\n"
	@printf "$(GREEN)║  Run '$(YELLOW)make test$(GREEN)' to test the bridge             ║$(NC)\n"
	@printf "$(GREEN)╚════════════════════════════════════════════════════════════╝$(NC)\n"

# Stop all components
stop-all:
	@printf "$(RED)🛑  Stopping all components...$(NC)\n"
	@-sudo pkill ubusd 2>/dev/null && printf "   $(GREEN)✓ ubusd stopped$(NC)\n" || printf "   $(YELLOW)⚠ ubusd not running$(NC)\n"
	@-pkill -f greet_ubus_provider 2>/dev/null && printf "   $(GREEN)✓ provider stopped$(NC)\n" || printf "   $(YELLOW)⚠ provider not running$(NC)\n"
	@-pkill -f rpc_server 2>/dev/null && printf "   $(GREEN)✓ rpc_server stopped$(NC)\n" || printf "   $(YELLOW)⚠ rpc_server not running$(NC)\n"
	@-pkill -f ubus_rpc_bridge 2>/dev/null && printf "   $(GREEN)✓ bridge stopped$(NC)\n" || printf "   $(YELLOW)⚠ bridge not running$(NC)\n"
	@rm -f /tmp/greet_rpc.sock /tmp/bridge_listener.sock
	@printf "$(GREEN)   ✅ All components stopped$(NC)\n"

# Restart all components
restart: stop-all run-all

# ============================================================================
# 🔍 STATUS & DIAGNOSTICS
# ============================================================================

# Check status of all components
status:
	@printf "$(CYAN)📊  SYSTEM STATUS:$(NC)\n"
	@printf "   ubusd: "
	@pgrep -x ubusd > /dev/null && printf "$(GREEN)✅ running (PID: $$(pgrep -x ubusd))$(NC)\n" || printf "$(RED)❌ not running$(NC)\n"
	@printf "   greet provider: "
	@pgrep -f greet_ubus_provider > /dev/null && printf "$(GREEN)✅ running (PID: $$(pgrep -f greet_ubus_provider))$(NC)\n" || printf "$(RED)❌ not running$(NC)\n"
	@printf "   RPC server: "
	@pgrep -f rpc_server > /dev/null && printf "$(GREEN)✅ running (PID: $$(pgrep -f rpc_server))$(NC)\n" || printf "$(RED)❌ not running$(NC)\n"
	@printf "   bridge: "
	@pgrep -f ubus_rpc_bridge > /dev/null && printf "$(GREEN)✅ running (PID: $$(pgrep -f ubus_rpc_bridge))$(NC)\n" || printf "$(RED)❌ not running$(NC)\n"
	@printf "   socket: "
	@[ -S /tmp/greet_rpc.sock ] && printf "$(GREEN)✅ exists$(NC)\n" || printf "$(RED)❌ missing$(NC)\n"

# Check dependencies
check-deps:
	@printf "$(BLUE)🔍  Checking dependencies...$(NC)\n"
	@command -v ubusd > /dev/null || (printf "$(RED)   ❌ ubusd not found!$(NC)\n" && exit 1)
	@command -v socat > /dev/null || (printf "$(RED)   ❌ socat not found!$(NC)\n" && exit 1)
	@printf "$(GREEN)   ✅ All dependencies found$(NC)\n"

# Check ubus installation
check:
	@printf "$(PURPLE)╔════════════════════════════════════════════════════════════╗$(NC)\n"
	@printf "$(PURPLE)║$(YELLOW)              UBUS INSTALLATION CHECK                   $(PURPLE)║$(NC)\n"
	@printf "$(PURPLE)╚════════════════════════════════════════════════════════════╝$(NC)\n"
	@printf "$(BLUE)📌  ubusd path: $(GREEN)$$(which ubusd)$(NC)\n"
	@printf "$(BLUE)📌  ubus path: $(GREEN)$$(which ubus)$(NC)\n"
	@printf "$(BLUE)📌  libraries:$(NC)\n"
	@ls -la /usr/local/lib/libubus* 2>/dev/null | sed 's/^/     /' || printf "$(RED)     libubus not found$(NC)\n"
	@ls -la /usr/local/lib/libubox* 2>/dev/null | sed 's/^/     /' || printf "$(RED)     libubox not found$(NC)\n"
	@printf "$(BLUE)📌  ubusd status: "
	@pgrep -x ubusd > /dev/null && printf "$(GREEN)running$(NC)\n" || printf "$(RED)not running$(NC)\n"
	@printf "$(BLUE)📌  library path: $(YELLOW)$$LD_LIBRARY_PATH$(NC)\n"
	@printf "$(GREEN)   ✅ Check complete$(NC)\n"

# ============================================================================
# 🧪 TESTING TARGETS
# ============================================================================

# Test both directions
test:
	@printf "$(CYAN)╔════════════════════════════════════════════════════════════╗$(NC)\n"
	@printf "$(CYAN)║$(YELLOW)                 RUNNING TESTS                          $(CYAN)║$(NC)\n"
	@printf "$(CYAN)╚════════════════════════════════════════════════════════════╝$(NC)\n"
	@printf "$(BLUE)📌  Test 1 (ubus → RPC):$(NC)\n"
	@ubus call rpc_greet welcome '{"name":"Shripad"}' 2>&1 | sed 's/^/     /'
	@printf "\n$(BLUE)📌  Test 2 (RPC → ubus):$(NC)\n"
	@echo '{"id":1,"method":"greet.welcome","params":{"name":"Shripad"}}' | socat - UNIX-CONNECT:/tmp/greet_rpc.sock 2>&1 | sed 's/^/     /' || printf "$(RED)     Failed - is rpc_server running?$(NC)\n"

# Quick test (just ubus → RPC)
test-quick:
	@ubus call rpc_greet welcome '{"name":"Shripad"}'

# ============================================================================
# 📚 HELP TARGET
# ============================================================================

# Help
help:
	@printf "$(PURPLE)╔════════════════════════════════════════════════════════════╗$(NC)\n"
	@printf "$(PURPLE)║$(YELLOW)              UBUS-RPC BRIDGE MAKEFILE                 $(PURPLE)║$(NC)\n"
	@printf "$(PURPLE)║$(CYAN)                    Available commands                    $(PURPLE)║$(NC)\n"
	@printf "$(PURPLE)╚════════════════════════════════════════════════════════════╝$(NC)\n"
	@printf "$(GREEN)  make $(NC)               - Build everything\n"
	@printf "$(GREEN)  make clean$(NC)          - Remove compiled files\n"
	@printf "$(GREEN)  make distclean$(NC)      - Deep clean (removes backups too)\n"
	@printf "$(GREEN)  make run-all$(NC)        - Start all components\n"
	@printf "$(GREEN)  make stop-all$(NC)       - Stop all components\n"
	@printf "$(GREEN)  make restart$(NC)        - Restart all components\n"
	@printf "$(GREEN)  make status$(NC)         - Check component status\n"
	@printf "$(GREEN)  make test$(NC)           - Run both tests\n"
	@printf "$(GREEN)  make test-quick$(NC)     - Quick test (ubus → RPC)\n"
	@printf "$(GREEN)  make check$(NC)          - Check ubus installation\n"
	@printf "$(GREEN)  make check-deps$(NC)     - Check dependencies\n"
	@printf "$(GREEN)  make size$(NC)           - Show binary sizes\n"
	@printf "$(GREEN)  make help$(NC)           - Show this help\n"

# ============================================================================
# 🏁 PHONY TARGETS (not actual files)
# ============================================================================

.PHONY: all clean distclean run-all stop-all restart status test test-quick check check-deps size help banner directories