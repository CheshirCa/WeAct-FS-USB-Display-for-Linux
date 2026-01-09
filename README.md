# WeActCLI v2.0.1

![WeAct 0.96" Display in Linux](https://github.com/CheshirCa/WeAct-FS-USB-Display-for-Linux/blob/main/demo.jpg)

## Console utility for WeAct Display FS 0.96-inch (160x80) USB display.

### What's New in v2.0.1
- ✅ **Fixed text clipping** - Text no longer cuts off at top of screen
- ✅ All font sizes (8pt, 12pt, 16pt) display correctly
- ✅ Proper font metrics calculation

### Features
- 🔤 **Real Cyrillic support** (not transliteration!)
- ✨ TrueType fonts (DejaVu Sans)
- 🌍 Full Unicode UTF-8
- 🎨 Anti-aliased rendering
- 📏 Variable font sizes (6-32pt)

## Quick Start

```bash
# Install
tar -xzf weactcli-linux-v2.0.1.tar.gz
cd weactcli-linux
./install.sh

# Test
weactcli -p /dev/ttyACM0 "Hello World"
weactcli -p /dev/ttyACM0 "Привет мир"
```

## Examples

```bash
# English
weactcli -p /dev/ttyACM0 "Hello"

# Russian
weactcli -p /dev/ttyACM0 "Привет"

# Centered
weactcli -p /dev/ttyACM0 --center "Test"

# Large font
weactcli -p /dev/ttyACM0 -z 16 "BIG"

# Colors
weactcli -p /dev/ttyACM0 -c green "OK"
weactcli -p /dev/ttyACM0 -c red "Error"
```

## Documentation

- **v2.0.1_NOTES.md** - What's fixed in this version
- **CYRILLIC_GUIDE.md** - Complete Cyrillic usage guide
- **CHANGELOG.md** - Full version history
- **INSTALL.md** - Installation instructions
- **QUICKSTART.md** - Quick start guide

## Requirements

- Debian 11-13 / Ubuntu 20.04+
- libfreetype6-dev (auto-installed by install.sh)

## Support

For issues, see v2.0.1_NOTES.md for troubleshooting.

---

**Version:** 2.0.1  
**Status:** ✅ STABLE  
**Released:** 2025-01-09
