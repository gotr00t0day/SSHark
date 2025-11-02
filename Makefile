# SSHark Makefile
# Author: c0d3Ninja
# Version: 1.0

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++20 -pthread -Wall -Wextra -O2
DEBUG_FLAGS = -g -DDEBUG

# Directories
SRC_DIR = .
MODULE_DIR = ./modules
OBJ_DIR = ./obj
BIN_DIR = ./bin

# Target binary
TARGET = $(BIN_DIR)/sshark

# Source files
MAIN_SRC = $(SRC_DIR)/sshkeyspread.cpp
MODULE_SRCS = $(MODULE_DIR)/parsers.cpp \
              $(MODULE_DIR)/readfile.cpp \
              $(MODULE_DIR)/executils.cpp \
              $(MODULE_DIR)/portscanner.cpp

# Object files
MAIN_OBJ = $(OBJ_DIR)/sshkeyspread.o
MODULE_OBJS = $(OBJ_DIR)/parsers.o \
              $(OBJ_DIR)/readfile.o \
              $(OBJ_DIR)/executils.o \
              $(OBJ_DIR)/portscanner.o

ALL_OBJS = $(MAIN_OBJ) $(MODULE_OBJS)

# Colors for output
GREEN = \033[0;32m
CYAN = \033[0;36m
YELLOW = \033[0;33m
RED = \033[0;31m
RESET = \033[0m

# Default target
all: banner directories $(TARGET)
	@echo "$(GREEN)✓ Build complete!$(RESET)"
	@echo "$(CYAN)Run with: $(TARGET) --help$(RESET)"

# Banner
banner:
	@echo "$(CYAN)"
	@echo "  ██████   ██████  ██░ ██  █████  ██████  ██   ██ "
	@echo "▒██    ▒ ▒██    ▒ ▓██░ ██▒▒██   ▀ ▒██   ██▒██  ██ "
	@echo "░ ▓██▄   ░ ▓██▄   ▒██▀▀██░░███   ░██  ████████   "
	@echo "  ▒   ██▒  ▒   ██▒░▓█ ░██ ░▓█▒  ░██  ░██ ▒██ ██  "
	@echo "▒██████▒▒▒██████▒▒░▓█▒░██▓░██████▒██  ░██ ▒██ ██  "
	@echo "$(RESET)"
	@echo "$(YELLOW)Building SSHark v1.0...$(RESET)\n"

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Link object files into final binary
$(TARGET): $(ALL_OBJS)
	@echo "$(CYAN)Linking $@...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(ALL_OBJS) -o $(TARGET)

# Compile main source file
$(OBJ_DIR)/sshkeyspread.o: $(MAIN_SRC)
	@echo "$(GREEN)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile module source files
$(OBJ_DIR)/parsers.o: $(MODULE_DIR)/parsers.cpp $(MODULE_DIR)/parsers.h
	@echo "$(GREEN)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/readfile.o: $(MODULE_DIR)/readfile.cpp $(MODULE_DIR)/readfile.h
	@echo "$(GREEN)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/executils.o: $(MODULE_DIR)/executils.cpp $(MODULE_DIR)/executils.h
	@echo "$(GREEN)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/portscanner.o: $(MODULE_DIR)/portscanner.cpp $(MODULE_DIR)/portscanner.h
	@echo "$(GREEN)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean all
	@echo "$(YELLOW)Debug build complete!$(RESET)"

# Clean build artifacts
clean:
	@echo "$(RED)Cleaning build artifacts...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@rm -rf $(BIN_DIR)
	@echo "$(GREEN)✓ Clean complete!$(RESET)"

# Install to system (requires sudo)
install: all
	@echo "$(CYAN)Installing SSHark to /usr/local/bin...$(RESET)"
	@sudo cp $(TARGET) /usr/local/bin/sshark
	@sudo chmod +x /usr/local/bin/sshark
	@echo "$(GREEN)✓ Installation complete!$(RESET)"
	@echo "$(CYAN)You can now run: sshark --help$(RESET)"

# Uninstall from system
uninstall:
	@echo "$(RED)Uninstalling SSHark...$(RESET)"
	@sudo rm -f /usr/local/bin/sshark
	@echo "$(GREEN)✓ Uninstall complete!$(RESET)"

# Run the program
run: all
	@echo "$(CYAN)Running SSHark...$(RESET)\n"
	@$(TARGET) --help

# Rebuild everything
rebuild: clean all

# Help target
help:
	@echo "$(CYAN)SSHark Makefile Targets:$(RESET)"
	@echo "  $(GREEN)make$(RESET)          - Build SSHark (default)"
	@echo "  $(GREEN)make debug$(RESET)    - Build with debug symbols"
	@echo "  $(GREEN)make clean$(RESET)    - Remove build artifacts"
	@echo "  $(GREEN)make rebuild$(RESET)  - Clean and rebuild"
	@echo "  $(GREEN)make install$(RESET)  - Install to /usr/local/bin (requires sudo)"
	@echo "  $(GREEN)make uninstall$(RESET) - Remove from system"
	@echo "  $(GREEN)make run$(RESET)      - Build and run SSHark"
	@echo "  $(GREEN)make help$(RESET)     - Show this help message"

# Phony targets
.PHONY: all banner directories debug clean install uninstall run rebuild help

