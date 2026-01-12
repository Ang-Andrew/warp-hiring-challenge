# Warp Hiring Challenge - Performance Analysis

## Challenge Overview
The goal was to parse a large log file (`space_missions.log`) containing over 100,000 records to find the **security code** of the longest successful Mars mission.

**Criteria:**
*   Destination: `Mars`
*   Status: `Completed`
*   Metric: Maximum `Duration`

## Performance Comparison
I implemented five variations of the solution to solve this problem. Below are the execution times measured on the local machine.

| Implementation | Language | Execution Time | Relative Speed |
| :--- | :--- | :--- | :--- |
| Original Script | Bash (awk) | 0.507s | 1x (Baseline) |
| Optimized Script | Bash (awk) | 0.362s | ~1.4x Faster |
| High Level | Python | 0.153s | ~3.3x Faster |
| System Level | Rust | **0.044s** | **~11.5x Faster** |
| System Level | C (mmap) | **0.027s** | **~18.8x Faster** |

## Implementation Details

### 1. Original Bash Script
The initial approach used `awk` with heavy regular expression usage. It performed `gsub` (global substitution) four times on every single line to trim whitespace, regardless of whether the line was relevant.

```bash
# Heavy regex overhead on every line
gsub(/^[ \t]+|[ \t]+$/, "", $3);
gsub(/^[ \t]+|[ \t]+$/, "", $4);
...
```

### 2. Optimized Bash Script
The optimized version reduced overhead by using loose regex matching (`~ /Mars/`) to filter lines *before* processing them. It relies on `awk`'s native ability to handle leading whitespace when converting strings to numbers, avoiding explicit trimming until the very end.

```bash
# Check first, avoid expensive operations
if ($3 ~ /Mars/ && $4 ~ /Completed/) {
    # Implicit conversion handles whitespace
    duration = $6 + 0
    ...
}
```

### 3. Python Implementation
The Python solution uses standard file I/O and string manipulation. While interpreted, Python's string methods (like `split` and `strip`) are implemented in C and are highly optimized, making it significantly faster than `awk` for this specific task.

```python
# Python's optimized string handling
parts = line.split('|')
if parts[2].strip() == "Mars" and parts[3].strip() == "Completed":
    duration = int(parts[5].strip())
    ...
```

### 4. Rust Implementation
The Rust solution leverages static typing, compilation optimizations, and zero-cost abstractions. It parses the file line-by-line using efficient string splitting and native integer parsing, completely bypassing the overhead of an interpreter and regex engine.

```rust
// Efficient, compiled logic
if destination == "Mars" && status == "Completed" {
    if let Ok(duration) = duration_str.parse::<i32>() {
        if duration > max_duration {
            max_duration = duration;
            security_code = parts[7].trim().to_string();
        }
    }
}
```

### 5. C Implementation
The C solution uses `mmap` to map the entire file into memory, avoiding the overhead of system calls for reading. It manually scans memory for delimiters (`|`) and parses integers directly from the byte buffer. This "zero-copy" approach is typically the fastest possible way to read files on modern OSes.

```c
// Zero-copy memory mapping
char *file_contents = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
// Manual pointer arithmetic
char *line_end = memchr(line_start, '\n', file_end - line_start);
...
```

## Profiling Analysis

### Python Profiling (cProfile)
Profiling reveals that string manipulation methods account for the majority of the execution time.

```text
   ncalls  tottime  percall  cumtime  percall filename:lineno(function)
        1    0.111    0.111    0.200    0.200 solve_challenge.py:3(solve)
   303991    0.040    0.000    0.040    0.000 {method 'strip' of 'str' objects}
   102038    0.031    0.000    0.031    0.000 {method 'split' of 'str' objects}
   105032    0.010    0.000    0.010    0.000 {method 'startswith' of 'str' objects}
```

*   **solve() logic**: ~55% of time (Main loop overhead)
*   **str.strip()**: ~20% of time (Called 300k+ times)
*   **str.split()**: ~15% of time (Called 100k+ times)

This confirms that while Python's C-implemented string methods are fast, the sheer volume of function calls adds up.

### Rust Profiling (System Stats)
System resource usage shows extreme efficiency:

```text
        0.29 real         0.05 user         0.00 sys
             1523712  maximum resident set size
                   0  average shared memory size
                   0  average unshared data size
                   0  average unshared stack size
                 255  page reclaims
                  30  page faults
```

*   **User CPU Time**: 0.05s (Actual computation)
*   **Memory Usage (RSS)**: ~1.5 MB
*   **Page Faults**: ~30 (Minimal memory churn)

The Rust implementation spends almost all its time in user-space computation with negligible system overhead, explaining its 11.5x speed advantage.
