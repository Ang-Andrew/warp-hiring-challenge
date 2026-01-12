# Warp Hiring Challenge

## About
This is a programming challenge for candidates interested in applying to Warp. The goal is to analyze a space mission log file to find specific mission details.

## Challenge Description
**Objective:** Find the security code of the **longest successful Mars mission**.
- **Input:** `space_missions.log` (Fields: Date, Mission ID, Destination, Status, Crew Size, Duration, Success Rate, Security Code)
- **Criteria:** 
    - Destination: "Mars"
    - Status: "Completed" 
    - Metric: Maximum "Duration"

## Answer
The security code for the longest successful Mars mission is: **`XRT-421-ZQP`**

## Solutions
I have implemented multiple solutions to solve this challenge and compared their performance:

### 1. Optimized Shell Script (`solve_challenge_optimized.sh`)
An efficient script utilizing `awk` for fast text processing without compilation.
- **Run:** `./solve_challenge_optimized.sh`

### 2. Rust Implementation (`solve_challenge.rs`)
A robust, high-performance solution using Rust's standard library.
- **Run:** `rustc solve_challenge.rs && ./solve_challenge`

### 3. Python Implementation (`solve_challenge.py`)
A high-level approach using Python's optimized string methods.
- **Run:** `python3 solve_challenge.py`

### 4. C Implementation (`solve_challenge.c`)
A system-level solution using `mmap` for zero-copy file reading. This was purely vibe coded to see if I could beat the Rust implementation (and because manual memory management builds character).
- **Run:** `gcc -O2 solve_challenge.c -o solve_challenge_c && ./solve_challenge_c`

## Performance
**TL;DR:** The C implementation (`mmap`) takes the crown with an **~19x** speedup, followed by Rust at **~11.5x**.

A performance comparison report has been generated in `performance_report.md`, benchmarking the execution time of all approaches.

## Methodology: Agentic Development
This repository serves as a practical demonstration of **Agentic Development**: the workflow Warp is building for. 

**Tech Stack**: Antigravity

Instead of manually writing every line of code, I effectively acted as a **Technical Lead** for an AI Agent. My role was to:
1.  **Iterative Optimization**: I started by solving the challenge with a Bash script to meet the core requirements. From there, I directed the agent to explore how far we could push performance limits using compiled languages (Rust) and system-level optimizations (C).
2.  **Architect the Benchmark**: Requesting a comparative analysis to prove performance claims.
3.  **Code Review & Optimization**: Directing the agent to implement advanced techniques (like `mmap` in C) to push the hardware limits.

This approach transformed a simple scripting task into a comprehensive, multi-language engineering study in a fraction of the time.

## Extension: Hardware Acceleration
Drawing from my background in FPGA design, I thought it would be fun to take Warp's principle to **"Leverage the cloud"** literally. To push performance beyond the limits of general-purpose CPUs (which are bottlenecked by instruction fetch/decode cycles and cache misses), the next logical step is **Hardware Acceleration via FPGA** running on **AWS EC2 F1** instances.

### Proposed System Architecture
We can offload the log processing to an FPGA (Field-Programmable Gate Array) connected via PCIe Gen4/5.

```text
+-------------------------------------------------------+                                 
|              AWS EC2 F1 Instance (Host)               |                                 
|                                                       |                                 
| +----------------------+    +-----------------------+ |                                 
| | Userspace App (C/C++)|--->|  Xilinx Runtime (XRT) | |                                 
| +----------------------+    +-----------+-----------+ |                                 
|                                         | user        |                                 
| ........................................|.............|                                 
|                                         | kernel      |                                 
|                             +-----------v-----------+ |   PCIe     +--------------------------+
|                             |  AWS FPGA DMA Driver  | | Gen3 x16   |      Xilinx UltraScale+  |
|                             +-----------+-----------+ |<==========>|      (AWS F1 Xilinx)     |
|                                         |             |            +------------+-------------+
|                             +-----------v-----------+ |                         |
|                             |   Host System Memory  | |                         v
|                             |    (Pinned Buffer)    | |            +--------------------------+
|                             +-----------------------+ |            |  AXI4-Stream Interface   |
+-------------------------------------------------------+            +------------+-------------+
                                                                                  | 128-bit width
                                                                                  v
                                                                     +--------------------------+
                                                                     |  Parallel ASCII Parser   |
                                                                     |  (Zero-Stall Pipeline)   |
                                                                     +------------+-------------+
                                                                                  |
                                                                     +------------v-------------+
                                                                     |   Comparator Logic       |
                                                                     | (Update Max in Register) |
                                                                     +--------------------------+
```

**High-Level Data Flow:**
1.  **Host (CPU)**: Maps the `space_missions.log` into a DMA-pinned memory buffer.
2.  **PCIe DMA**: Streams raw data to the FPGA using AXI4-Stream, bypassing CPU caches.
3.  **FPGA Kernel (Custom Logic)**:
    *   **Input Shifter**: A sliding window buffer consumes 512 bits per clock cycle from the AXI4-Stream interface.
    *   **Parallel Parsers**: 64 parallel CAM (Content Addressable Memory) matchers check for "Mars" and "Completed" strings simultaneously across the entire bus width.
    *   **Field Extractor**: Cycle-accurate logic extracts the "Duration" field and parses the ASCII integer into binary instantly.
    *   **Comparator Engine**: Updates the local BRAM (Block RAM) registers only if the validity flags are set and the new duration exceeds the stored maximum.
    *   **CSR (Control Status Register)**: AXI4-Lite interface allows the host to read the final result without interrupting the high-speed datapath.
4.  **Write Back**: The Host CPU performs a low-latency register read via PCIe to fetch the final `Security_Code`.

### FPGA Internal Architecture (Microarchitecture)
A deeper look into the custom logic block instantiated on the FPGA fabric:

```text
                                     +-------------------------------------------+
                                     |           FPGA HW Logic                   |
                                     +-------------------------------------------+
                                     |                                           |
[AXI4-Stream] =====================> |  +---------------+     +---------------+  |
 (512-bit bus)                       |  |  Input FIFO   |====>| Shifter Window|  |
 @ 250 MHz                           |  +-------+-------+     +-------+-------+  |
                                     |          |                     |          |
                                     |          |                     v          |
                                     |          |         +-----------+-------+  |
                                     |          |         | Parallel Parsers  |  |
                                     |          |         | (64x CAM Matchers)|  |
                                     |          |         +-----------+-------+  |
                                     |          |                     | Flags    |
                                     |          v                     v          |
                                     |  +-------+-------+     +-------+-------+  |
                                     |  | Field Extractor|--->| Valid Logic   |  |
                                     |  +-------+-------+     +-------+-------+  |
                                     |          | Duration            | Enable   |
                                     |          v                     v          |
                                     |  +-------+---------------------+-------+  |
                                     |  |      Comparator & Storage Unit      |  |
                                     |  |    (if en & dur > max: update)      |  |
                                     |  +-------+---------------------+-------+  |
                                     |          |                     ^          |
[AXI4-Lite]   <---------------------+|          v                     |          |
 (Register Read)                     |  +-------+---------------------+-------+  |
                                     |  |      Result CSR (Control Status)    |  |
                                     |  +-------------------------------------+  |
                                     +-------------------------------------------+
```

### Cycle Analysis (Theoretical)
Assuming a **250 MHz** clock and a **512-bit (64-byte)** data bus:

*   **Throughput**: $64 \text{ bytes} \times 250 \text{ MHz} = 16 \text{ GB/s}$ (Wire Speed)
*   **Dataset Size**: ~10.1 MB (`space_missions.log`)
*   **Latency**: $10.1 \text{ MB} / 16 \text{ GB/s} \approx \mathbf{635 \text{ } \mu\text{s}}$

| Metric | CPU (Sequential) | FPGA (Pipelined) |
| :--- | :--- | :--- |
| **Clock Speed** | 3-4 GHz | 250 MHz |
| **Data Width** | 64-bit (register) | **512-bit** (AXI Stream) |
| **Cost per Char** | ~20-50 cycles (Branching) | **0.015 cycles** (64 chars/cycle) |
| **Total Latency** | ~27,000 µs (C/mmap) | **~635 µs** (Theoretical) |
| **Speedup** | 1x (Baseline C) | **~42x** (vs Optimized C) |

This architecture shifts the bottleneck entirely from the compute (CPU) to the interconnect (PCIe Bandwidth).
