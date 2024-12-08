LOG_FILE="connectivity_log.txt"
INPUT_FILE="targets.txt"

check_connectivity() {
    local target=$1
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    # 2 attempts for ping
    if ping -c 2 "$target" >/dev/null 2>&1; then
        echo "$timestamp - SUCCESS: $target is reachable" >> "$LOG_FILE"
        return 0
    else
        echo "$timestamp - FAILURE: $target is unreachable" >> "$LOG_FILE"
        echo "Warning: Host $target is unreachable"
        return 1
    fi
}

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: $INPUT_FILE not found"
    exit 1
fi

> "$LOG_FILE"

while IFS= read -r target || [ -n "$target" ]; do
    # skip empty lines and comments
    [[ -z "$target" || "$target" =~ ^[[:space:]]*# ]] && continue
    check_connectivity "$target"
done < "$INPUT_FILE"