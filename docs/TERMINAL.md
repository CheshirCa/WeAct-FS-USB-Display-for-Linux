# WeActTerm - Terminal Emulator Guide

Complete guide for using WeActTerm terminal emulator on WeAct Display.

## 🎯 What is WeActTerm?

WeActTerm is a **mini terminal emulator** that turns your WeAct Display into a functional text terminal. Perfect for:

- **Headless SBC debugging** (Raspberry Pi, Orange Pi, etc.)
- **Emergency access** without HDMI monitor
- **Quick diagnostics** and troubleshooting
- **Network configuration** on-site
- **Service status checking**

## 📐 Display Specifications

- **Resolution**: 160×80 pixels
- **Character display**: 20 columns × 10 rows
- **Font size**: 8pt (fixed for optimal readability)
- **Scrollback buffer**: 100 lines
- **Colors**: White text on black background

## 🚀 Quick Start

### Basic Usage

```bash
# Start terminal
weactterm -p /dev/ttyACM0

# With verbose output
weactterm -p /dev/ttyACM0 -v
```

Once started, you can type commands normally. The display shows a miniature terminal window.

### Example Session

```bash
$ weactterm -p /dev/ttyACM0
WeActTerm started. Press Ctrl+C to exit.
Display: 20x10 chars on /dev/ttyACM0

# Now type on your keyboard:
$ ip addr show
$ systemctl status ssh
$ dmesg | tail
$ ping -c 3 8.8.8.8
```

### Exiting

- **Ctrl+C** - Exit WeActTerm
- The shell process is terminated automatically

## 💡 Best Use Cases

### ✅ Perfect For

1. **Quick diagnostics**
   ```bash
   ip addr
   ip route
   ping 8.8.8.8
   systemctl status
   ```

2. **Service management**
   ```bash
   systemctl start nginx
   systemctl status nginx
   journalctl -u nginx -n 5
   ```

3. **System information**
   ```bash
   uptime
   free -h
   df -h
   hostname -I
   ```

4. **Network troubleshooting**
   ```bash
   ifconfig
   nmcli device status
   ss -tuln
   ```

5. **Log viewing**
   ```bash
   dmesg | tail
   tail /var/log/syslog
   journalctl -n 20
   ```

6. **File operations**
   ```bash
   ls -la
   cat config.txt
   pwd
   cd /etc
   ```

### ❌ Not Suitable For

1. **Text editors** (vim, nano, emacs)
   - Too small for editing
   - Better use SSH or real terminal

2. **Interactive TUI applications**
   - htop, mc, ncurses apps
   - Screen too small

3. **Long output commands**
   - Use output redirection: `command > file.txt`
   - Then view with: `cat file.txt`

4. **Complex pipelines**
   - Better to script them first

## 🔧 Technical Details

### Terminal Emulation

WeActTerm creates a **PTY (pseudo-terminal)** and spawns a shell. This means:

- ✅ Real terminal, not just output display
- ✅ Bash features work (tab completion, history, etc.)
- ✅ Environment variables preserved
- ✅ Standard input/output/error handling
- ⚠️ Limited ANSI escape sequence support

### Supported Features

**Keyboard Input:**
- All ASCII characters
- Enter, Backspace
- Ctrl+C (exit WeActTerm)
- Most control characters

**Display:**
- Text rendering with scrollback
- Auto line wrapping
- Cursor indicator (green rectangle)
- Scrolling when buffer fills

**Shell Features:**
- Command history (use arrow keys)
- Tab completion
- Variables and environment
- Pipes and redirection
- Background processes (with `&`)

### Limitations

**Screen Size:**
- Only 20×10 characters visible
- Scrollback helps but limited
- Commands with wide output get wrapped

**ANSI Sequences:**
- Basic support only
- Colors mostly ignored
- Complex cursor movements may not work

**Performance:**
- ~30 FPS display refresh
- Serial port at 115200 baud
- Slight delay on rapid output

## 🛠️ Command-Line Options

```bash
weactterm [OPTIONS]

OPTIONS:
  -p, --port PORT    Serial port for display (required)
                     Example: /dev/ttyACM0, /dev/ttyUSB0
  
  -v, --verbose      Enable verbose output
                     Shows debug information
  
  -h, --help         Display help message
```

### Examples

```bash
# Basic usage
weactterm -p /dev/ttyACM0

# Verbose mode (useful for debugging)
weactterm -p /dev/ttyACM0 -v

# Different port
weactterm -p /dev/ttyUSB0

# Show help
weactterm --help
```

## 📝 Usage Tips

### 1. Keep Commands Short

Good:
```bash
ip a
df -h
free -m
```

Bad:
```bash
systemctl list-units --type=service --state=running --all
```

### 2. Use Pipes Wisely

```bash
# Good - filter early
dmesg | tail -5

# Good - grep for specific
ps aux | grep nginx

# Bad - too much output
cat /var/log/syslog
```

### 3. Output Redirection

For long outputs:
```bash
# Redirect to file
dmesg > /tmp/dmesg.txt

# Then view in parts
head /tmp/dmesg.txt
tail /tmp/dmesg.txt
```

### 4. Use Aliases

Create shortcuts in ~/.bashrc:
```bash
alias checkip='hostname -I'
alias checknet='ping -c 1 8.8.8.8'
alias status='systemctl status'
```

### 5. Screen Clearing

If display gets messy:
```bash
clear
# or
Ctrl+L
```

## 🐛 Troubleshooting

### Display Shows Garbage

**Problem**: Random characters or corrupted display

**Solutions**:
```bash
# Exit and restart weactterm
Ctrl+C
weactterm -p /dev/ttyACM0

# Or clear display first
weactcli -p /dev/ttyACM0 --cls
weactterm -p /dev/ttyACM0
```

### Cannot Type

**Problem**: Keyboard input doesn't work

**Possible causes**:
1. Terminal not in raw mode (shouldn't happen)
2. PTY issue

**Solutions**:
```bash
# Restart weactterm
Ctrl+C
weactterm -p /dev/ttyACM0

# Check if terminal works
echo test
```

### Text Wrapping Issues

**Problem**: Lines wrap awkwardly

**This is normal** due to 20-character width. Solutions:
- Use shorter commands
- Redirect long output to file
- Use abbreviated options

### Cursor Not Visible

**Problem**: Can't see where you're typing

**Note**: Cursor is a green rectangle. If not visible:
- Try typing anyway (it may be working)
- Restart if necessary

### Connection Lost

**Problem**: "Display not connected" error

**Solutions**:
```bash
# Check display connection
ls -l /dev/ttyACM0

# Check if display is busy
sudo fuser /dev/ttyACM0

# Kill stuck processes
sudo fuser -k /dev/ttyACM0
```

## 🎓 Example Workflows

### Workflow 1: Network Diagnostics

```bash
# Start terminal
weactterm -p /dev/ttyACM0

# Check IP address
$ ip a

# Test connectivity
$ ping -c 3 8.8.8.8

# Check DNS
$ nslookup google.com

# View routes
$ ip route
```

### Workflow 2: Service Troubleshooting

```bash
# Start terminal
weactterm -p /dev/ttyACM0

# Check service status
$ systemctl status nginx

# View recent logs
$ journalctl -u nginx -n 10

# Restart if needed
$ sudo systemctl restart nginx

# Verify
$ systemctl status nginx
```

### Workflow 3: Disk Space Check

```bash
# Start terminal
weactterm -p /dev/ttyACM0

# Check disk usage
$ df -h

# Find large files
$ du -sh /var/*

# Check inodes
$ df -i
```

### Workflow 4: Initial SBC Setup

```bash
# Start terminal on fresh Raspberry Pi
weactterm -p /dev/ttyACM0

# Check current IP
$ hostname -I

# Set static IP (example)
$ sudo nmcli con mod "Wired connection 1" \
  ipv4.addresses 192.168.1.100/24

# Set gateway
$ sudo nmcli con mod "Wired connection 1" \
  ipv4.gateway 192.168.1.1

# Apply
$ sudo nmcli con up "Wired connection 1"

# Verify
$ ip a
```

## 🔍 Under The Hood

### How It Works

1. **PTY Creation**
   - WeActTerm creates a pseudo-terminal pair
   - Master side: WeActTerm controls
   - Slave side: Shell runs

2. **Shell Spawning**
   - Forks process
   - Executes /bin/bash (or $SHELL)
   - Sets TERM=vt100

3. **I/O Multiplexing**
   - Uses select() to monitor:
     - Keyboard input (stdin)
     - Shell output (PTY master)
   - Non-blocking I/O

4. **Rendering**
   - Maintains screen buffer (20×10 chars)
   - Processes ANSI sequences (basic)
   - Renders via FreeType to display
   - Updates at ~30 FPS

5. **Cleanup**
   - Ctrl+C triggers shutdown
   - Terminates shell process
   - Restores terminal settings
   - Closes display connection

### Technical Specifications

```c
#define TERM_FONT_SIZE 8        // Fixed 8pt font
#define TERM_COLS 20            // 20 characters wide
#define TERM_ROWS 10            // 10 lines tall
#define SCROLLBACK_LINES 100    // Buffer history

Display: 160×80 pixels
Character size: ~8×8 pixels
Update rate: ~30 FPS
Serial baud: 115200
```

## 🚀 Advanced Usage

### Running Scripts

```bash
# Create diagnostic script
cat > diag.sh << 'EOF'
#!/bin/bash
echo "=== System Diagnostic ==="
echo "IP: $(hostname -I)"
echo "Uptime: $(uptime -p)"
echo "Memory: $(free -h | grep Mem | awk '{print $3"/"$2}')"
echo "Disk: $(df -h / | tail -1 | awk '{print $5}')"
EOF

chmod +x diag.sh

# Run in weactterm
$ ./diag.sh
```

### Custom Prompt

For better visibility on small screen:

```bash
# Add to ~/.bashrc
if [ "$TERM" = "vt100" ]; then
    PS1='\$ '  # Minimal prompt
fi
```

### Monitoring Mode

```bash
# Create monitoring script
cat > monitor.sh << 'EOF'
#!/bin/bash
while true; do
    clear
    echo "IP: $(hostname -I)"
    echo "Time: $(date +%H:%M:%S)"
    echo "Load: $(uptime | awk -F'load average:' '{print $2}')"
    sleep 5
done
EOF

chmod +x monitor.sh

# Run in weactterm
$ ./monitor.sh
```

## 📊 Performance Considerations

### Display Update Rate

- Normal typing: Instant
- Burst output: ~30 FPS
- Serial bandwidth: 115200 baud = ~11 KB/s
- Frame size: 25.6 KB (full screen)

### CPU Usage

- Idle: <1% CPU
- Active typing: ~2-5% CPU
- Scrolling: ~5-10% CPU

Minimal impact on system performance.

## 🎯 Summary

WeActTerm transforms your WeAct Display into a functional mini terminal, perfect for headless SBC debugging. While limited by screen size, it's invaluable for quick diagnostics, network checks, and emergency access.

**Best for**: Quick commands, status checks, diagnostics
**Not for**: Text editing, complex TUI apps, long outputs

With WeActTerm, you can:
- ✅ Debug headless systems on-site
- ✅ Configure network without SSH
- ✅ Check service status quickly
- ✅ View logs and system info
- ✅ Run diagnostic commands

No HDMI monitor needed! 🎉

## 📚 See Also

- [README.md](../README.md) - Project overview
- [CLI_GUIDE.md](CLI_GUIDE.md) - Text display utility
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Common issues
- [API_REFERENCE.md](API_REFERENCE.md) - Library documentation
