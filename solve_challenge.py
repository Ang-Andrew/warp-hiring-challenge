import time

def solve():
    start_time = time.time()
    
    max_duration = 0
    security_code = ""
    
    try:
        with open('space_missions.log', 'r') as f:
            for line in f:
                # Skip comments and empty lines
                if line.startswith('#') or not line.strip():
                    continue
                
                parts = line.split('|')
                if len(parts) < 8:
                    continue
                
                # parts[2] is Destination, parts[3] is Status
                # We can strip whitespace here
                destination = parts[2].strip()
                status = parts[3].strip()
                
                if destination == "Mars" and status == "Completed":
                    try:
                        # parts[5] is Duration
                        duration = int(parts[5].strip())
                        if duration > max_duration:
                            max_duration = duration
                            # parts[7] is Security Code
                            security_code = parts[7].strip()
                    except ValueError:
                        continue
                        
    except FileNotFoundError:
        print("Error: space_missions.log not found")
        return

    end_time = time.time()
    duration_seconds = end_time - start_time
    
    print(f"Security Code: {security_code}")
    print(f"Process Time: {duration_seconds:.3f}s")

if __name__ == "__main__":
    solve()
