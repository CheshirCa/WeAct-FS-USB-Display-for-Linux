#!/bin/bash
# Simple system status monitor for WeAct Display
# Usage: ./status_monitor.sh [port]

PORT="${1:-/dev/ttyACM0}"
REFRESH_INTERVAL=5

echo "Starting status monitor on $PORT"
echo "Press Ctrl+C to stop"
echo ""

while true; do
    # Collect system info
    TIMESTAMP=$(date "+%H:%M:%S")
    IP=$(hostname -I | awk '{print $1}')
    UPTIME=$(uptime -p | sed 's/up //')
    LOAD=$(uptime | awk -F'load average:' '{print $2}' | awk '{print $1}')
    MEM=$(free -h | grep Mem | awk '{printf "%s/%s", $3, $2}')
    DISK=$(df -h / | tail -1 | awk '{print $5}')
    
    # CPU temp (Raspberry Pi only)
    if [ -f /sys/class/thermal/thermal_zone0/temp ]; then
        TEMP=$(cat /sys/class/thermal/thermal_zone0/temp)
        TEMP_C=$((TEMP / 1000))
        TEMP_LINE="Temp: ${TEMP_C}°C"
    else
        TEMP_LINE=""
    fi
    
    # Display status
    (
        echo "=== System Status ==="
        echo "Time: $TIMESTAMP"
        echo "IP: $IP"
        echo "Uptime: $UPTIME"
        echo "Load: $LOAD"
        echo "Memory: $MEM"
        echo "Disk: $DISK"
        [ -n "$TEMP_LINE" ] && echo "$TEMP_LINE"
    ) | weactcli -p "$PORT" -z 8 -c cyan
    
    sleep $REFRESH_INTERVAL
done
