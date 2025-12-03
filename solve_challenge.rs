use std::fs::File;
use std::io::{self, BufRead};
use std::path::Path;
use std::time::Instant;

fn main() -> io::Result<()> {
    let start = Instant::now();

    let path = Path::new("space_missions.log");
    let file = File::open(&path)?;
    let reader = io::BufReader::new(file);

    let mut max_duration = 0;
    let mut security_code = String::new();

    for line in reader.lines() {
        let line = line?;
        // Skip comments and empty lines
        if line.starts_with('#') || line.trim().is_empty() {
            continue;
        }

        let parts: Vec<&str> = line.split('|').collect();
        if parts.len() < 8 {
            continue;
        }

        // parts[2] is Destination, parts[3] is Status
        let destination = parts[2].trim();
        let status = parts[3].trim();

        if destination == "Mars" && status == "Completed" {
            // parts[5] is Duration
            let duration_str = parts[5].trim();
            if let Ok(duration) = duration_str.parse::<i32>() {
                if duration > max_duration {
                    max_duration = duration;
                    // parts[7] is Security Code
                    security_code = parts[7].trim().to_string();
                }
            }
        }
    }

    let duration = start.elapsed();

    println!("Security Code: {}", security_code);
    println!("Process Time: {:.3}s", duration.as_secs_f64());

    Ok(())
}
