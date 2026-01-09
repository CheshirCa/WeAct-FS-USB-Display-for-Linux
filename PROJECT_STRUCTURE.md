# WeActCLI for Linux - Project Structure

Complete C/Linux port of WeActCLI from Windows/PureBasic version.

## 📁 Project Structure

```
weactcli/
│
├── 📄 Core Library Files
│   ├── weact_display.h          # Display library header
│   ├── weact_display.c          # Display library implementation
│   ├── text_renderer.h          # Text rendering header
│   └── text_renderer.c          # Text rendering with bitmap font
│
├── 🎯 Main Application
│   └── weactcli.c               # CLI utility main program
│
├── 📚 Examples & Utilities
│   ├── example.c                # Demo program (clock, graphics, etc.)
│   └── weact-utils.sh           # Bash utility scripts
│
├── 🔧 Build & Installation
│   ├── Makefile                 # Build system
│   └── install.sh               # Automated installation script
│
└── 📖 Documentation
    ├── README.md                # Full documentation (Russian)
    ├── INSTALL.md               # Installation guide
    ├── QUICKSTART.md            # Quick start guide
    └── PROJECT_STRUCTURE.md    # This file
```

## 🔨 Build System

### Quick Commands

```bash
# Build
make

# Build and install
make && sudo make install

# Build library only
make lib

# Clean
make clean

# Test
make test

# Detect ports
make detect-ports

# Help
make help
```

### Build Targets

- `all` (default) - Build weactcli executable
- `lib` - Build static library (libweact.a)
- `install` - Install weactcli to /usr/local/bin
- `install-all` - Install everything (binary, library, headers)
- `uninstall` - Remove installed files
- `clean` - Remove build artifacts
- `test` - Run basic test
- `detect-ports` - List available serial ports
- `help` - Show help

## 📦 Components

### 1. Core Library (weact_display.c/h)

**Purpose:** Low-level display communication

**Features:**
- Serial port initialization (termios)
- Display protocol implementation (v1.1)
- Buffer management (double buffering)
- Graphics primitives (pixel, line, rect, circle)
- Display control (orientation, brightness, fill)
- Color conversion (RGB888 to BRG565)

**Key Functions:**
```c
weact_init()              // Initialize display
weact_clear_buffer()      // Clear buffer
weact_draw_pixel()        // Draw pixel
weact_draw_line()         // Draw line
weact_draw_rect()         // Draw rectangle
weact_draw_circle()       // Draw circle
weact_update_display()    // Update display
weact_set_orientation()   // Set orientation
weact_set_brightness()    // Set brightness
weact_fill_screen()       // Fill with color (FULL command)
```

### 2. Text Renderer (text_renderer.c/h)

**Purpose:** Text rendering with bitmap fonts

**Features:**
- Embedded 6x8 bitmap font (ASCII 32-126)
- Multiple font sizes (small, medium, large)
- Text alignment (left, center, right)
- Word wrapping
- Color support

**Key Functions:**
```c
text_init()               // Initialize text context
text_set_font_size()      // Set font size
text_set_color()          // Set text color
text_draw()               // Draw text
text_draw_wrapped()       // Draw with word wrap
text_draw_centered()      // Draw centered
```

### 3. CLI Application (weactcli.c)

**Purpose:** Command-line interface

**Features:**
- Command-line argument parsing (getopt)
- Multiple input sources (file, stdin, pipe)
- Text scrolling animation
- Color support
- Font size selection
- Text centering
- Verbose mode

**Usage:**
```bash
weactcli -p /dev/ttyUSB0 [options] "text"
```

### 4. Example Program (example.c)

**Purpose:** Demonstration and testing

**Demos:**
- Clock display
- Progress bar
- Graphics (shapes)
- Scrolling text
- Orientation test
- System information

**Compile & Run:**
```bash
gcc example.c weact_display.c text_renderer.c -o example -lm
./example /dev/ttyUSB0
```

### 5. Utility Scripts (weact-utils.sh)

**Purpose:** Convenience scripts

**Commands:**
- `clock` - Digital clock
- `status` - System status
- `network` - Network info
- `temperature` - CPU temperature (RPi)
- `countdown` - Timer
- `monitor` - Log monitoring
- `message` - Custom message
- `resources` - System resources
- `clear` - Clear display
- `detect` - Detect ports
- `test` - Quick test

## 🔌 Protocol Implementation

Based on WeAct Studio Display Communication Protocol v1.1

### Supported Commands

| Command | Code | Description |
|---------|------|-------------|
| SET_ORIENTATION | 0x02 | Set display orientation (0-3, 5) |
| SET_BRIGHTNESS | 0x03 | Set brightness (0-255) with fade |
| FULL | 0x04 | Fill screen with color |
| SET_BITMAP | 0x05 | Send bitmap data |
| SYSTEM_RESET | 0x40 | Reset display |

### Display Specifications

- **Resolution:** 160x80 pixels
- **Color Format:** BRG565 (Blue-Red-Green)
- **Baud Rate:** 115200
- **Interface:** Serial (UART)
- **Buffer Size:** 25600 bytes (160×80×2)

### Orientations

- `0` - Portrait (80x160)
- `1` - Reverse Portrait (80x160)
- `2` - Landscape (160x80)
- `3` - Reverse Landscape (160x80)
- `5` - Auto-rotation mode

## 🎨 Color System

### Predefined Colors (BRG565)

```c
WEACT_RED      0x07C0
WEACT_GREEN    0x001F
WEACT_BLUE     0xF800
WEACT_WHITE    0xFFFF
WEACT_BLACK    0x0000
WEACT_YELLOW   0x07FF
WEACT_CYAN     0xF81F
WEACT_MAGENTA  0xFFE0
```

### Custom Colors

```c
uint16_t color = weact_rgb_to_brg565(r, g, b);
```

## 🔧 Dependencies

### Build Time
- gcc (>= 4.8)
- make
- build-essential

### Runtime
- Linux kernel with serial support
- /dev/ttyUSB* or /dev/ttyACM* device
- dialout group membership

### No External Libraries
- ✅ No FreeType
- ✅ No SDL
- ✅ No X11
- ✅ Pure C with standard library only

## 📱 Platform Support

### Tested On
- Debian 11 (Bullseye)
- Debian 12 (Bookworm)
- Debian 13 (Trixie)
- Ubuntu 20.04+
- Raspberry Pi OS

### Requirements
- Linux kernel 4.x+
- GCC 4.8+
- POSIX-compliant system

## 🔍 Code Quality

### Features
- ✅ Memory safe (no leaks)
- ✅ Error handling
- ✅ Buffer overflow protection
- ✅ Clean compilation (-Wall -Wextra)
- ✅ Consistent code style
- ✅ Well documented
- ✅ Modular design

### Performance
- Double buffering for smooth animation
- Optimized drawing functions
- Minimal memory footprint (~26KB buffer)
- Fast serial communication (115200 baud)

## 🚀 Usage Examples

### Basic Usage

```bash
# Simple text
weactcli -p /dev/ttyUSB0 "Hello"

# With options
weactcli -p /dev/ttyUSB0 -c green --center "OK"

# From file
weactcli -p /dev/ttyUSB0 -f message.txt

# From pipe
echo "Test" | weactcli -p /dev/ttyUSB0

# Scrolling
weactcli -p /dev/ttyUSB0 -s 30:u "Scroll up..."
```

### Programming

```c
#include "weact_display.h"
#include "text_renderer.h"

weact_display_t display;
weact_init(&display, "/dev/ttyUSB0");

weact_clear_buffer(&display, WEACT_BLACK);
weact_draw_rect(&display, 10, 10, 50, 30, WEACT_RED, true);

text_context_t ctx;
text_init(&ctx, &display);
text_draw(&ctx, 10, 50, "Hello!");

weact_update_display(&display);
weact_cleanup(&display);
```

## 📊 Comparison with Original

### Windows/PureBasic Version
- Uses Windows GDI for text rendering
- TrueType font support
- COM port via WinAPI
- PureBasic compiler

### Linux/C Version
- Embedded bitmap font
- termios for serial port
- GCC compiler
- Fully portable

### Shared Features
- Same protocol (v1.1)
- Same display support
- Double buffering
- All drawing functions
- Scrolling
- Color support

## 🔐 Security

### Permissions
- Requires dialout group membership
- No root required after setup
- Safe serial port handling
- Input validation

### Best Practices
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Verify permissions
ls -l /dev/ttyUSB0

# Test without sudo
weactcli -p /dev/ttyUSB0 "Test"
```

## 🐛 Debugging

### Verbose Mode
```bash
weactcli -p /dev/ttyUSB0 --verbose "Test"
```

### Check Device
```bash
# List devices
ls -l /dev/tty{USB,ACM}*

# Check driver
dmesg | grep tty

# USB info
lsusb
```

### Test Connection
```bash
# Clear screen test
weactcli -p /dev/ttyUSB0 --cls

# Simple message
weactcli -p /dev/ttyUSB0 "Test"
```

## 📝 License

Open source project. Free to use in your projects.

Based on original WeActDisplay library by CheshirCa.

## 🔗 Links

- Original PureBasic version: https://github.com/CheshirCa/WeActDisplay
- Protocol: WeAct Studio Display Communication Protocol v1.1
- WeAct Studio: Official manufacturer

## 📧 Support

For issues:
1. Check INSTALL.md troubleshooting
2. Run with `--verbose`
3. Check `dmesg | tail`
4. Verify port permissions
5. Test with example program

---

**Version:** 1.0  
**Date:** January 2025  
**Author:** C/Linux port based on PureBasic original  
**License:** Open Source  
**Compatibility:** Debian 11-13, Linux 4.x+
