# QuickStart Guide

Get started with WeAct Display Tools in 5 minutes!

## 📦 Installation

```bash
# 1. Clone repository
git clone https://github.com/yourusername/weact-display-tools.git
cd weact-display-tools

# 2. Install dependencies (Debian/Ubuntu)
sudo apt-get update
sudo apt-get install build-essential libfreetype6-dev pkg-config

# 3. Build and install
make
sudo make install

# 4. Add user to dialout group
sudo usermod -aG dialout $USER
# Then logout and login again
```

## 🚀 First Test

```bash
# Find your display
ls -l /dev/ttyACM0  # or /dev/ttyUSB0

# Test text display
weactcli -p /dev/ttyACM0 "Hello World!"

# Test with color
weactcli -p /dev/ttyACM0 --center -c green "Success!"

# Test terminal
weactterm -p /dev/ttyACM0
# Type: ip addr show
# Press Ctrl+C to exit
```

## 📖 Common Commands

```bash
# Display text
weactcli -p /dev/ttyACM0 "Your text here"

# Clear screen
weactcli -p /dev/ttyACM0 --cls

# Start terminal
weactterm -p /dev/ttyACM0

# Show clock
weact-utils clock

# Show system status
weact-utils status
```

## 🐛 Problems?

```bash
# Permission denied?
sudo usermod -aG dialout $USER  # Then logout/login

# Can't find device?
ls -l /dev/ttyACM* /dev/ttyUSB*  # Find your device

# Text not showing?
weactcli -p /dev/ttyACM0 --cls   # Clear first
weactcli -p /dev/ttyACM0 -v "Test"  # Verbose mode
```

## 📚 Full Documentation

- [README.md](README.md) - Complete overview
- [docs/CLI_GUIDE.md](docs/CLI_GUIDE.md) - Text display guide
- [docs/TERMINAL.md](docs/TERMINAL.md) - Terminal emulator guide
- [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) - Solve problems

## 🎯 What's Next?

1. **Learn more**: Read [CLI_GUIDE.md](docs/CLI_GUIDE.md)
2. **Try terminal**: See [TERMINAL.md](docs/TERMINAL.md)
3. **Explore utilities**: Run `weact-utils help`
4. **Build custom app**: Check [API_REFERENCE.md](docs/API_REFERENCE.md)

---

**Ready to go!** 🎉
