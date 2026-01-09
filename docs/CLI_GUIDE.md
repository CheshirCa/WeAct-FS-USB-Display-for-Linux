# WeActCLI - Text Display Utility Guide

Complete guide for using `weactcli` - the command-line text display utility.

## 🎯 Overview

`weactcli` is a versatile utility for displaying text on WeAct Display. Perfect for:

- Status messages and notifications
- System monitoring output
- Log display
- Custom information dashboards
- Cyrillic text display

## 📖 Quick Reference

### Basic Syntax

```bash
weactcli -p PORT [OPTIONS] "text"
```

### Essential Options

| Option | Description | Example |
|--------|-------------|---------|
| `-p, --port` | Serial port (required) | `-p /dev/ttyACM0` |
| `-c, --color` | Text color | `-c red` |
| `-z, --size` | Font size (8/12/16) | `-z 16` |
| `-s, --scroll` | Scrolling text | `-s 30:u` |
| `--center` | Center text | `--center` |
| `--cls` | Clear screen only | `--cls` |
| `-v, --verbose` | Verbose output | `-v` |

## 🚀 Usage Examples

### Basic Text Display

```bash
# Simple text
weactcli -p /dev/ttyACM0 "Hello World"

# Russian/Cyrillic
weactcli -p /dev/ttyACM0 "Привет мир"

# Multi-line text
weactcli -p /dev/ttyACM0 "Line 1
Line 2
Line 3"
```

### Text Alignment

```bash
# Left-aligned (default)
weactcli -p /dev/ttyACM0 "Left text"

# Centered
weactcli -p /dev/ttyACM0 --center "Centered"

# Note: Right alignment not directly supported
# Use spaces: "     Right text"
```

### Font Sizes

```bash
# Small (8pt) - ~20 columns
weactcli -p /dev/ttyACM0 -z 8 "Small text"

# Medium (12pt) - ~13 columns (default)
weactcli -p /dev/ttyACM0 -z 12 "Medium text"

# Large (16pt) - ~10 columns
weactcli -p /dev/ttyACM0 -z 16 "Large text"
```

### Colors

```bash
# Available colors
weactcli -p /dev/ttyACM0 -c red "Error!"
weactcli -p /dev/ttyACM0 -c green "Success"
weactcli -p /dev/ttyACM0 -c blue "Info"
weactcli -p /dev/ttyACM0 -c yellow "Warning"
weactcli -p /dev/ttyACM0 -c cyan "Status"
weactcli -p /dev/ttyACM0 -c magenta "Debug"
weactcli -p /dev/ttyACM0 -c white "Normal"
weactcli -p /dev/ttyACM0 -c black "Hidden"
```

### Scrolling Text

```bash
# Scroll up at 30 px/s
weactcli -p /dev/ttyACM0 -s 30:u "Scrolling text..."

# Scroll down at 40 px/s
weactcli -p /dev/ttyACM0 -s 40:d "Scrolling down..."

# Fast scroll
weactcli -p /dev/ttyACM0 -s 60:u "Fast scroll!"

# Slow scroll
weactcli -p /dev/ttyACM0 -s 15:u "Slow scroll..."
```

### Combined Options

```bash
# Large centered green text
weactcli -p /dev/ttyACM0 -z 16 --center -c green "OK"

# Small red error message
weactcli -p /dev/ttyACM0 -z 8 -c red "Error: Failed"

# Scrolling status in yellow
weactcli -p /dev/ttyACM0 -s 25:u -c yellow "System starting..."
```

## 📥 Input Methods

### Command Line

```bash
# Direct text
weactcli -p /dev/ttyACM0 "Hello"

# Multiple arguments joined
weactcli -p /dev/ttyACM0 Hello World  # Shows "Hello World"
```

### Stdin (Pipe)

```bash
# From echo
echo "Hello World" | weactcli -p /dev/ttyACM0

# From command output
date | weactcli -p /dev/ttyACM0 --center

# From file
cat status.txt | weactcli -p /dev/ttyACM0 -c cyan

# From pipeline
ps aux | grep nginx | weactcli -p /dev/ttyACM0 -z 8
```

### File Input

```bash
# Read from file
weactcli -p /dev/ttyACM0 -f /var/log/syslog

# With options
weactcli -p /dev/ttyACM0 -f status.txt -c green --center
```

### Input Redirection

```bash
# Redirect file
weactcli -p /dev/ttyACM0 < input.txt

# With options
weactcli -p /dev/ttyACM0 -c blue < message.txt
```

## 💡 Practical Examples

### System Status

```bash
# Current time
date "+%H:%M:%S" | weactcli -p /dev/ttyACM0 --center -z 16

# IP address
hostname -I | weactcli -p /dev/ttyACM0 --center -c cyan

# Uptime
uptime -p | weactcli -p /dev/ttyACM0 -c green

# Disk usage
df -h / | tail -1 | awk '{print $5}' | \
  weactcli -p /dev/ttyACM0 --center -c yellow
```

### Monitoring

```bash
# CPU temperature (Raspberry Pi)
cat /sys/class/thermal/thermal_zone0/temp | \
  awk '{print $1/1000"°C"}' | \
  weactcli -p /dev/ttyACM0 --center -z 16 -c red

# Load average
uptime | awk -F'load average:' '{print "Load:"$2}' | \
  weactcli -p /dev/ttyACM0 -c cyan

# Memory usage
free -h | grep Mem | awk '{print "RAM: "$3"/"$2}' | \
  weactcli -p /dev/ttyACM0 -c green
```

### Notifications

```bash
# Build complete
weactcli -p /dev/ttyACM0 --center -c green -z 16 "Build OK"

# Error notification
weactcli -p /dev/ttyACM0 --center -c red -z 16 "ERROR!"

# Custom notification
notify() {
  weactcli -p /dev/ttyACM0 --center -c cyan "$1"
}
notify "Task complete"
```

### Service Status

```bash
# Check if service is running
if systemctl is-active --quiet nginx; then
  weactcli -p /dev/ttyACM0 -c green "nginx: UP"
else
  weactcli -p /dev/ttyACM0 -c red "nginx: DOWN"
fi

# Show service status
systemctl status nginx | head -3 | \
  weactcli -p /dev/ttyACM0 -z 8
```

## 🔄 Loops and Updates

### Clock Display

```bash
#!/bin/bash
while true; do
  date "+%H:%M:%S" | weactcli -p /dev/ttyACM0 --center -c cyan -z 16
  sleep 1
done
```

### Status Monitor

```bash
#!/bin/bash
while true; do
  (
    echo "Time: $(date +%H:%M)"
    echo "Load: $(uptime | awk -F'load average:' '{print $2}')"
    echo "IP: $(hostname -I | awk '{print $1}')"
  ) | weactcli -p /dev/ttyACM0 -c green
  sleep 5
done
```

### Scrolling Log

```bash
#!/bin/bash
tail -f /var/log/syslog | while read line; do
  echo "$line" | weactcli -p /dev/ttyACM0 -s 30:u -z 8
done
```

## 🎨 Display Control

### Clear Screen

```bash
# Clear display
weactcli -p /dev/ttyACM0 --cls

# Clear before showing text
weactcli -p /dev/ttyACM0 --cls
weactcli -p /dev/ttyACM0 "New text"
```

### Display Duration

```bash
# Show for 5 seconds
weactcli -p /dev/ttyACM0 "Message"
sleep 5
weactcli -p /dev/ttyACM0 --cls

# Function for timed display
show_timed() {
  weactcli -p /dev/ttyACM0 "$1"
  sleep $2
  weactcli -p /dev/ttyACM0 --cls
}
show_timed "Hello" 3
```

## 📝 Scripting Tips

### Functions

```bash
# Display function
display() {
  weactcli -p /dev/ttyACM0 "$@"
}

# Status function
status() {
  display --center -c green "$1"
}

# Error function
error() {
  display --center -c red -z 16 "$1"
}

# Usage
status "System OK"
error "Failed!"
```

### Aliases

```bash
# Add to ~/.bashrc
alias disp='weactcli -p /dev/ttyACM0'
alias dispc='weactcli -p /dev/ttyACM0 --center'
alias disperr='weactcli -p /dev/ttyACM0 -c red --center'

# Usage
disp "Hello"
dispc "Centered"
disperr "Error!"
```

### Variables

```bash
# Configuration
PORT="/dev/ttyACM0"
COLOR="cyan"
SIZE="12"

# Use variables
weactcli -p "$PORT" -c "$COLOR" -z "$SIZE" "Text"

# Or set defaults
WEACT_PORT="${WEACT_PORT:-/dev/ttyACM0}"
weactcli -p "$WEACT_PORT" "Text"
```

## 🌍 Cyrillic Text

### Basic Cyrillic

```bash
# Russian
weactcli -p /dev/ttyACM0 "Привет мир"
weactcli -p /dev/ttyACM0 "Здравствуйте!"

# Ukrainian
weactcli -p /dev/ttyACM0 "Привіт світ"

# Bulgarian
weactcli -p /dev/ttyACM0 "Здравей свят"
```

### Encoding

```bash
# Ensure UTF-8
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

# From file (must be UTF-8 encoded)
weactcli -p /dev/ttyACM0 -f cyrillic.txt

# Convert if needed
iconv -f CP1251 -t UTF-8 input.txt | weactcli -p /dev/ttyACM0
```

## ⚙️ Advanced Usage

### Error Handling

```bash
# Check if command succeeded
if weactcli -p /dev/ttyACM0 "Test"; then
  echo "Display OK"
else
  echo "Display failed"
fi

# Retry logic
try_display() {
  local attempts=3
  local delay=1
  
  for i in $(seq 1 $attempts); do
    if weactcli -p /dev/ttyACM0 "$1"; then
      return 0
    fi
    echo "Attempt $i failed, retrying..."
    sleep $delay
  done
  
  echo "All attempts failed"
  return 1
}
```

### Verbose Mode

```bash
# Show what's happening
weactcli -p /dev/ttyACM0 -v "Test"

# Debug connection issues
weactcli -p /dev/ttyACM0 -v --cls 2>&1 | tee debug.log
```

### Multiple Displays

```bash
# If you have multiple displays
display1() {
  weactcli -p /dev/ttyACM0 "$@"
}

display2() {
  weactcli -p /dev/ttyUSB0 "$@"
}

# Use different displays
display1 "Display 1"
display2 "Display 2"
```

## 🐛 Troubleshooting

### Common Issues

```bash
# Permission denied
sudo chmod 666 /dev/ttyACM0
# Or add user to dialout group

# Display not found
ls -l /dev/ttyACM* /dev/ttyUSB*

# Text not showing
weactcli -p /dev/ttyACM0 --cls  # Clear first
weactcli -p /dev/ttyACM0 -v "Test"  # Verbose mode

# Wrong encoding
export LC_ALL=en_US.UTF-8
```

See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for more details.

## 📚 See Also

- [README.md](../README.md) - Project overview
- [TERMINAL.md](TERMINAL.md) - Terminal emulator guide
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Problem solving
- [UTILS_GUIDE.md](UTILS_GUIDE.md) - Utility scripts

## 💡 Tips

1. **Start simple** - Test with basic text first
2. **Use verbose mode** - When debugging issues
3. **Clear before update** - For clean display
4. **Script everything** - Makes updates easier
5. **Check encoding** - UTF-8 for Cyrillic
6. **Size matters** - Smaller font = more text
7. **Test colors** - Find what looks best
8. **Add delays** - Between rapid updates

---

**Happy displaying!** 🎉
