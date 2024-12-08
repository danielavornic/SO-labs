LOG_FILE="process_log.txt"

track_processes() {
    timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    echo "Process Memory Usage Report $timestamp" > "$LOG_FILE"

    echo "----------------------------------------" >> "$LOG_FILE"
    echo "Top 5 Memory-Consuming Processes:" >> "$LOG_FILE"
    echo "----------------------------------------" >> "$LOG_FILE"
    
    ps aux --sort=-%mem | head -n 6 | awk 'NR>1 {
        printf "PID: %5s | User: %-10s | Memory: %5s%% | CPU: %5s%% | Command: %s\n", 
        $2, $1, $4, $3, $11
    }' >> "$LOG_FILE"
    
    echo "----------------------------------------" >> "$LOG_FILE"
    echo "Full Process List:" >> "$LOG_FILE"
    echo "----------------------------------------" >> "$LOG_FILE"
    
    ps aux --sort=-%mem | awk 'NR>1 {
        printf "PID: %5s | User: %-10s | Memory: %5s%% | CPU: %5s%% | Command: %s\n", 
        $2, $1, $4, $3, $11
    }' >> "$LOG_FILE"
}

track_processes