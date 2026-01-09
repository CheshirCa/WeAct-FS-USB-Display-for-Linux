# Makefile for WeAct Display Tools
# Supports: weactcli, weactterm, and library

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 $(shell pkg-config --cflags freetype2)
LDFLAGS = $(shell pkg-config --libs freetype2) -lutil
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include

# Source files
LIB_SRC = weact_display.c text_freetype.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB_TARGET = libweact.a

CLI_SRC = weactcli.c
CLI_TARGET = weactcli

TERM_SRC = weactterm.c
TERM_TARGET = weactterm

HEADERS = weact_display.h text_freetype.h

# Targets
.PHONY: all clean install uninstall help

all: $(CLI_TARGET) $(TERM_TARGET) $(LIB_TARGET)

# Build library
$(LIB_TARGET): $(LIB_OBJ)
	ar rcs $@ $^
	@echo "Library built: $@"

# Build weactcli
$(CLI_TARGET): $(CLI_SRC) $(LIB_TARGET)
	$(CC) $(CFLAGS) -o $@ $(CLI_SRC) $(LIB_TARGET) $(LDFLAGS)
	@echo "Built: $@"

# Build weactterm
$(TERM_TARGET): $(TERM_SRC) $(LIB_TARGET)
	$(CC) $(CFLAGS) -o $@ $(TERM_SRC) $(LIB_TARGET) $(LDFLAGS)
	@echo "Built: $@"

# Compile object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Install
install: all
	@echo "Installing to $(PREFIX)..."
	install -d $(BINDIR)
	install -d $(LIBDIR)
	install -d $(INCDIR)
	install -m 755 $(CLI_TARGET) $(BINDIR)/
	install -m 755 $(TERM_TARGET) $(BINDIR)/
	install -m 755 weact-utils.sh $(BINDIR)/weact-utils
	install -m 644 $(LIB_TARGET) $(LIBDIR)/
	install -m 644 $(HEADERS) $(INCDIR)/
	@echo "Installation complete!"
	@echo ""
	@echo "Installed binaries:"
	@echo "  $(BINDIR)/$(CLI_TARGET)"
	@echo "  $(BINDIR)/$(TERM_TARGET)"
	@echo "  $(BINDIR)/weact-utils"
	@echo ""
	@echo "Next steps:"
	@echo "  1. Add user to dialout group: sudo usermod -aG dialout \$$USER"
	@echo "  2. Logout and login again"
	@echo "  3. Connect display and run: $(CLI_TARGET) -p /dev/ttyACM0 \"Hello\""
	@echo "  4. Or start terminal: $(TERM_TARGET) -p /dev/ttyACM0"

# Uninstall
uninstall:
	@echo "Uninstalling from $(PREFIX)..."
	rm -f $(BINDIR)/$(CLI_TARGET)
	rm -f $(BINDIR)/$(TERM_TARGET)
	rm -f $(BINDIR)/weact-utils
	rm -f $(LIBDIR)/$(LIB_TARGET)
	rm -f $(INCDIR)/weact_display.h
	rm -f $(INCDIR)/text_freetype.h
	@echo "Uninstallation complete"

# Clean build artifacts
clean:
	rm -f $(LIB_OBJ) $(LIB_TARGET) $(CLI_TARGET) $(TERM_TARGET)
	@echo "Clean complete"

# Help
help:
	@echo "WeAct Display Tools - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build all targets"
	@echo "  make install      - Install to system (requires sudo for /usr/local)"
	@echo "  make uninstall    - Remove from system"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make help         - Show this help"
	@echo ""
	@echo "Local install (no sudo):"
	@echo "  PREFIX=~/.local make install"
	@echo ""
	@echo "Components:"
	@echo "  weactcli   - Command-line text display utility"
	@echo "  weactterm  - Terminal emulator for headless SBC"
	@echo "  libweact.a - Static library for custom applications"
	@echo ""
	@echo "Dependencies:"
	@echo "  - gcc"
	@echo "  - make"
	@echo "  - libfreetype6-dev"
	@echo "  - pkg-config"
	@echo ""
	@echo "Install dependencies (Debian/Ubuntu):"
	@echo "  sudo apt-get install build-essential libfreetype6-dev pkg-config"
