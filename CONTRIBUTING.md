# Contributing to WeAct Display Tools

Thank you for your interest in contributing! This document provides guidelines and instructions for contributing to the project.

## 🤝 Ways to Contribute

- **Bug Reports**: Report issues you encounter
- **Feature Requests**: Suggest new features or improvements
- **Code Contributions**: Submit pull requests
- **Documentation**: Improve or translate documentation
- **Testing**: Test on different hardware/distributions
- **Examples**: Share creative uses and examples

## 🐛 Reporting Issues

### Before Submitting

1. **Search existing issues** to avoid duplicates
2. **Test with latest version** from main branch
3. **Gather information**:
   - Linux distribution and version
   - Hardware (SBC model, display version)
   - Command that failed (if applicable)
   - Error messages
   - Steps to reproduce

### Creating an Issue

Use our issue templates:

**Bug Report:**
```markdown
**Description:**
Brief description of the bug

**Environment:**
- Linux: Debian 12
- Hardware: Raspberry Pi 4
- Display: WeAct FS 0.96"
- Port: /dev/ttyACM0

**Steps to Reproduce:**
1. Run `weactterm -p /dev/ttyACM0`
2. Type `ls -la`
3. See error...

**Expected Behavior:**
What should happen

**Actual Behavior:**
What actually happens

**Logs/Output:**
```
paste error messages here
```

**Additional Context:**
Any other relevant information
```

## 💡 Feature Requests

We welcome feature suggestions! Please include:

- **Use case**: Why is this feature needed?
- **Description**: What should it do?
- **Examples**: How would it be used?
- **Alternatives**: Have you tried workarounds?

## 🔧 Development Setup

### Prerequisites

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt-get install build-essential libfreetype6-dev pkg-config git

# For testing
sudo apt-get install valgrind gdb
```

### Clone and Build

```bash
# Fork the repository on GitHub first
git clone https://github.com/YOUR_USERNAME/weact-display-tools.git
cd weact-display-tools

# Build
make

# Test
./weactcli --help
./weactterm --help
```

### Project Structure

```
weact-display-tools/
├── weactcli.c          # Text display utility
├── weactterm.c         # Terminal emulator (NEW!)
├── weact_display.c     # Core display library
├── weact_display.h
├── text_freetype.c     # Text rendering with FreeType
├── text_freetype.h
├── weact-utils.sh      # Utility scripts
├── Makefile            # Build system
├── install.sh          # Installation script
├── docs/               # Documentation
│   ├── TERMINAL.md
│   ├── CLI_GUIDE.md
│   └── ...
└── README.md
```

## 📝 Code Style

### C Code Style

Follow Linux kernel coding style:

```c
// Good
void function_name(int param) {
    if (condition) {
        do_something();
    }
}

// Use meaningful variable names
int bytes_read = 0;
bool is_connected = false;

// Comment complex logic
/* Calculate text width accounting for UTF-8 multi-byte characters */
int width = calculate_width(text);
```

### Guidelines

1. **Naming Conventions**:
   - Functions: `snake_case`
   - Constants: `UPPER_CASE`
   - Structs: `snake_case_t`
   
2. **Indentation**: 4 spaces (not tabs)

3. **Line Length**: Max 100 characters

4. **Comments**:
   - Add function documentation
   - Explain complex algorithms
   - Update comments when code changes

5. **Error Handling**:
   - Always check return values
   - Provide meaningful error messages
   - Clean up resources on error

### Example

```c
/**
 * Initialize display connection
 * @param display Display structure to initialize
 * @param port_name Serial port path (e.g., "/dev/ttyACM0")
 * @return true on success, false on failure
 */
bool weact_init(weact_display_t *display, const char *port_name) {
    if (!display || !port_name) {
        return false;
    }
    
    /* Open serial port */
    display->fd = open(port_name, O_RDWR | O_NOCTTY);
    if (display->fd < 0) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Failed to open port: %s", strerror(errno));
        return false;
    }
    
    /* Continue initialization... */
    return true;
}
```

## 🧪 Testing

### Manual Testing

Before submitting PR:

```bash
# Build
make clean
make

# Test all tools
./weactcli -p /dev/ttyACM0 "Test"
./weactterm -p /dev/ttyACM0
./weact-utils test /dev/ttyACM0

# Test error cases
./weactcli -p /dev/INVALID  # Should fail gracefully
./weactterm --help          # Should show help
```

### Test on Multiple Systems

If possible, test on:
- Different Linux distributions (Debian, Ubuntu, Arch, Fedora)
- Different architectures (x86_64, ARM/aarch64)
- Different SBC platforms (Raspberry Pi, Orange Pi, etc.)

### Memory Testing

```bash
# Check for memory leaks
valgrind --leak-check=full ./weactcli -p /dev/ttyACM0 "Test"
```

## 🚀 Submitting Pull Requests

### Workflow

1. **Fork the repository** on GitHub

2. **Create a feature branch**:
   ```bash
   git checkout -b feature/my-new-feature
   # or
   git checkout -b fix/bug-description
   ```

3. **Make your changes**:
   - Write clean, documented code
   - Follow code style guidelines
   - Add/update tests if applicable
   - Update documentation

4. **Commit changes**:
   ```bash
   git add .
   git commit -m "feat: add new feature"
   ```
   
   Use conventional commits:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation only
   - `style:` - Code style changes
   - `refactor:` - Code refactoring
   - `test:` - Add/update tests
   - `chore:` - Maintenance tasks

5. **Push to your fork**:
   ```bash
   git push origin feature/my-new-feature
   ```

6. **Create Pull Request** on GitHub:
   - Describe what changes you made
   - Reference related issues
   - Add screenshots if applicable
   - List testing done

### PR Checklist

Before submitting, ensure:

- [ ] Code builds without warnings: `make`
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] Tested on real hardware
- [ ] No memory leaks (valgrind)
- [ ] Commit messages follow conventions
- [ ] PR description is clear

### PR Example

```markdown
## Description
Adds color support for terminal cursor in weactterm

## Changes
- Modified cursor rendering to use configurable color
- Added `-cc` option for cursor color
- Updated documentation

## Testing
- Tested on Raspberry Pi 4 (Debian 12)
- Tested on Orange Pi Zero (Ubuntu 22.04)
- Verified with valgrind (no leaks)

## Related Issues
Closes #42

## Screenshots
![Cursor Color Demo](screenshots/cursor-demo.png)
```

## 📚 Documentation

### When to Update Documentation

Update documentation when:
- Adding new features
- Changing behavior
- Fixing bugs that affect usage
- Adding examples

### Documentation Files

- `README.md` - Project overview, quick start
- `docs/TERMINAL.md` - Terminal emulator guide
- `docs/CLI_GUIDE.md` - CLI tool documentation
- `docs/TROUBLESHOOTING.md` - Common issues
- Code comments - Function documentation

### Writing Style

- **Clear and concise**
- **Use examples**
- **Include command output**
- **Add troubleshooting hints**
- **Keep formatting consistent**

## 🎯 Priority Areas

We especially welcome contributions in:

1. **Platform Support**
   - Testing on different distributions
   - ARM architecture support
   - macOS/BSD ports

2. **Features**
   - Better ANSI escape sequence support
   - Color support in terminal
   - Improved text rendering
   - More utility scripts

3. **Documentation**
   - More examples
   - Video tutorials
   - Translations
   - Troubleshooting guides

4. **Testing**
   - Automated tests
   - CI/CD pipelines
   - Hardware compatibility testing

## 💬 Communication

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Questions and general discussion
- **Pull Requests**: Code contributions

## 📜 License

By contributing, you agree that your contributions will be licensed under the MIT License.

## 🙏 Recognition

Contributors will be recognized in:
- README.md contributors section
- Release notes
- Git commit history

Thank you for contributing! 🎉

## ❓ Questions?

If you have questions about contributing, please:
1. Check existing documentation
2. Search closed issues/PRs
3. Open a GitHub Discussion
4. Ask in your PR/issue

---

**Happy Contributing!** 🚀
