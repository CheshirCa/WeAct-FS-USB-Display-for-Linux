# Troubleshooting Guide

Common issues and solutions for WeAct Display Tools.

## 📋 Table of Contents

- [Installation Issues](#installation-issues)
- [Connection Issues](#connection-issues)
- [Display Issues](#display-issues)
- [Terminal Emulator Issues](#terminal-emulator-issues)
- [Performance Issues](#performance-issues)
- [Platform-Specific Issues](#platform-specific-issues)

## Installation Issues

### ❌ Error: "FreeType not found"

**Problem**: Missing FreeType development files

**Solution**:
```bash
# Debian/Ubuntu
sudo apt-get install libfreetype6-dev pkg-config

# Arch Linux
sudo pacman -S freetype2 pkg-config

# Fedora/RHEL
sudo dnf install freetype-devel pkgconfig
```

### ❌ Error: "make: command not found"

**Problem**: Build tools not installed

**Solution**:
```bash
# Debian/Ubuntu
sudo apt-get install build-essential

# Arch Linux
sudo pacman -S base-devel

# Fedora/RHEL
sudo dnf groupinstall "Development Tools"
```

### ❌ Error: "Permission denied" during installation

**Problem**: Installing to system directories without sudo

**Solution**:
```bash
# System-wide install (requires sudo)
sudo make install

# OR local install (no sudo)
PREFIX=~/.local make install
export PATH="$HOME/.local/bin:$PATH"
```

## Connection Issues

### ❌ Error: "Failed to open port /dev/ttyACM0"

**Problem**: Device not found or permission denied

**Diagnosis**:
```bash
# Check if device exists
ls -l /dev/ttyACM* /dev/ttyUSB*

# Check permissions
ls -l /dev/ttyACM0

# Check if device is busy
sudo fuser /dev/ttyACM0
```

**Solutions**:

**1. Device not found**:
```bash
# Unplug and replug display
# Check dmesg for errors
dmesg | tail -20

# List USB devices
lsusb
```

**2. Permission denied**:
```bash
# Temporary fix
sudo chmod 666 /dev/ttyACM0

# Permanent fix - add user to dialout group
sudo usermod -aG dialout $USER
# Then logout and login again
```

**3. Device busy**:
```bash
# Find process using device
sudo fuser /dev/ttyACM0

# Kill process
sudo fuser -k /dev/ttyACM0
```

### ❌ Display shows up as /dev/ttyUSB0 instead of /dev/ttyACM0

**Problem**: Different USB-serial chip

**Solution**: This is normal! Just use the correct port:
```bash
weactcli -p /dev/ttyUSB0 "Test"
weactterm -p /dev/ttyUSB0
```

### ❌ No serial ports found

**Problem**: USB drivers not loaded

**Solution**:
```bash
# Load USB serial drivers
sudo modprobe usbserial
sudo modprobe cdc-acm

# For CH340/CH341 chips (common on cheap displays)
sudo modprobe ch341

# Check if loaded
lsmod | grep usb
```

## Display Issues

### ❌ Display shows nothing

**Diagnosis**:
```bash
# Try clearing display first
weactcli -p /dev/ttyACM0 --cls

# Try simple test
weactcli -p /dev/ttyACM0 "TEST"

# Try with verbose mode
weactcli -p /dev/ttyACM0 -v "TEST"
```

**Possible causes**:

**1. Wrong port**:
```bash
# Try all available ports
for port in /dev/ttyUSB* /dev/ttyACM*; do
    echo "Testing $port"
    weactcli -p "$port" "TEST" 2>&1
done
```

**2. Display in wrong mode**:
```bash
# Reset display
weactcli -p /dev/ttyACM0 --cls
sleep 1
weactcli -p /dev/ttyACM0 "RESET"
```

**3. Cable issue**:
- Try different USB cable
- Check USB cable is data cable (not charge-only)

### ❌ Text is cut off or clipped

**Problem**: Text positioning issue

**Solutions**:

**1. Try different font size**:
```bash
weactcli -p /dev/ttyACM0 -z 8 "Text"   # Small
weactcli -p /dev/ttyACM0 -z 12 "Text"  # Medium
weactcli -p /dev/ttyACM0 -z 16 "Text"  # Large
```

**2. Use centered text**:
```bash
weactcli -p /dev/ttyACM0 --center "Text"
```

**3. Check text length**:
```bash
# Too long text gets wrapped
# Try shorter messages
```

### ❌ Cyrillic shows as boxes/garbage

**Problem**: Font doesn't support Cyrillic or encoding issue

**Solutions**:

**1. Check UTF-8 encoding**:
```bash
# Check terminal encoding
locale

# Should show UTF-8
# If not, set it:
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8
```

**2. Verify FreeType font**:
```bash
# Check if DejaVu Sans is installed
ls -l /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf

# If missing, install
sudo apt-get install fonts-dejavu-core
```

**3. Test Cyrillic**:
```bash
echo "Привет мир" | weactcli -p /dev/ttyACM0
```

### ❌ Colors don't work

**Problem**: Wrong color names

**Solution**: Use correct color names:
```bash
# Valid colors:
weactcli -p /dev/ttyACM0 -c red "Text"
weactcli -p /dev/ttyACM0 -c green "Text"
weactcli -p /dev/ttyACM0 -c blue "Text"
weactcli -p /dev/ttyACM0 -c white "Text"
weactcli -p /dev/ttyACM0 -c black "Text"
weactcli -p /dev/ttyACM0 -c yellow "Text"
weactcli -p /dev/ttyACM0 -c cyan "Text"
weactcli -p /dev/ttyACM0 -c magenta "Text"
```

## Terminal Emulator Issues

### ❌ weactterm: Cannot type anything

**Problem**: Terminal not in raw mode or PTY issue

**Solutions**:

**1. Restart weactterm**:
```bash
# Kill with Ctrl+C
# Start again
weactterm -p /dev/ttyACM0
```

**2. Check terminal settings**:
```bash
# Before starting weactterm, check:
stty -a

# Should show normal terminal settings
```

**3. Try with verbose mode**:
```bash
weactterm -p /dev/ttyACM0 -v
```

### ❌ weactterm: Display shows garbage

**Problem**: ANSI escape sequences not handled properly

**Solutions**:

**1. Clear and restart**:
```bash
weactcli -p /dev/ttyACM0 --cls
weactterm -p /dev/ttyACM0
```

**2. Use simple commands**:
- Avoid complex TUI applications
- Stick to basic commands
- Use `clear` command to reset

**3. Disable fancy shell prompts**:
```bash
# In shell, set simple prompt
export PS1='\$ '
```

### ❌ weactterm: Text wraps weirdly

**Problem**: Small screen (20 characters wide)

**This is normal!** Solutions:

**1. Use shorter commands**:
```bash
# Good
ip a

# Bad
systemctl list-units --type=service --all
```

**2. Use output redirection**:
```bash
# Send to file
long_command > /tmp/output.txt

# Then view in parts
head /tmp/output.txt
```

**3. Use aliases**:
```bash
# In ~/.bashrc
alias ll='ls -lh'
alias gs='git status'
```

### ❌ weactterm: Exits immediately

**Problem**: Shell exited or crashed

**Diagnosis**:
```bash
# Start with verbose mode
weactterm -p /dev/ttyACM0 -v

# Check what shell is being used
echo $SHELL

# Try forcing bash
SHELL=/bin/bash weactterm -p /dev/ttyACM0
```

**Solution**: Check system logs:
```bash
dmesg | tail
journalctl -n 50
```

## Performance Issues

### ❌ Slow display updates

**Problem**: Serial communication bottleneck or CPU overload

**Diagnosis**:
```bash
# Check system load
uptime
htop  # or top

# Monitor USB traffic
dmesg -w  # In another terminal while using display
```

**Solutions**:

**1. Reduce update frequency**:
- Don't run loops with constant updates
- Add delays between updates

**2. Use smaller text**:
```bash
# Smaller font = faster rendering
weactcli -p /dev/ttyACM0 -z 8 "Text"
```

**3. Optimize commands in weactterm**:
- Use `grep` to filter output
- Redirect long outputs to files
- Use `head` and `tail`

### ❌ High CPU usage

**Problem**: Constant display updates

**Solution**:
- Don't run continuous monitoring in tight loop
- Add sleep delays
- Use event-driven updates

Example fix:
```bash
# Bad (high CPU)
while true; do
    date | weactcli -p /dev/ttyACM0 --center
done

# Good (reasonable CPU)
while true; do
    date | weactcli -p /dev/ttyACM0 --center
    sleep 1
done
```

## Platform-Specific Issues

### Raspberry Pi Issues

#### ❌ Temperature monitoring doesn't work

**Problem**: Thermal zone not available

**Check**:
```bash
ls -l /sys/class/thermal/thermal_zone0/temp

# Should exist on Raspberry Pi
```

**Solution**:
- This is normal on non-RPi systems
- Use alternative monitoring commands

#### ❌ Display not recognized on Raspberry Pi Zero

**Problem**: USB OTG configuration

**Solution**:
```bash
# Enable USB host mode
# Add to /boot/config.txt:
dtoverlay=dwc2

# Add to /boot/cmdline.txt:
modules-load=dwc2,g_serial
```

### Orange Pi Issues

#### ❌ Permission issues despite being in dialout group

**Problem**: Permissions not updated

**Solution**:
```bash
# Force permission update
sudo udevadm control --reload-rules
sudo udevadm trigger

# Or reboot
sudo reboot
```

### Arch Linux Issues

#### ❌ Font not found

**Problem**: Different font path

**Solution**:
```bash
# Install DejaVu fonts
sudo pacman -S ttf-dejavu

# Verify installation
ls -l /usr/share/fonts/TTF/DejaVuSans.ttf
```

### Fedora/RHEL Issues

#### ❌ SELinux blocking serial port access

**Problem**: SELinux policy

**Diagnosis**:
```bash
sudo ausearch -m avc -ts recent | grep tty
```

**Solution**:
```bash
# Temporary: Set SELinux to permissive
sudo setenforce 0

# Permanent: Create policy or disable for serial
sudo setsebool -P allow_serial_port_access on
```

## General Debugging

### Enable Verbose Mode

Most tools support `-v` or `--verbose`:

```bash
weactcli -p /dev/ttyACM0 -v "Test"
weactterm -p /dev/ttyACM0 -v
```

### Check System Logs

```bash
# General logs
dmesg | tail -50

# USB device logs
dmesg | grep -i usb

# Serial logs
dmesg | grep -i tty

# System journal
journalctl -n 100 -f
```

### Test Display Connection

```bash
# Simple connectivity test
echo "test" > /dev/ttyACM0

# If error, connection is bad
# If no error, device is responding
```

### Memory Leaks

If you suspect memory issues:

```bash
# Install valgrind
sudo apt-get install valgrind

# Test for leaks
valgrind --leak-check=full ./weactcli -p /dev/ttyACM0 "Test"
```

## Still Having Issues?

### 1. Gather Information

```bash
# System info
uname -a
cat /etc/os-release

# Hardware info
lsusb
ls -l /dev/tty*

# Software versions
weactcli --version 2>&1 || weactcli --help | head -1
gcc --version
pkg-config --modversion freetype2
```

### 2. Create an Issue

Open an issue on GitHub with:
- Description of problem
- System information (from above)
- Commands that failed
- Error messages
- Steps to reproduce

### 3. Community Help

- Check [GitHub Discussions](https://github.com/yourusername/weact-display-tools/discussions)
- Search [closed issues](https://github.com/yourusername/weact-display-tools/issues?q=is%3Aissue+is%3Aclosed)

## Quick Reference

### Common Commands

```bash
# Test connection
weactcli -p /dev/ttyACM0 "TEST"

# Clear display
weactcli -p /dev/ttyACM0 --cls

# Check device
ls -l /dev/ttyACM0

# Check permissions
groups

# Add to dialout
sudo usermod -aG dialout $USER

# Start terminal
weactterm -p /dev/ttyACM0
```

### Emergency Reset

```bash
# If display is stuck:
1. Unplug USB
2. Wait 5 seconds
3. Plug back in
4. Clear display: weactcli -p /dev/ttyACM0 --cls
```

---

**Need More Help?**
- 📖 [README.md](../README.md) - Project documentation
- 🖥️ [TERMINAL.md](TERMINAL.md) - Terminal emulator guide
- 💬 [GitHub Issues](https://github.com/yourusername/weact-display-tools/issues)
