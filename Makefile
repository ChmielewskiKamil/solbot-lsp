# Default goal to run when 'make' is invoked
.DEFAULT_GOAL := all

# Compiler and flags from the Nix environment
CC = clang
CFLAGS ?= $(NIX_CFLAGS_COMPILE)

# Output binaries
TARGET = solbot-lsp
TEST_RUNNER = test-runner

# Define all source and header files for the unity build
# This makes it easy to add new files later.
MAIN_SRC = main.c
TEST_SRC = test_runner.c

UNITY_C_FILES = json/lexer.c json/parser.c lsp/dispatcher.c
UNITY_H_FILES = json/lexer.h json/parser.h lsp/dispatcher.h libs/foundation.h

# A complete list of all dependencies for any build target
ALL_DEPS = $(UNITY_C_FILES) $(UNITY_H_FILES)

# Installation directory
DESTDIR ?=./result

# Use .PHONY for targets that are not files
.PHONY: all test clean install

# --- Build Targets ---

# Default build target
all: $(TARGET)

# Build the main LSP. It depends on its main source file AND all shared files.
$(TARGET): $(MAIN_SRC) $(ALL_DEPS)
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN_SRC)

# Build the test runner. It also depends on all shared files.
$(TEST_RUNNER): $(TEST_SRC) $(ALL_DEPS)
	$(CC) $(CFLAGS) -o $(TEST_RUNNER) $(TEST_SRC)

# --- Commands ---

# Run tests
test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

# Install the main binary. This is used by `nix build`.
install: $(TARGET)
	install -D $(TARGET) $(DESTDIR)/bin/$(TARGET)

# Clean up build artifacts
clean:
	$(RM) $(TARGET) $(TEST_RUNNER)
