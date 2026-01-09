# WeActCLI - Quick Start Guide

Ultra-fast guide to get started with WeActCLI in 5 minutes.

## ⚡ 1-Minute Installation

```bash
./install.sh
```

That's it! The script will:
- Check dependencies
- Build the project
- Install to `/usr/local/bin`
- Setup permissions

## ⚡ First Test

```bash
# Find your device
ls -l /dev/ttyUSB* /dev/ttyACM*

# Test display (replace port with yours)
weactcli -p /dev/ttyUSB0 "Hello World!"
```

## ⚡ Most Used Commands

```bash
# Simple text
weactcli -p /dev/ttyUSB0 "Your text"

# With color
weactcli -p /dev/ttyUSB0 -c green "Status: OK"

# Centered
weactcli -p /dev/ttyUSB0 --center "Welcome"

# Scrolling
weactcli -p /dev/ttyUSB0 -s 30:u "Scrolling..."

# From pipe
echo "Test" | weactcli -p /dev/ttyUSB0

# Clear screen
weactcli -p /dev/ttyUSB0 --cls
```

## ⚡ Utilities

```bash
# Clock
weact-utils clock

# System status
weact-utils status

# Network info
weact-utils network

# Countdown timer
weact-utils countdown /dev/ttyUSB0 30
```

## ⚠️ Common Issues

### "Permission denied"
```bash
sudo usermod -a -G dialout $USER
# Then logout and login
```

### "Device not found"
```bash
# List devices
ls -l /dev/ttyUSB* /dev/ttyACM*

# Or use auto-detect
weact-utils detect
```

### "Command not found"
```bash
# Add to PATH
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## 📖 Full Documentation

- `README.md` - Complete documentation
- `INSTALL.md` - Detailed installation guide
- `weactcli --help` - Command-line help
- `weact-utils help` - Utilities help

## 🎯 What's Next?

1. **Try utilities:** `weact-utils help`
2. **Read examples:** `cat README.md`
3. **Write your own:** Check `example.c`
4. **Create scripts:** Use in bash scripts
5. **Monitor system:** Real-time display updates

---

That's all you need to get started! 🚀

For more details, see `README.md` and `INSTALL.md`.
