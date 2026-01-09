# Changelog

All notable changes to WeAct Display Tools project.

## [2.3.0] - 2025-01-09

### WeActCLI - Added
- ✨ Font type selection with `-t / --font` option (mono/sans/serif)
- ✨ Display rotation with `-r / --rotate` option (0-3 orientations)
- 📚 Extended help with examples for new options
- 🔧 Verbose mode shows font type and orientation

### WeActCLI - Changed
- 📈 Improved font path detection (searches multiple locations)
- 📈 Better help documentation with practical examples
- 📈 Font type now explicitly set (was using default before)

### WeActCLI - Fixed
- 🐛 Font initialization now uses explicit path instead of NULL
- 🐛 Orientation changes now properly update display dimensions

### WeActTerm - No changes
- Remains at v2.2.0 with all features

---

## [2.2.0] - 2025-01-09

### WeActTerm - Added
- ✨ Monospace font support (DejaVu Sans Mono) - now default!
- ✨ Font type selection: mono/sans/serif via `-f` option
- ✨ Scroll mode with Page Up/Down keys
- ✨ Scrollback buffer (100 lines)
- ✨ Scroll mode indicator (yellow square)
- ✨ Press 'q' to exit scroll mode

### Infrastructure - Added
- 📦 Autostart configuration files in `autostart-configs/`
- 🔧 Automatic installer script `setup-autostart.sh`
- 📚 Comprehensive autostart guides

---

**Latest Version:** 2.3.0
**Release Date:** 2025-01-09
**Status:** Production Ready 🟢
