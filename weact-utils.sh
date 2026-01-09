#!/bin/bash
# WeAct Display Utility Scripts
# Collection of useful shell scripts for WeAct Display

# Default configuration
DEFAULT_PORT="/dev/ttyUSB0"
WEACTCLI="weactcli"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if weactcli is installed
check_weactcli() {
    if ! command -v $WEACTCLI &> /dev/null; then
        echo -e "${RED}Error: weactcli not found${NC}"
        echo "Please install weactcli first:"
        echo "  make && sudo make install"
        exit 1
    fi
}

# Detect available serial port
detect_port() {
    local port=""
    
    # Try common ports
    for p in /dev/ttyUSB0 /dev/ttyACM0 /dev/ttyUSB1 /dev/ttyACM1; do
        if [ -e "$p" ]; then
            port="$p"
            break
        fi
    done
    
    if [ -z "$port" ]; then
        echo -e "${RED}Error: No serial port found${NC}"
        echo "Available devices:"
        ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "None"
        exit 1
    fi
    
    echo "$port"
}

# Get port from args or detect
get_port() {
    if [ -n "$1" ]; then
        echo "$1"
    else
        detect_port
    fi
}

# Show system clock
show_clock() {
    local port=$(get_port "$1")
    echo -e "${GREEN}Starting clock display on $port${NC}"
    echo "Press Ctrl+C to stop"
    
    while true; do
        date "+%H:%M:%S" | $WEACTCLI -p "$port" --center -c cyan -z 16
        sleep 1
    done
}

# Show system status
show_status() {
    local port=$(get_port "$1")
    echo -e "${GREEN}Showing system status on $port${NC}"
    
    (
        echo "=== SYSTEM STATUS ==="
        echo ""
        echo "Time: $(date +%H:%M:%S)"
        echo "Uptime: $(uptime -p | sed 's/up //')"
        echo "Load: $(uptime | awk -F'load average:' '{print $2}')"
        echo ""
        echo "Memory:"
        free -h | grep Mem | awk '{printf "  Used: %s / %s\n", $3, $2}'
        echo ""
        echo "Disk (/):"
        df -h / | tail -1 | awk '{printf "  Used: %s (%s)\n", $3, $5}'
        echo ""
        echo "Network:"
        hostname -I | awk '{printf "  IP: %s\n", $1}'
    ) | $WEACTCLI -p "$port" -z 8 -s 20:u
}

# Show network info
show_network() {
    local port=$(get_port "$1")
    echo -e "${GREEN}Showing network info on $port${NC}"
    
    (
        echo "Network Info"
        echo ""
        echo "Hostname:"
        echo "  $(hostname)"
        echo ""
        echo "IP Address:"
        hostname -I | awk '{print "  " $1}'
        echo ""
        echo "Gateway:"
        ip route | grep default | awk '{print "  " $3}'
    ) | $WEACTCLI -p "$port" -z 8
}

# Show CPU temperature (Raspberry Pi)
show_temperature() {
    local port=$(get_port "$1")
    
    if [ ! -f /sys/class/thermal/thermal_zone0/temp ]; then
        echo -e "${YELLOW}Warning: Temperature sensor not available${NC}"
        echo "This function works on Raspberry Pi"
        return 1
    fi
    
    echo -e "${GREEN}Starting temperature monitor on $port${NC}"
    echo "Press Ctrl+C to stop"
    
    while true; do
        local temp=$(cat /sys/class/thermal/thermal_zone0/temp)
        local temp_c=$((temp / 1000))
        
        local color="green"
        if [ $temp_c -gt 60 ]; then
            color="yellow"
        fi
        if [ $temp_c -gt 75 ]; then
            color="red"
        fi
        
        echo "CPU: ${temp_c}°C" | $WEACTCLI -p "$port" --center -c $color -z 16
        sleep 2
    done
}

# Show countdown timer
countdown() {
    local port=$(get_port "$1")
    local seconds=${2:-10}
    
    echo -e "${GREEN}Starting countdown from $seconds seconds on $port${NC}"
    
    for ((i=seconds; i>=0; i--)); do
        local color="green"
        if [ $i -le 3 ]; then
            color="red"
        elif [ $i -le 5 ]; then
            color="yellow"
        fi
        
        echo "$i" | $WEACTCLI -p "$port" --center -c $color -z 16
        sleep 1
    done
    
    echo "DONE!" | $WEACTCLI -p "$port" --center -c green -z 16
    sleep 1
    $WEACTCLI -p "$port" --cls
}

# Monitor log file
monitor_log() {
    local port=$(get_port "$1")
    local logfile=${2:-"/var/log/syslog"}
    
    if [ ! -f "$logfile" ]; then
        echo -e "${RED}Error: Log file not found: $logfile${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Monitoring $logfile on $port${NC}"
    echo "Press Ctrl+C to stop"
    
    tail -f "$logfile" | while read line; do
        echo "$line" | $WEACTCLI -p "$port" -z 8 -s 25:u
    done
}

# Show custom message with effects
show_message() {
    local port=$(get_port "$1")
    local message="$2"
    local color="${3:-white}"
    local effect="${4:-static}"
    
    if [ -z "$message" ]; then
        echo -e "${RED}Error: No message specified${NC}"
        echo "Usage: $0 message <port> \"Your message\" [color] [effect]"
        echo "Colors: red, green, blue, white, yellow, cyan, magenta"
        echo "Effects: static, scroll-up, scroll-down, center"
        return 1
    fi
    
    echo -e "${GREEN}Showing message on $port${NC}"
    
    case "$effect" in
        scroll-up)
            echo "$message" | $WEACTCLI -p "$port" -c "$color" -s 30:u
            ;;
        scroll-down)
            echo "$message" | $WEACTCLI -p "$port" -c "$color" -s 30:d
            ;;
        center)
            echo "$message" | $WEACTCLI -p "$port" -c "$color" --center -z 12
            ;;
        *)
            echo "$message" | $WEACTCLI -p "$port" -c "$color" -z 10
            ;;
    esac
}

# Display system resources graph
show_resources() {
    local port=$(get_port "$1")
    
    echo -e "${GREEN}Monitoring system resources on $port${NC}"
    echo "Press Ctrl+C to stop"
    
    while true; do
        # Get CPU usage
        local cpu=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d'%' -f1)
        
        # Get memory usage percentage
        local mem=$(free | grep Mem | awk '{printf "%.0f", ($3/$2) * 100}')
        
        # Get disk usage
        local disk=$(df -h / | tail -1 | awk '{print $5}' | cut -d'%' -f1)
        
        (
            echo "System Resources"
            echo ""
            printf "CPU:  %3s%% " "$cpu"
            print_bar "$cpu"
            printf "Mem:  %3s%% " "$mem"
            print_bar "$mem"
            printf "Disk: %3s%% " "$disk"
            print_bar "$disk"
        ) | $WEACTCLI -p "$port" -z 8
        
        sleep 3
    done
}

# Helper function to print a simple ASCII bar
print_bar() {
    local value=$1
    local max=20
    local filled=$((value * max / 100))
    
    printf "["
    for ((i=0; i<filled; i++)); do printf "#"; done
    for ((i=filled; i<max; i++)); do printf " "; done
    printf "]\n"
}

# Clear display
clear_display() {
    local port=$(get_port "$1")
    echo -e "${GREEN}Clearing display on $port${NC}"
    $WEACTCLI -p "$port" --cls
}

# Show help
show_help() {
    cat << EOF
${BLUE}WeAct Display Utility Scripts${NC}

Usage: $0 <command> [port] [options]

${YELLOW}Commands:${NC}
  clock [port]              Show digital clock
  status [port]             Show system status (scrolling)
  network [port]            Show network information
  temperature [port]        Monitor CPU temperature (RPi)
  countdown [port] [sec]    Countdown timer (default: 10 sec)
  monitor [port] [logfile]  Monitor log file (default: syslog)
  message [port] "text" [color] [effect]
                            Show custom message
  resources [port]          Monitor system resources
  clear [port]              Clear display
  detect                    Detect available serial ports
  test [port]               Run quick test
  help                      Show this help

${YELLOW}Colors:${NC}
  red, green, blue, white, black, yellow, cyan, magenta

${YELLOW}Effects:${NC}
  static, scroll-up, scroll-down, center

${YELLOW}Examples:${NC}
  $0 clock                          # Show clock
  $0 clock /dev/ttyUSB0             # Clock on specific port
  $0 status                         # System status
  $0 countdown /dev/ttyUSB0 30      # 30 second countdown
  $0 message /dev/ttyUSB0 "Hello!" green center
  $0 temperature                    # CPU temp (RPi only)
  $0 clear                          # Clear screen

${YELLOW}Notes:${NC}
  - If port is not specified, auto-detection is used
  - Default port priority: /dev/ttyUSB0 > /dev/ttyACM0
  - Requires weactcli to be installed
  - Most commands can be stopped with Ctrl+C

EOF
}

# Detect and list ports
detect_ports() {
    echo -e "${YELLOW}Scanning for serial ports...${NC}"
    echo ""
    
    local found=0
    for port in /dev/ttyUSB* /dev/ttyACM* /dev/ttyS*; do
        if [ -e "$port" ]; then
            echo -e "${GREEN}Found:${NC} $port"
            ls -l "$port"
            found=1
        fi
    done
    
    if [ $found -eq 0 ]; then
        echo -e "${RED}No serial ports found${NC}"
    fi
    
    echo ""
    echo "USB devices:"
    lsusb | grep -i "serial\|uart\|cp210\|ch340\|ftdi" || echo "No USB serial devices"
}

# Quick test
test_display() {
    local port=$(get_port "$1")
    
    echo -e "${GREEN}Running quick test on $port${NC}"
    echo ""
    
    echo "Test 1: Clear screen"
    $WEACTCLI -p "$port" --cls
    sleep 1
    
    echo "Test 2: Simple text"
    echo "WeAct Test" | $WEACTCLI -p "$port" --center -c green
    sleep 2
    
    echo "Test 3: Color test"
    for color in red green blue yellow cyan magenta white; do
        echo "$color" | $WEACTCLI -p "$port" --center -c $color
        sleep 0.5
    done
    
    echo "Test 4: Scrolling"
    echo "Scrolling test - WeAct Display is working!" | $WEACTCLI -p "$port" -s 40:u
    
    echo "Test 5: Clear"
    $WEACTCLI -p "$port" --cls
    
    echo ""
    echo -e "${GREEN}Test complete!${NC}"
}

# Main script logic
check_weactcli

case "$1" in
    clock)
        show_clock "$2"
        ;;
    status)
        show_status "$2"
        ;;
    network)
        show_network "$2"
        ;;
    temperature|temp)
        show_temperature "$2"
        ;;
    countdown)
        countdown "$2" "$3"
        ;;
    monitor)
        monitor_log "$2" "$3"
        ;;
    message|msg)
        show_message "$2" "$3" "$4" "$5"
        ;;
    resources|res)
        show_resources "$2"
        ;;
    clear|cls)
        clear_display "$2"
        ;;
    detect)
        detect_ports
        ;;
    test)
        test_display "$2"
        ;;
    help|--help|-h)
        show_help
        ;;
    "")
        echo -e "${RED}Error: No command specified${NC}"
        echo "Use '$0 help' for usage information"
        exit 1
        ;;
    *)
        echo -e "${RED}Error: Unknown command '$1'${NC}"
        echo "Use '$0 help' for usage information"
        exit 1
        ;;
esac
