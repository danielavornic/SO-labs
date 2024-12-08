LOG_FILE="service_status.log"

check_and_restart_service() {
    local service_name=$1
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    # if service exists
    if ! systemctl list-unit-files | grep -q "^$service_name.service"; then
        echo "$timestamp - Error: Service $service_name does not exist." >> "$LOG_FILE"
        return
    fi
    
    if ! systemctl is-active --quiet "$service_name"; then
        echo "$timestamp - Service $service_name is not running. Attempting to restart." >> "$LOG_FILE"
        
        # restart the service 
        if systemctl restart "$service_name"; then
            echo "$timestamp - Successfully restarted service $service_name" >> "$LOG_FILE"
        else
            echo "$timestamp - Failed to restart service $service_name" >> "$LOG_FILE"
        fi
    else
        echo "$timestamp - Service $service_name is running normally." >> "$LOG_FILE"
    fi
}

if [ $# -eq 0 ]; then
    echo "Usage: $0 service1 service2 ..."
    exit 1
fi

> "$LOG_FILE"

for service in "$@"; do
    check_and_restart_service "$service"
done