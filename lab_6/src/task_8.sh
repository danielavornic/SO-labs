CLEANUP_LOG="cleanup.log"

clean_old_logs() {
    local directory=$1
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    if [ ! -d "$directory" ]; then
        echo "Error: Directory '$directory' does not exist."
        exit 1
    fi
    
    # log files older than 7 days
    old_logs=$(find "$directory" -name "*.log" -type f -mtime +7)
    
    if [ -z "$old_logs" ]; then
        echo "No log files older than 7 days found."
        return
    fi
    
    echo "The following log files will be deleted:"
    echo "$old_logs"
    read -p "Do you want to proceed with deletion? (y/n): " confirm
    
    if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]; then
        echo "$timestamp - Beginning cleanup operation" >> "$CLEANUP_LOG"
        
        while IFS= read -r file; do
            if rm "$file"; then
                echo "$timestamp - Deleted: $file" >> "$CLEANUP_LOG"
                echo "Deleted: $file"
            else
                echo "$timestamp - Failed to delete: $file" >> "$CLEANUP_LOG"
                echo "Failed to delete: $file"
            fi
        done <<< "$old_logs"
        
        echo "$timestamp - Cleanup operation completed" >> "$CLEANUP_LOG"
    else
        echo "Operation cancelled by user."
        echo "$timestamp - Cleanup operation cancelled by user" >> "$CLEANUP_LOG"
    fi
}

if [ $# -ne 1 ]; then
    echo "Usage: $0 <directory_path>"
    exit 1
fi

clean_old_logs "$1"