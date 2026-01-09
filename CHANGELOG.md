# Changelog - WeActCLI

## v2.0.1 (2025-01-09) - HOTFIX ✅

### Fixed
- ✅ **Text clipping at top:** Fixed issue where text was cut off at the top
  - Small fonts (8pt): slight clipping → **FIXED**
  - Large fonts (16pt): half of text cut off → **FIXED**
  
### Technical Details

**Problem:** FreeType uses baseline coordinates, not top-of-text coordinates.
When we specified Y=5, the text baseline was at Y=5, causing ascenders (parts 
of letters above baseline) to be cut off at the top of screen.

**Solution:** 
1. Added automatic ascent calculation from font metrics
2. Modified `ft_text_draw()` to offset Y by ascent amount:
   ```c
   int ascent = ctx->face->size->metrics.ascender >> 6;
   int pen_y = y + ascent;  // Move baseline down
   ```
3. Updated `ft_text_get_height()` to return actual font height (ascent + descent)
4. Fixed line height calculation in `ft_text_draw_wrapped()` using font metrics

**Result:**
- ✅ Text at Y=0 now starts at top of screen (no clipping)
- ✅ All font sizes (8pt, 12pt, 16pt) display fully
- ✅ Line spacing is correct based on actual font metrics
- ✅ Word wrapping accounts for proper text height

### Files Changed
- `text_freetype.c`: Added ascent offset in `ft_text_draw()`
- `text_freetype.c`: Fixed `ft_text_get_height()` to use font metrics
- `text_freetype.c`: Fixed `ft_text_draw_wrapped()` line height calculation

### Testing
```bash
# Should show complete text (not clipped)
weactcli -p /dev/ttyACM0 -z 8 "Test"   # Small font - fully visible
weactcli -p /dev/ttyACM0 -z 12 "Test"  # Medium font - fully visible
weactcli -p /dev/ttyACM0 -z 16 "Test"  # Large font - fully visible

# Cyrillic should also be fully visible
weactcli -p /dev/ttyACM0 -z 16 "Привет"  # No clipping!
```

---

## v2.0 (2025-01-09) - MAJOR UPDATE 🎉

### Added
- 🔤 **Full Cyrillic support** via FreeType2 library
- ✨ TrueType font rendering (DejaVu Sans default)
- 🌍 UTF-8 Unicode support (not just transliteration!)
- 🎨 Anti-aliased text rendering
- 📏 Variable font sizes (6-32pt)
- 📚 Comprehensive Cyrillic documentation

### Changed
- Replaced `text_renderer.c/h` (bitmap) with `text_freetype.c/h` (TrueType)
- Updated `Makefile` to link FreeType2 library  
- Updated `install.sh` to check for libfreetype6-dev
- All text rendering now uses FreeType

### Dependencies
- **New:** libfreetype6-dev (FreeType2 library)
  ```bash
  sudo apt-get install libfreetype6-dev
  ```

### Documentation
- Added `CYRILLIC_GUIDE.md` - Complete guide for Russian text
- Added `README_v2.0.md` - Full v2.0 documentation
- Added `V2_SUMMARY.md` - Quick summary of changes

### Examples
```bash
# Real Russian text!
weactcli -p /dev/ttyACM0 "Привет мир"

# Centered Cyrillic
weactcli -p /dev/ttyACM0 --center "Тест"

# Large Cyrillic
weactcli -p /dev/ttyACM0 -z 16 "ПРИВЕТ"
```

### Known Issues in v2.0
- ⚠️ Text clips at top of screen → **FIXED in v2.0.1**

---

## v1.2-DEBUG (2025-01-09)

### Purpose
Diagnostic build to identify root causes of v1.1 issues.

### Added
- Extensive verbose debugging output (`--verbose`)
- Safety checks for text positioning
- Automated test suite (`test-all.sh`)
- Debug documentation (`DEBUG_GUIDE.md`)
- Bounds checking for coordinates

### Fixed
- Text centering bounds checking
- Negative X coordinate protection
- Simplified rendering logic

### Status
⚠️ Diagnostic build only - superseded by v2.0

---

## v1.1 (2025-01-09)

### Attempted Fixes
- Cyrillic transliteration (Latin approximation)
- Text centering logic improvements
- Font rendering thickness reduction

### Issues Found
- ❌ Cyrillic showed blank screen (transliteration failed)
- ❌ Centered text showed only partial (negative X coords)
- ❌ Pipe input showed blank (text width = 0)
- ❌ Font still too thick/bold

### Root Cause
Bitmap fonts insufficient for proper Cyrillic support.
Solution: Use TrueType fonts (implemented in v2.0).

### Status
⚠️ Superseded by v2.0 with real FreeType support

---

## v1.0 (2025-01-09)

### Initial Release
First Linux/C port from Windows/PureBasic version.

### Features
- ✅ Serial port communication
- ✅ BRG565 color support
- ✅ Basic graphics primitives
- ✅ 3 bitmap font sizes (8, 12, 16pt)
- ✅ Word wrapping
- ✅ Text scrolling
- ✅ Image support

### Components
- `weact_display.c/h` - Core display driver
- `text_renderer.c/h` - Bitmap text renderer
- `weactcli.c` - CLI tool
- `install.sh` - Installation script
- `weact-utils.sh` - Utility scripts

### Limitations
- ❌ No Cyrillic support (ASCII only)
- ❌ Bitmap fonts (not TrueType)
- ❌ No anti-aliasing
- ❌ Fixed font sizes

### Documentation
- `README.md`
- `INSTALL.md`
- `QUICKSTART.md`
- `PROJECT_STRUCTURE.md`

---

## Migration Guide

### From v1.x to v2.0+

**Code:** No changes needed! Fully backwards compatible.

**Dependencies:** Install FreeType2
```bash
sudo apt-get install libfreetype6-dev
cd weactcli-linux
make clean && make
```

**Benefits:**
- ✅ Real Cyrillic: `weactcli -p /dev/ttyACM0 "Привет"`
- ✅ Better quality (anti-aliased TrueType)
- ✅ All existing commands work

### From v2.0 to v2.0.1

**Update:** Just rebuild
```bash
cd weactcli-linux
make clean && make
```

**Benefits:**
- ✅ Text no longer clips at top
- ✅ All font sizes display correctly

---

## Version Summary

| Version | Status | Cyrillic | Font Type | Issues |
|---------|--------|----------|-----------|--------|
| v1.0 | Archived | ❌ No | Bitmap | ASCII only |
| v1.1 | Archived | ❌ Transliteration | Bitmap | Blank screens |
| v1.2 | Debug | ❌ Transliteration | Bitmap | Diagnostic only |
| v2.0 | Stable | ✅ Real | TrueType | Text clips |
| **v2.0.1** | **✅ CURRENT** | **✅ Real** | **TrueType** | **None** |

---

## Recommendations

**Current Users:**
- **Using v1.x?** → Upgrade to v2.0.1 for Cyrillic support
- **Using v2.0?** → Update to v2.0.1 to fix text clipping

**New Users:**
- Install **v2.0.1** (latest stable)

---

## Known Issues

### v2.0.1 (Current)
✅ None currently known

### Previous Versions
- v2.0: Text clips at top → **FIXED in v2.0.1**
- v1.x: No Cyrillic, font issues → **FIXED in v2.0**

---

**Current Version:** 2.0.1  
**Release Date:** 2025-01-09  
**Status:** ✅ PRODUCTION READY  
**Recommended:** Install v2.0.1
