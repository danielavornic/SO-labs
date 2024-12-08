LOG_FILE="open_ports.log"
# common services
ALLOWED_PORTS=(22 80 443 3306 5432 8080)

scan_ports() {
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    > "$LOG_FILE"
    echo "$timestamp - Beginning port scan..." >> "$LOG_FILE"
        
    # excluding localhost
    ss -tuln | grep LISTEN | grep -v "127.0.0.1" | while read -r line; do
        local port=$(echo "$line" | awk '{print $5}' | rev | cut -d':' -f1 | rev)
        local proto=$(echo "$line" | awk '{print $1}')
        
        local process=$(ss -tulnp | grep ":$port" | awk '{print $7}' | cut -d'"' -f2)
        
        echo "$timestamp - Found $proto port $port (Process: $process)" >> "$LOG_FILE"
        
        local is_allowed=false
        for allowed_port in "${ALLOWED_PORTS[@]}"; do
            if [ "$port" -eq "$allowed_port" ]; then
                is_allowed=true
                break
            fi
        done
        
        if [ "$is_allowed" = false ]; then
            echo "WARNING: Unexpected port $port is open ($proto - $process)" | tee -a "$LOG_FILE"
        fi
    done
    
    echo "$timestamp - Scan completed" >> "$LOG_FILE"
}

echo "Allowed ports: ${ALLOWED_PORTS[*]}"

scan_ports