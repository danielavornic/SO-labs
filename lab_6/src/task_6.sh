BACKUP_DIR="/tmp/backups"

create_backup() {
    local source_dir="$1"
    local timestamp=$(date "+%Y%m%d_%H%M%S")
    local dir_name=$(basename "$source_dir")
    local backup_file="${dir_name}_${timestamp}.tar.gz"
    
    if [ ! -d "$source_dir" ]; then
        echo "Directory '$source_dir' does not exist."
        exit 1
    fi
    
    mkdir -p "$BACKUP_DIR"
    
    if tar -czf "$backup_file" -C "$(dirname "$source_dir")" "$dir_name"; then
        echo "Successfully created backup archive: $backup_file"
        
        if mv "$backup_file" "$BACKUP_DIR/"; then
            echo "Backup moved to: $BACKUP_DIR/$backup_file"
        else
            echo "Error: Failed to move backup to destination."
            exit 1
        fi
    else
        echo "Error: Failed to create backup archive."
        exit 1
    fi
}

if [ $# -ne 1 ]; then
    echo "Usage: $0 <directory_to_backup>"
    exit 1
fi

create_backup "$1"