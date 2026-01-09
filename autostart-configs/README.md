# WeActTerm Autostart Configuration Files

This directory contains configuration files for automatically starting weactterm when a USB display is connected.

## 🚀 Quick Setup (Recommended)

**Use the automatic setup script:**

```bash
cd autostart-configs/
./setup-autostart.sh
```

The script will:
1. Detect your USB-Serial chip automatically
2. Install the appropriate udev rule
3. Install systemd service
4. Configure everything for you

## 📁 Files in this directory

### udev rules (pick one for your chip):

- `99-weact-ch340.rules` - For CH340 chip (most common)
- `99-weact-ftdi.rules` - For FTDI FT232 chip
- `99-weact-cp2102.rules` - For CP2102 chip

### systemd service:

- `weactterm@.service` - Service template for weactterm

### Setup script:

- `setup-autostart.sh` - Automatic installation script

## 🔧 Manual Installation

If you prefer to install manually:

### Step 1: Identify your USB chip

```bash
lsusb | grep -i "serial\|CH340\|FTDI\|CP210"
```

### Step 2: Copy the appropriate udev rule

```bash
# For CH340 (most common):
sudo cp 99-weact-ch340.rules /etc/udev/rules.d/

# For FTDI:
sudo cp 99-weact-ftdi.rules /etc/udev/rules.d/

# For CP2102:
sudo cp 99-weact-cp2102.rules /etc/udev/rules.d/
```

### Step 3: Edit and copy systemd service

```bash
# Edit service file - change 'pi' to your username
nano weactterm@.service

# Copy to system
sudo cp weactterm@.service /etc/systemd/system/
```

### Step 4: Add yourself to dialout group

```bash
sudo usermod -aG dialout $USER
```

### Step 5: Reload and apply

```bash
sudo systemctl daemon-reload
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Step 6: Reboot (required!)

```bash
sudo reboot
```

## ✅ Testing

After reboot:

```bash
# Disconnect display
# Wait 5 seconds
# Connect display

# Check if service started:
systemctl status 'weactterm@*'

# View logs:
journalctl -u 'weactterm@*' -f
```

## 🎨 Customization

### Change font size

Edit `/etc/systemd/system/weactterm@.service`:

```ini
# Large font (recommended):
ExecStart=/usr/local/bin/weactterm -p /dev/%i -f mono -z 12

# Medium font:
ExecStart=/usr/local/bin/weactterm -p /dev/%i -f mono -z 10

# Small font (more text):
ExecStart=/usr/local/bin/weactterm -p /dev/%i -f mono -z 8
```

Then reload:
```bash
sudo systemctl daemon-reload
```

### Run custom program instead of shell

Create a script:

```bash
sudo nano /usr/local/bin/my-display-app.sh
```

Make it executable:

```bash
sudo chmod +x /usr/local/bin/my-display-app.sh
```

Edit service to use your script:

```ini
ExecStart=/usr/local/bin/my-display-app.sh
```

## 🐛 Troubleshooting

### Service doesn't start

```bash
# Check logs:
journalctl -u 'weactterm@*' -xe

# Common issues:
# 1. Wrong username in service file
# 2. User not in dialout group
# 3. weactterm not installed
```

### Display not detected

```bash
# Test udev rule:
sudo udevadm test /sys/class/tty/ttyUSB0

# Monitor udev events:
sudo udevadm monitor

# Then disconnect and reconnect display
```

### Wrong USB ID in udev rule

```bash
# Find correct USB ID:
udevadm info -a -n /dev/ttyUSB0 | grep -E 'idVendor|idProduct'

# Edit udev rule with correct values:
sudo nano /etc/udev/rules.d/99-weact-display.rules
```

## 📚 More Information

For detailed guides see:
- `../AUTOSTART_GUIDE.md` - Complete autostart guide
- `../AUTOSTART_QUICK.md` - Quick setup guide
- `../README.md` - Main project documentation

## 🎯 Common USB-Serial Chips

| Chip | VendorID:ProductID | Rule File |
|------|-------------------|-----------|
| CH340 | 1a86:7523 | 99-weact-ch340.rules |
| FTDI FT232 | 0403:6001 | 99-weact-ftdi.rules |
| CP2102 | 10c4:ea60 | 99-weact-cp2102.rules |

---

**Recommended:** Use `setup-autostart.sh` for automatic configuration!
