LOG_FILE="ram_usage.log"
CHECK_INTERVAL=60 
TOTAL_CHECKS=10    # run for 10 minutes
ALERT_THRESHOLD=50 # percentage

monitor_ram() {
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    # memory info
    memory_info=$(free -m | grep "^Mem:")
    
    total=$(echo "$memory_info" | awk '{print $2}')
    used=$(echo "$memory_info" | awk '{print $3}')
    free=$(echo "$memory_info" | awk '{print $4}')
    
    usage_percent=$(( (used * 100) / total ))
    
    echo "$timestamp - Total: ${total}MB, Used: ${used}MB, Free: ${free}MB, Usage: ${usage_percent}%" >> "$LOG_FILE"
    
    if [ $usage_percent -gt $ALERT_THRESHOLD ]; then
        echo "$timestamp - ALERT: Memory usage is at ${usage_percent}% (exceeds ${ALERT_THRESHOLD}%)" >> "$LOG_FILE"
        echo "ALERT: Memory usage is at ${usage_percent}%"
    fi
}

> "$LOG_FILE"
echo "Starting RAM usage monitoring for 10 minutes..." >> "$LOG_FILE"

# for 10 minutes
for ((i=1; i<=TOTAL_CHECKS; i++)); do
    monitor_ram
    
    # don't sleep after the last check
    if [ $i -lt $TOTAL_CHECKS ]; then
        sleep $CHECK_INTERVAL
    fi
done

echo "Monitoring completed"