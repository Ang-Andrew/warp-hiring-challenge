#!/bin/bash

# Optimized script to solve the Warp Hiring Challenge

# Start measuring time
start_time=$(python3 -c 'import time; print(time.time())')

# Run the optimized awk script
output=$(awk -F '|' '
{
    # Skip comments and lines without enough fields
    if ($0 ~ /^#/ || NF < 8) next;

    # Optimization: Check for "Mars" and "Completed" using regex match 
    # instead of trimming and string comparison.
    # This avoids expensive gsub calls on every line.
    if ($3 ~ /Mars/ && $4 ~ /Completed/) {
        
        # awk automatically handles leading whitespace when converting to number
        duration = $6 + 0
        
        if (duration > max_duration) {
            max_duration = duration
            security_code = $8
        }
    }
}
END {
    # Only trim the final security code once at the end
    gsub(/^[ \t]+|[ \t]+$/, "", security_code)
    print security_code
}
' space_missions.log)

# End measuring time
end_time=$(python3 -c 'import time; print(time.time())')

# Calculate duration
duration=$(python3 -c "print(f'{${end_time} - ${start_time}:.3f}')")

echo "Security Code: $output"
echo "Process Time: ${duration}s"
