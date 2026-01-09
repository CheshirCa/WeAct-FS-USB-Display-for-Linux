# WeAct Display Tools

🖥️ **Complete toolkit for WeAct Display FS 0.96" (160×80) USB displays on Linux**

![WeAct 0.96" Display tool](https://github.com/CheshirCa/WeAct-FS-USB-Display-for-Linux/blob/main/demo.jpg) ![WeAct 0.96" Display Terminal](https://github.com/CheshirCa/WeAct-FS-USB-Display-for-Linux/blob/main/demo_console.jpg)

Perfect for Raspberry Pi, Orange Pi, and other headless SBC debugging and monitoring!

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Linux-blue.svg)](https://www.linux.org/)
[![Arch](https://img.shields.io/badge/arch-x86__64%20%7C%20ARM-green.svg)](https://github.com)

## 🌟 Features

### 🔤 Full Cyrillic Support
- Real Cyrillic text rendering (not transliteration!)
- TrueType fonts via FreeType2 (DejaVu Sans)
- Full Unicode UTF-8 support
- Anti-aliased rendering

### 🛠️ Three Powerful Tools

**1. `weactcli` - Text Display Utility**
- Display text, status, logs on the screen
- Multiple font sizes (8, 12, 16pt)
- Colors support
- Text scrolling and centering
- Pipe and file input support

**2. `weactterm` - Terminal Emulator** ⭐ NEW!
- Mini terminal for headless SBC debugging
- 20×10 character display
- PTY-based real terminal emulation
- Perfect for quick diagnostics without HDMI monitor
- Just plug display + keyboard into your SBC!

**3. `weact-utils` - Utility Scripts**
- System clock display
- Status monitoring
- Temperature monitoring (Raspberry Pi)
- Network info
- Countdown timer
- And more!

## 🚀 Quick Start

### Installation

```bash
# Clone repository
git clone https://github.com/yourusername/weact-display-tools.git
cd weact-display-tools

# Install dependencies (Debian/Ubuntu)
sudo apt-get update
sudo apt-get install build-essential libfreetype6-dev pkg-config

# Build and install
make
sudo make install

# Add user to dialout group (required for serial port access)
sudo usermod -aG dialout $USER
# Then logout and login again
```

### Usage Examples

#### Text Display (`weactcli`)

```bash
# Simple text
weactcli -p /dev/ttyACM0 "Hello World"

# Russian/Cyrillic
weactcli -p /dev/ttyACM0 "Привет мир"

# Centered with color
weactcli -p /dev/ttyACM0 --center -c green "Status: OK"

# Large font
weactcli -p /dev/ttyACM0 -z 16 "BIG TEXT"

# Scrolling
weactcli -p /dev/ttyACM0 -s 30:u "Long text that scrolls..."

# From pipe
echo "System ready" | weactcli -p /dev/ttyACM0 -c cyan

# From file
weactcli -p /dev/ttyACM0 -f /var/log/syslog
```

#### Terminal Emulator (`weactterm`) ⭐

```bash
# Start terminal on display
weactterm -p /dev/ttyACM0

# Now type commands:
# ip addr show
# systemctl status
# dmesg | tail
# ping 8.8.8.8
# etc.
```

Perfect for:
- Quick diagnostics on headless Raspberry Pi/Orange Pi
- Emergency access without HDMI monitor
- Checking IP address, services, logs
- Running simple commands

#### Utility Scripts (`weact-utils`)

```bash
# Digital clock
weact-utils clock

# System status
weact-utils status

# Network info
weact-utils network

# CPU temperature (Raspberry Pi)
weact-utils temperature

# 30 second countdown
weact-utils countdown /dev/ttyACM0 30

# Custom message
weact-utils message /dev/ttyACM0 "Hello!" green center

# See all commands
weact-utils help
```

## 📋 Requirements

### Hardware
- WeAct Display FS 0.96" (160×80) USB display
- Linux-based system (tested on Debian 11-13, Ubuntu 20.04+)
- USB port

### Software Dependencies
- GCC compiler
- Make
- libfreetype6-dev
- pkg-config

**Install dependencies:**
```bash
# Debian/Ubuntu
sudo apt-get install build-essential libfreetype6-dev pkg-config

# Arch Linux
sudo pacman -S base-devel freetype2 pkg-config

# Fedora/RHEL
sudo dnf install gcc make freetype-devel pkgconfig
```

## 🏗️ Building from Source

```bash
# Clone repository
git clone https://github.com/yourusername/weact-display-tools.git
cd weact-display-tools

# Build
make

# Local install (no sudo required)
PREFIX=~/.local make install
# Add ~/.local/bin to PATH if not already

# System-wide install
sudo make install
```

## 📖 Documentation

- **[TERMINAL.md](docs/TERMINAL.md)** - Complete guide for `weactterm` terminal emulator
- **[CLI_GUIDE.md](docs/CLI_GUIDE.md)** - Detailed `weactcli` usage and examples
- **[TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)** - Common issues and solutions

## 🎯 Use Cases

### For Headless SBC (Raspberry Pi, Orange Pi, etc.)
- **Emergency terminal access** without HDMI monitor
- Quick network configuration check
- Service status monitoring
- Log viewing on-the-go

### For Servers
- Status dashboard
- Real-time monitoring (CPU, memory, disk)
- Alert display
- Clock display

### For Projects
- IoT device status display
- Sensor data visualization
- System notifications
- Custom dashboards

## 🛠️ Library for Custom Applications

You can use `libweact.a` to build your own applications:

```c
#include <weact_display.h>
#include <text_freetype.h>

weact_display_t display;
weact_init(&display, "/dev/ttyACM0");

ft_text_context_t *text = ft_text_init(&display, NULL, 12);
ft_text_draw(text, 10, 10, "Hello World!");
weact_update_display(&display);

ft_text_cleanup(text);
weact_cleanup(&display);
```

See [API_REFERENCE.md](docs/API_REFERENCE.md) for complete documentation.

## 🐛 Troubleshooting

### Display not found
```bash
# List available ports
ls -l /dev/ttyUSB* /dev/ttyACM*

# Check USB devices
lsusb
```

### Permission denied
```bash
# Add user to dialout group
sudo usermod -aG dialout $USER

# Or temporary fix
sudo chmod 666 /dev/ttyACM0
```

### Text not displaying
- Check display connection
- Try different font sizes
- Verify text encoding (must be UTF-8)

See [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) for more solutions.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests.

### Development Setup
```bash
git clone https://github.com/yourusername/weact-display-tools.git
cd weact-display-tools
make
```

### Code Style
- Follow Linux kernel coding style
- Add comments for complex logic
- Test on multiple distributions

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## 📝 Version History

- **v2.1.0** (2025-01-09)
  - ⭐ Added `weactterm` terminal emulator
  - Complete toolkit for headless SBC debugging
  - Improved documentation

- **v2.0.1** (2025-01-09)
  - Fixed text clipping issues
  - Proper font metrics calculation
  - All font sizes work correctly

- **v2.0.0** (2024)
  - Full Cyrillic support via FreeType2
  - TrueType font rendering
  - Anti-aliased text

- **v1.0.0** (2024)
  - Initial Linux/C port
  - Basic text display
  - Color support

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Original WeAct Display protocol implementation based on work by CheshirCa.

## 🙏 Credits

- Original PureBasic implementation: CheshirCa
- Linux/C port and extensions: WeAct Display Tools Contributors
- FreeType2 library: The FreeType Project
- DejaVu fonts: DejaVu fonts team

## 📧 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/weact-display-tools/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/weact-display-tools/discussions)
- **Documentation**: [docs/](docs/)

## ⭐ Star History

If you find this project useful, please consider giving it a star!

---

**Made with ❤️ for the maker community**
