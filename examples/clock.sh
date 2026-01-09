#!/bin/bash
# Digital clock for WeAct Display
# Usage: ./clock.sh [port] [format]

PORT="${1:-/dev/ttyACM0}"
FORMAT="${2:-%H:%M:%S}"

echo "Starting clock on $PORT"
echo "Format: $FORMAT"
echo "Press Ctrl+C to stop"
echo ""

# Clear display first
weactcli -p "$PORT" --cls

while true; do
    # Get current time
    TIME=$(date "+$FORMAT")
    
    # Display centered
    weactcli -p "$PORT" --center -c cyan -z 16 "$TIME"
    
    sleep 1
done
