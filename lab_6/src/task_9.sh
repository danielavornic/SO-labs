LOG_FILE="cron_failures.log"

monitor_cron_jobs() {
    local user=$1
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    > "$LOG_FILE"
    echo "$timestamp - Beginning cron job analysis for user: $user" >> "$LOG_FILE"
    
    echo "Current cron jobs for user $user:"
    crontab -l -u "$user"
    
    crontab -l -u "$user" | grep -v '^#' | while read -r job; do
        if [ -n "$job" ]; then
            command=$(echo "$job" | awk '{for(i=6;i<=NF;i++) printf "%s ", $i}')
            command=${command%% }
                        
            if $command >/dev/null 2>&1; then
                echo "$timestamp - Command $command executes successfully" >> "$LOG_FILE"
            else
                # check the logs for its executions
                log_entries=$(journalctl -u cronie.service --since "24 hours ago" | grep "($user).*$command")
                if [ -n "$log_entries" ]; then
                    echo "Found failed execution for job: $command"
                    echo "$timestamp - Failed job: $command" >> "$LOG_FILE"
                    echo "$log_entries" | tail -n 5 >> "$LOG_FILE"
                fi
            fi
        fi
    done
    
    echo "$timestamp - Cron job analysis completed" >> "$LOG_FILE"
}

if [ $# -ne 1 ]; then
    echo "Usage: $0 <username>"
    exit 1
fi

monitor_cron_jobs "$1"