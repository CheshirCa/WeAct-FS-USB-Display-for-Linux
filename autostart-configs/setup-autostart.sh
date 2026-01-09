#!/bin/bash
# WeActTerm Autostart Installation Script
# Automatically configures weactterm to start when USB display is connected

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔═══════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   WeActTerm Autostart Setup                  ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════╝${NC}"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo -e "${RED}Error: Do not run this script as root${NC}"
    echo "Run as normal user: ./setup-autostart.sh"
    exit 1
fi

# Check if weactterm is installed
if ! command -v weactterm &> /dev/null; then
    echo -e "${RED}Error: weactterm not found${NC}"
    echo "Please install weactterm first:"
    echo "  cd /path/to/weact-display-tools"
    echo "  make && sudo make install"
    exit 1
fi

echo -e "${GREEN}✓ weactterm found: $(which weactterm)${NC}"
echo ""

# Detect USB device
echo -e "${YELLOW}Step 1: Detecting USB-Serial device...${NC}"
echo "Please make sure your WeAct Display is connected"
echo ""

sleep 2

# List USB devices
DEVICES=$(lsusb | grep -iE 'CH340|FTDI|CP210|serial')

if [ -z "$DEVICES" ]; then
    echo -e "${YELLOW}Warning: No common USB-serial devices found${NC}"
    echo "Available USB devices:"
    lsusb
    echo ""
    echo "Please identify your device manually and edit the udev rules"
    read -p "Continue anyway? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    echo "Found USB-Serial device(s):"
    echo "$DEVICES"
    echo ""
fi

# Detect chip type
VENDOR=""
PRODUCT=""
CHIP_TYPE=""

if echo "$DEVICES" | grep -q "1a86:7523"; then
    VENDOR="1a86"
    PRODUCT="7523"
    CHIP_TYPE="ch340"
    echo -e "${GREEN}Detected: CH340 chip${NC}"
elif echo "$DEVICES" | grep -q "0403:6001"; then
    VENDOR="0403"
    PRODUCT="6001"
    CHIP_TYPE="ftdi"
    echo -e "${GREEN}Detected: FTDI chip${NC}"
elif echo "$DEVICES" | grep -q "10c4:ea60"; then
    VENDOR="10c4"
    PRODUCT="ea60"
    CHIP_TYPE="cp2102"
    echo -e "${GREEN}Detected: CP2102 chip${NC}"
else
    echo -e "${YELLOW}Warning: Unknown chip type${NC}"
    echo "You'll need to manually configure udev rules"
    echo ""
    read -p "Enter Vendor ID (e.g., 1a86): " VENDOR
    read -p "Enter Product ID (e.g., 7523): " PRODUCT
    CHIP_TYPE="custom"
fi

echo ""

# Check group membership
echo -e "${YELLOW}Step 2: Checking permissions...${NC}"

if ! groups | grep -q dialout; then
    echo -e "${YELLOW}Adding user $USER to dialout group...${NC}"
    sudo usermod -aG dialout "$USER"
    echo -e "${GREEN}✓ User added to dialout group${NC}"
    echo -e "${YELLOW}⚠  You must logout and login again for this to take effect!${NC}"
    echo ""
else
    echo -e "${GREEN}✓ User already in dialout group${NC}"
    echo ""
fi

# Install udev rule
echo -e "${YELLOW}Step 3: Installing udev rule...${NC}"

UDEV_FILE="/etc/udev/rules.d/99-weact-display.rules"

if [ -f "$UDEV_FILE" ]; then
    echo -e "${YELLOW}Warning: $UDEV_FILE already exists${NC}"
    read -p "Overwrite? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Skipping udev rule installation"
    else
        # Create udev rule
        sudo tee "$UDEV_FILE" > /dev/null <<EOF
# WeAct Display - Auto-generated udev rule
# Chip: $CHIP_TYPE
# VendorID:ProductID = $VENDOR:$PRODUCT

ACTION=="add", SUBSYSTEM=="tty", ATTRS{idVendor}=="$VENDOR", ATTRS{idProduct}=="$PRODUCT", \\
    TAG+="systemd", ENV{SYSTEMD_WANTS}="weactterm@%k.service", \\
    SYMLINK+="weact-display", MODE="0666", \\
    ENV{ID_MM_DEVICE_IGNORE}="1"
EOF
        echo -e "${GREEN}✓ udev rule installed${NC}"
    fi
else
    # Create udev rule
    sudo tee "$UDEV_FILE" > /dev/null <<EOF
# WeAct Display - Auto-generated udev rule
# Chip: $CHIP_TYPE
# VendorID:ProductID = $VENDOR:$PRODUCT

ACTION=="add", SUBSYSTEM=="tty", ATTRS{idVendor}=="$VENDOR", ATTRS{idProduct}=="$PRODUCT", \\
    TAG+="systemd", ENV{SYSTEMD_WANTS}="weactterm@%k.service", \\
    SYMLINK+="weact-display", MODE="0666", \\
    ENV{ID_MM_DEVICE_IGNORE}="1"
EOF
    echo -e "${GREEN}✓ udev rule installed${NC}"
fi

echo ""

# Install systemd service
echo -e "${YELLOW}Step 4: Installing systemd service...${NC}"

SERVICE_FILE="/etc/systemd/system/weactterm@.service"

if [ -f "$SERVICE_FILE" ]; then
    echo -e "${YELLOW}Warning: $SERVICE_FILE already exists${NC}"
    read -p "Overwrite? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Skipping service installation"
    else
        # Create service file
        sudo tee "$SERVICE_FILE" > /dev/null <<EOF
[Unit]
Description=WeActTerm Terminal on %I
Documentation=https://github.com/yourusername/weact-display-tools
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/local/bin/weactterm -p /dev/%i -f mono -z 12
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal
User=$USER
Group=dialout
Environment="TERM=linux"
Environment="HOME=$HOME"

[Install]
WantedBy=multi-user.target
EOF
        echo -e "${GREEN}✓ systemd service installed${NC}"
    fi
else
    # Create service file
    sudo tee "$SERVICE_FILE" > /dev/null <<EOF
[Unit]
Description=WeActTerm Terminal on %I
Documentation=https://github.com/yourusername/weact-display-tools
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/local/bin/weactterm -p /dev/%i -f mono -z 12
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal
User=$USER
Group=dialout
Environment="TERM=linux"
Environment="HOME=$HOME"

[Install]
WantedBy=multi-user.target
EOF
    echo -e "${GREEN}✓ systemd service installed${NC}"
fi

echo ""

# Reload systemd and udev
echo -e "${YELLOW}Step 5: Applying configuration...${NC}"

sudo systemctl daemon-reload
echo -e "${GREEN}✓ systemd reloaded${NC}"

sudo udevadm control --reload-rules
sudo udevadm trigger
echo -e "${GREEN}✓ udev rules reloaded${NC}"

echo ""

# Summary
echo -e "${BLUE}╔═══════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   Installation Complete!                     ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════╝${NC}"
echo ""

echo -e "${GREEN}Configuration:${NC}"
echo "  Chip: $CHIP_TYPE"
echo "  USB ID: $VENDOR:$PRODUCT"
echo "  udev rule: $UDEV_FILE"
echo "  systemd service: $SERVICE_FILE"
echo ""

echo -e "${YELLOW}Next steps:${NC}"
echo ""

if ! groups | grep -q dialout; then
    echo -e "${RED}IMPORTANT: You MUST logout and login again!${NC}"
    echo "This is required for group membership to take effect."
    echo ""
fi

echo "1. Disconnect your WeAct Display (if connected)"
echo "2. Wait 5 seconds"
echo "3. Connect your WeAct Display"
echo "4. weactterm should start automatically!"
echo ""

echo -e "${YELLOW}Testing:${NC}"
echo ""
echo "Check if service is running:"
echo "  systemctl status 'weactterm@*'"
echo ""
echo "View logs:"
echo "  journalctl -u 'weactterm@*' -f"
echo ""

echo -e "${GREEN}Enjoy your auto-starting WeActTerm! 🎉${NC}"
echo ""
