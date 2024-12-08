check_file_permissions() {
    local file=$1
    if [ ! -f "$file" ]; then
        echo "'$file' does not exist."
        exit 1
    fi

    # current permissions in octal
    current_perms=$(stat -c "%a" "$file")

    if [ "$current_perms" -gt 644 ]; then
        echo "The file $file has insecure permissions."
        chmod 644 "$file"
        echo "Permissions have been corrected to 644 (rw-r--r--)."
    fi
}

check_sh_files() {
    local dir=$1
    if [ ! -d "$dir" ]; then
        echo "Directory '$dir' does not exist"
        exit 1
    fi

    # all .sh files in the directory
    while IFS= read -r -d '' script_file; do
        if [ ! -x "$script_file" ]; then
            echo "The script $script_file is not executable."
        fi
    done < <(find "$dir" -type f -name "*.sh" -print0)
}

if [ $# -ne 2 ]; then
    echo "Usage: $0 <file_to_check> <directory_to_scan>"
    exit 1
fi

check_file_permissions "$1"
check_sh_files "$2"