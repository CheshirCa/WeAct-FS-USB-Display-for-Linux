# Makefile for WeActCLI
# Linux/C version of WeAct Display FS utility

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 $(shell pkg-config --cflags freetype2)
LDFLAGS = -lm $(shell pkg-config --libs freetype2)

# Source files
SOURCES = weactcli.c weact_display.c text_freetype.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = weactcli

# Library files
LIB_SOURCES = weact_display.c text_freetype.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
LIB_TARGET = libweact.a

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include

# Default target
all: check-freetype $(TARGET)

# Check for FreeType2
check-freetype:
	@pkg-config --exists freetype2 || (echo "Error: FreeType2 not found. Install with: sudo apt-get install libfreetype6-dev"; exit 1)

# Build executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"
	@echo "Usage: ./$(TARGET) -p /dev/ttyUSB0 \"Hello World\""

# Build static library
lib: $(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJECTS)
	ar rcs $(LIB_TARGET) $(LIB_OBJECTS)
	@echo "Library created: $(LIB_TARGET)"

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
weactcli.o: weactcli.c weact_display.h text_freetype.h
weact_display.o: weact_display.c weact_display.h
text_freetype.o: text_freetype.c text_freetype.h weact_display.h

# Install
install: $(TARGET)
	@echo "Installing $(TARGET) to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)
	@echo "Installation complete"
	@echo "Note: You may need to add your user to the 'dialout' group:"
	@echo "      sudo usermod -a -G dialout $$USER"
	@echo "Then logout and login again for changes to take effect"

# Install with library
install-all: $(TARGET) $(LIB_TARGET)
	@echo "Installing $(TARGET) and library..."
	install -d $(BINDIR)
	install -d $(LIBDIR)
	install -d $(INCDIR)
	install -m 755 $(TARGET) $(BINDIR)
	install -m 644 $(LIB_TARGET) $(LIBDIR)
	install -m 644 weact_display.h $(INCDIR)
	install -m 644 text_renderer.h $(INCDIR)
	@echo "Installation complete"

# Uninstall
uninstall:
	rm -f $(BINDIR)/$(TARGET)
	rm -f $(LIBDIR)/$(LIB_TARGET)
	rm -f $(INCDIR)/weact_display.h
	rm -f $(INCDIR)/text_renderer.h
	@echo "Uninstallation complete"

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET) $(LIB_TARGET)
	@echo "Clean complete"

# Test (requires device connected)
test: $(TARGET)
	@echo "Testing WeActCLI..."
	@if [ ! -e /dev/ttyUSB0 ] && [ ! -e /dev/ttyACM0 ]; then \
		echo "Error: No USB serial device found"; \
		echo "Please connect the WeAct Display and update the port"; \
		exit 1; \
	fi
	@PORT=/dev/ttyUSB0; \
	if [ ! -e $$PORT ]; then PORT=/dev/ttyACM0; fi; \
	echo "Using port: $$PORT"; \
	./$(TARGET) -p $$PORT --verbose "Test: Hello World!"

# Find available serial ports
detect-ports:
	@echo "Available serial ports:"
	@ls -1 /dev/ttyUSB* /dev/ttyACM* /dev/ttyS* 2>/dev/null || echo "No serial ports found"
	@echo ""
	@echo "Device permissions:"
	@ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -5 || echo "No USB devices"

# Help
help:
	@echo "WeActCLI Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all (default)   - Build weactcli executable"
	@echo "  lib             - Build static library (libweact.a)"
	@echo "  install         - Install weactcli to $(BINDIR)"
	@echo "  install-all     - Install weactcli and library"
	@echo "  uninstall       - Remove installed files"
	@echo "  clean           - Remove build artifacts"
	@echo "  test            - Run basic test (requires device)"
	@echo "  detect-ports    - List available serial ports"
	@echo "  help            - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build"
	@echo "  make install            # Install to /usr/local/bin"
	@echo "  sudo make install       # Install system-wide"
	@echo "  make PREFIX=~/.local install  # Install to user directory"
	@echo "  make test               # Test with connected device"
	@echo ""
	@echo "Usage after installation:"
	@echo "  weactcli -p /dev/ttyUSB0 \"Hello World\""
	@echo "  echo \"Test\" | weactcli -p /dev/ttyUSB0"
	@echo "  weactcli -p /dev/ttyUSB0 -f file.txt"
	@echo ""

.PHONY: all lib install install-all uninstall clean test detect-ports help
