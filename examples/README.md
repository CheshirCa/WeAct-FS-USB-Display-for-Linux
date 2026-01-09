# Examples

Example scripts demonstrating WeAct Display Tools usage.

## Available Examples

### 1. status_monitor.sh
Real-time system status monitoring.

**Features:**
- Current time
- IP address
- System uptime
- Load average
- Memory usage
- Disk usage
- CPU temperature (Raspberry Pi)

**Usage:**
```bash
chmod +x status_monitor.sh
./status_monitor.sh /dev/ttyACM0
```

### 2. clock.sh
Simple digital clock display.

**Usage:**
```bash
chmod +x clock.sh

# Default format (HH:MM:SS)
./clock.sh /dev/ttyACM0

# Custom format
./clock.sh /dev/ttyACM0 "%H:%M"
./clock.sh /dev/ttyACM0 "%I:%M %p"
./clock.sh /dev/ttyACM0 "%d/%m/%Y %H:%M"
```

## Creating Your Own

### Template Script

```bash
#!/bin/bash
# My custom display script

PORT="${1:-/dev/ttyACM0}"

# Your logic here
MESSAGE="Hello World"

# Display
weactcli -p "$PORT" --center -c green "$MESSAGE"
```

### Tips

1. **Always specify port** as first argument
2. **Add help message** with usage
3. **Handle Ctrl+C** for loops
4. **Clear display** when appropriate
5. **Use variables** for configuration

## More Examples

See also:
- [CLI_GUIDE.md](../docs/CLI_GUIDE.md) - Many inline examples
- [weact-utils.sh](../weact-utils.sh) - Utility functions

## Contributing

Have a cool example? Submit a pull request!
