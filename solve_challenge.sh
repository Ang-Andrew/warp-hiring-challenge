#!/bin/bash

# Script to solve the Warp Hiring Challenge
# Finds the security code of the longest successful Mars mission

# Start measuring time
start_time=$(python3 -c 'import time; print(time.time())')

# Run the awk script
output=$(awk -F '|' '
{
    # Skip comments and lines without enough fields
    if ($0 ~ /^#/ || NF < 8) next;

    # Trim whitespace from relevant fields
    # $3: Destination
    # $4: Status
    # $6: Duration
    # $8: Security Code
    
    # Using gsub to trim leading/trailing whitespace
    gsub(/^[ \t]+|[ \t]+$/, "", $3);
    gsub(/^[ \t]+|[ \t]+$/, "", $4);
    gsub(/^[ \t]+|[ \t]+$/, "", $6);
    gsub(/^[ \t]+|[ \t]+$/, "", $8);

    # Check if mission is to Mars and Completed
    if ($3 == "Mars" && $4 == "Completed") {
        # Convert duration to number and compare
        if ($6 + 0 > max_duration) {
            max_duration = $6 + 0;
            security_code = $8;
        }
    }
}
END {
    print security_code;
}
' space_missions.log)

# End measuring time
end_time=$(python3 -c 'import time; print(time.time())')

# Calculate duration
duration=$(python3 -c "print(f'{${end_time} - ${start_time}:.3f}')")

echo "Security Code: $output"
echo "Process Time: ${duration}s"
