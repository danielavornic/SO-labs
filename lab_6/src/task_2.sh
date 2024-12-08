get_file_hash() {
    local file=$1
    if [ ! -f "$file" ]; then
        echo "Error: File '$file' does not exist."
        exit 1
    fi

    # only keep the hash part, not the filename
    md5sum "$file" | cut -d' ' -f1
}

check_file_integrity() {
    local original_file=$1
    local dir=$2
    local original_hash=$3
    local filename=$(basename "$original_file")
    local match_found=false

    if [ ! -d "$dir" ]; then
        echo "Error: Directory '$dir' does not exist."
        exit 1
    fi

    while IFS= read -r -d '' file; do
        current_hash=$(md5sum "$file" | cut -d' ' -f1)
        if [ "$current_hash" = "$original_hash" ]; then
            echo "File integrity verified for $file"
            match_found=true
        fi
    done < <(find "$dir" -type f -name "$filename" -print0)

    if [ "$match_found" = false ]; then
        echo "File integrity compromised or file not found."
    fi
}

if [ $# -ne 2 ]; then
    echo "Usage: $0 <file_to_check> <directory_to_scan>"
    exit 1
fi

# hash of original file
original_hash=$(get_file_hash "$1")

check_file_integrity "$1" "$2" "$original_hash"