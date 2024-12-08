check_disk_usage() {
    df -h | grep -vE '^Filesystem|tmpfs|cdrom' | awk '
    {
        cmd = "date \"+%Y-%m-%d %H:%M:%S\""
        cmd | getline timestamp
        close(cmd)
        
        # convert usage to number explicitly
        usage = substr($5, 1, length($5)-1) + 0
        
        print timestamp " - Partition: " $6 ", Usage: " $5 >> "disk_usage.log"
        
        if (usage + 0 > 80) {
            print "Warning: Partition " $6 " is above 80% usage." > "/dev/stderr"
        }
    }'
}

check_disk_usage