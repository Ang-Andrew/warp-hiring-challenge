#!/bin/bash

# Start measuring time
start_time=$(date +%s.%N)

# Run the optimized awk script with debugging
output=$(awk -F '|' '
BEGIN {
    max_duration = -1
}
# Skip comments early
/^#/ { next }

# Only process lines with Mars and Completed
$3 ~ /Mars/ && $4 ~ /Completed/ {
    duration = $6 + 0
    
    if (duration > max_duration) {
        max_duration = duration
        security_code = $8
    }
}
END {
    # Trim whitespace from security code
    gsub(/^[ \t]+|[ \t]+$/, "", security_code)
    if (security_code != "") {
        print security_code
    } else {
        print "ERROR: No matching missions found" > "/dev/stderr"
    }
}
' space_missions.log)

# End measuring time
end_time=$(date +%s.%N)

# Calculate duration using awk
duration=$(awk "BEGIN {printf \"%.3f\", ${end_time} - ${start_time}}")

echo "Security Code: $output"
echo "Process Time: ${duration}s"