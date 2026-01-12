#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

int main() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int fd = open("space_missions.log", O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    char *file_contents = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_contents == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return 1;
    }

    int max_duration = 0;
    char security_code[32] = {0};
    
    char *line_start = file_contents;
    char *file_end = file_contents + sb.st_size;

    while (line_start < file_end) {
        char *line_end = memchr(line_start, '\n', file_end - line_start);
        if (!line_end) line_end = file_end;

        // Skip comments and empty lines
        if (*line_start == '#' || line_start == line_end) {
            line_start = line_end + 1;
            continue;
        }

        // We need to find the fields. 
        // Format: Date | Mission ID | Destination | Status | Crew Size | Duration | Success Rate | Security Code
        // Indexes: 0    | 1          | 2           | 3      | 4         | 5        | 6            | 7
        
        // Find 2nd pipe (start of Destination)
        char *p1 = memchr(line_start, '|', line_end - line_start);
        if (!p1) { line_start = line_end + 1; continue; }
        
        char *p2 = memchr(p1 + 1, '|', line_end - (p1 + 1));
        if (!p2) { line_start = line_end + 1; continue; }

        // Find 3rd pipe (end of Destination / start of Status)
        char *p3 = memchr(p2 + 1, '|', line_end - (p2 + 1));
        if (!p3) { line_start = line_end + 1; continue; }

        // Check Destination "Mars"
        // We need to handle whitespace. The field is between p2+1 and p3.
        char *dest_start = p2 + 1;
        while (dest_start < p3 && (*dest_start == ' ' || *dest_start == '\t')) dest_start++;
        
        // Check if it starts with Mars
        if (strncmp(dest_start, "Mars", 4) != 0) {
            line_start = line_end + 1; 
            continue; 
        }
        
        // Check if char after Mars is whitespace or pipe
        char *after_mars = dest_start + 4;
        while (after_mars < p3 && (*after_mars == ' ' || *after_mars == '\t')) after_mars++;
        if (after_mars != p3) {
            line_start = line_end + 1; 
            continue;
        }

        // Find 4th pipe (end of Status)
        char *p4 = memchr(p3 + 1, '|', line_end - (p3 + 1));
        if (!p4) { line_start = line_end + 1; continue; }

        // Check Status "Completed"
        char *status_start = p3 + 1;
        while (status_start < p4 && (*status_start == ' ' || *status_start == '\t')) status_start++;
        
        if (strncmp(status_start, "Completed", 9) != 0) {
            line_start = line_end + 1; 
            continue; 
        }
        
        char *after_status = status_start + 9;
        while (after_status < p4 && (*after_status == ' ' || *after_status == '\t')) after_status++;
        if (after_status != p4) {
            line_start = line_end + 1; 
            continue;
        }

        // We found a match! Now get Duration (field 5)
        // p4 is end of Status (field 3). 
        // Field 4 is Crew Size. Need p5.
        char *p5 = memchr(p4 + 1, '|', line_end - (p4 + 1)); // End of Crew Size
        if (!p5) { line_start = line_end + 1; continue; }
        
        char *p6 = memchr(p5 + 1, '|', line_end - (p5 + 1)); // End of Duration
        if (!p6) { line_start = line_end + 1; continue; }

        // Parse Duration
        char *dur_start = p5 + 1;
        while (dur_start < p6 && (*dur_start == ' ' || *dur_start == '\t')) dur_start++;
        
        int duration = 0;
        while (dur_start < p6 && *dur_start >= '0' && *dur_start <= '9') {
            duration = duration * 10 + (*dur_start - '0');
            dur_start++;
        }

        if (duration > max_duration) {
            max_duration = duration;
            
            // Get Security Code (Field 7)
            // p6 is end of Duration (field 5).
            // Field 6 is Success Rate. Need p7.
            char *p7 = memchr(p6 + 1, '|', line_end - (p6 + 1)); // End of Success Rate
            if (!p7) { line_start = line_end + 1; continue; }
            
            // Security Code is from p7+1 to line_end
            char *code_start = p7 + 1;
            while (code_start < line_end && (*code_start == ' ' || *code_start == '\t')) code_start++;
            
            char *code_end = line_end - 1;
            while (code_end > code_start && (*code_end == ' ' || *code_end == '\t' || *code_end == '\r')) code_end--;
            
            int len = code_end - code_start + 1;
            if (len > 31) len = 31;
            memcpy(security_code, code_start, len);
            security_code[len] = '\0';
        }

        line_start = line_end + 1;
    }

    munmap(file_contents, sb.st_size);
    close(fd);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Security Code: %s\n", security_code);
    printf("Process Time: %.3fs\n", time_taken);

    return 0;
}
