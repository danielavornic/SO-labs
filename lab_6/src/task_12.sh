LOG_FILE="dir_size_audit.log"

audit_directories() {
    local target_dir="$1"
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    if [ ! -d "$target_dir" ]; then
        echo "Error: Directory '$target_dir' does not exist."
        exit 1
    fi
    
    {
        echo "$timestamp - Directory size audit for: $target_dir"
        echo "----------------------------------------"
        echo "Top 5 largest directories:"
        
        # store the dir sizes in a temp var
        cd "$target_dir"
        du_output=$(du -h --max-depth=1 */ 2>/dev/null | sort -hr | head -n 5)
        
        while IFS= read -r line; do
            size=$(echo "$line" | awk '{print $1}')
            dir=$(echo "$line" | awk '{print $2}' | xargs basename)
            echo "Size: $size  Directory: $dir"
        done <<< "$du_output"
        
        echo "----------------------------------------"
        echo "$timestamp - Audit completed"
    } | tee "$LOG_FILE"
}

if [ $# -ne 1 ]; then
    echo "Usage: $0 <directory_to_audit>"
    exit 1
fi

audit_directories "$1"