#!/bin/bash
# WeActCLI Installation Script for Debian/Linux
# Supports Debian 11, 12, 13 and Ubuntu-based systems

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Installation directories
PREFIX="${PREFIX:-/usr/local}"
BINDIR="$PREFIX/bin"
LIBDIR="$PREFIX/lib"
INCDIR="$PREFIX/include"

# Check if running as root when needed
check_root() {
    if [ "$EUID" -ne 0 ] && [ "$PREFIX" = "/usr/local" ]; then
        echo -e "${YELLOW}Note: System-wide installation requires sudo${NC}"
        echo "Either run with sudo, or set PREFIX for local install:"
        echo "  PREFIX=~/.local $0"
        echo ""
        read -p "Continue with sudo? [y/N] " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
        exec sudo -E "$0" "$@"
    fi
}

# Print header
print_header() {
    echo -e "${BLUE}â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—${NC}"
    echo -e "${BLUE}â•‘   WeActCLI Installation Script            â•‘${NC}"
    echo -e "${BLUE}â•‘   Linux/C version for Debian 11-13        â•‘${NC}"
    echo -e "${BLUE}â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•${NC}"
    echo ""
}

# Check dependencies
check_dependencies() {
    echo -e "${YELLOW}Checking dependencies...${NC}"
    
    local missing=""
    
    # Check for GCC
    if ! command -v gcc &> /dev/null; then
        missing="$missing gcc"
    fi
    
    # Check for make
    if ! command -v make &> /dev/null; then
        missing="$missing make"
    fi
    
    # Check for basic build tools
    if ! dpkg -l | grep -q build-essential 2>/dev/null; then
        missing="$missing build-essential"
    fi
    
    # Check for FreeType2 development files
    if ! pkg-config --exists freetype2 2>/dev/null; then
        missing="$missing libfreetype6-dev"
    fi
    
    if [ -n "$missing" ]; then
        echo -e "${RED}Missing dependencies:$missing${NC}"
        echo ""
        echo "Install with:"
        echo "  sudo apt-get update"
        echo "  sudo apt-get install$missing"
        echo ""
        read -p "Install dependencies now? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "Installing dependencies..."
            sudo apt-get update
            sudo apt-get install -y $missing
            echo -e "${GREEN}Dependencies installed${NC}"
        else
            exit 1
        fi
    else
        echo -e "${GREEN}All dependencies satisfied${NC}"
    fi
    echo ""
}

# Build project
build_project() {
    echo -e "${YELLOW}Building WeActCLI...${NC}"
    
    if [ ! -f "Makefile" ]; then
        echo -e "${RED}Error: Makefile not found${NC}"
        echo "Make sure you're in the correct directory"
        exit 1
    fi
    
    # Clean previous builds
    make clean 2>/dev/null || true
    
    # Build
    if make; then
        echo -e "${GREEN}Build successful${NC}"
    else
        echo -e "${RED}Build failed${NC}"
        exit 1
    fi
    echo ""
}

# Install files
install_files() {
    echo -e "${YELLOW}Installing files to $PREFIX...${NC}"
    
    # Create directories
    mkdir -p "$BINDIR"
    mkdir -p "$LIBDIR"
    mkdir -p "$INCDIR"
    
    # Install binary
    if [ -f "weactcli" ]; then
        install -m 755 weactcli "$BINDIR/"
        echo "Installed: $BINDIR/weactcli"
    else
        echo -e "${RED}Error: weactcli binary not found${NC}"
        exit 1
    fi
    
    # Install library if exists
    if [ -f "libweact.a" ]; then
        install -m 644 libweact.a "$LIBDIR/"
        echo "Installed: $LIBDIR/libweact.a"
    fi
    
    # Install headers
    for header in weact_display.h text_renderer.h; do
        if [ -f "$header" ]; then
            install -m 644 "$header" "$INCDIR/"
            echo "Installed: $INCDIR/$header"
        fi
    done
    
    # Install utility script
    if [ -f "weact-utils.sh" ]; then
        install -m 755 weact-utils.sh "$BINDIR/weact-utils"
        echo "Installed: $BINDIR/weact-utils"
    fi
    
    echo -e "${GREEN}Installation complete${NC}"
    echo ""
}

# Setup permissions
setup_permissions() {
    echo -e "${YELLOW}Setting up serial port permissions...${NC}"
    
    # Check if user is in dialout group
    if ! groups | grep -q dialout; then
        echo "Adding user $USER to dialout group..."
        sudo usermod -a -G dialout "$USER"
        echo -e "${GREEN}User added to dialout group${NC}"
        echo -e "${YELLOW}âš  You must logout and login again for changes to take effect${NC}"
    else
        echo "User already in dialout group"
    fi
    echo ""
}

# Verify installation
verify_installation() {
    echo -e "${YELLOW}Verifying installation...${NC}"
    
    if command -v weactcli &> /dev/null; then
        echo -e "${GREEN}âœ“ weactcli command available${NC}"
        
        # Check version/help
        if weactcli --help &> /dev/null; then
            echo -e "${GREEN}âœ“ weactcli is working${NC}"
        else
            echo -e "${YELLOW}âš  weactcli installed but may have issues${NC}"
        fi
    else
        echo -e "${RED}âœ— weactcli command not found${NC}"
        echo "Make sure $BINDIR is in your PATH"
        
        # Check if PATH update is needed
        if [[ ":$PATH:" != *":$BINDIR:"* ]]; then
            echo ""
            echo "Add to PATH by adding this line to ~/.bashrc:"
            echo "  export PATH=\"$BINDIR:\$PATH\""
        fi
    fi
    
    echo ""
}

# Post-installation info
print_post_install() {
    echo -e "${BLUE}â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—${NC}"
    echo -e "${BLUE}â•‘   Installation Complete!                  â•‘${NC}"
    echo -e "${BLUE}â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•${NC}"
    echo ""
    echo -e "${GREEN}What's next:${NC}"
    echo ""
    echo "1. Connect your WeAct Display via USB"
    echo ""
    echo "2. Find your device:"
    echo "   ls -l /dev/ttyUSB* /dev/ttyACM*"
    echo ""
    echo "3. Test the display:"
    echo "   weactcli -p /dev/ttyUSB0 \"Hello World!\""
    echo ""
    echo "4. Explore utilities:"
    echo "   weact-utils help"
    echo ""
    echo "5. View full documentation:"
    echo "   cat README.md"
    echo ""
    
    if ! groups | grep -q dialout; then
        echo -e "${YELLOW}âš  IMPORTANT: Logout and login again to apply group changes${NC}"
        echo ""
    fi
    
    echo -e "${GREEN}Example commands:${NC}"
    echo "  weactcli -p /dev/ttyUSB0 --center -c green \"Status: OK\""
    echo "  echo \"Test\" | weactcli -p /dev/ttyUSB0"
    echo "  weact-utils clock"
    echo "  weact-utils status"
    echo ""
    echo "For help:"
    echo "  weactcli --help"
    echo "  weact-utils help"
    echo ""
}

# Uninstall function
uninstall() {
    echo -e "${YELLOW}Uninstalling WeActCLI...${NC}"
    
    rm -f "$BINDIR/weactcli"
    rm -f "$BINDIR/weact-utils"
    rm -f "$LIBDIR/libweact.a"
    rm -f "$INCDIR/weact_display.h"
    rm -f "$INCDIR/text_renderer.h"
    
    echo -e "${GREEN}Uninstallation complete${NC}"
}

# Main installation flow
main() {
    # Parse arguments
    if [ "$1" = "uninstall" ] || [ "$1" = "--uninstall" ]; then
        check_root
        uninstall
        exit 0
    fi
    
    if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
        echo "Usage: $0 [uninstall]"
        echo ""
        echo "Installation script for WeActCLI"
        echo ""
        echo "Options:"
        echo "  uninstall    Remove installed files"
        echo "  --help       Show this help"
        echo ""
        echo "Environment variables:"
        echo "  PREFIX       Installation prefix (default: /usr/local)"
        echo "               Example: PREFIX=~/.local $0"
        echo ""
        exit 0
    fi
    
    print_header
    
    # Check if we need root
    if [ "$PREFIX" = "/usr/local" ]; then
        check_root
    fi
    
    # Installation steps
    check_dependencies
    build_project
    install_files
    setup_permissions
    verify_installation
    print_post_install
}

# Run main function
main "$@"
