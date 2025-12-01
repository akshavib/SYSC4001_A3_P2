# SYSC4001_A3_P2

**Student 1:** Akshavi Baskaran (101315124)  
**Student 2:** Liam Addie (101302106)  

---

## 1. Project Overview
This project simulates a concurrent exam marking system where multiple Teaching Assistant (TA) processes access shared resources (a rubric and a pile of exams).

* **Part 2a:** Implements the simulation using Shared Memory but **without** synchronization. Race conditions are expected and observed.
* **Part 2b:** Adds Semaphores to synchronize access, ensuring it satisfies the requirements of the Critical Section Solution.

---

## 2. File List
Ensure the following files are present in your directory before running:

**Source Code:**
* `part2a_marking_101302106_101315124.c` (Unsynchronized)
* `part2b_marking_101302106_101315124.c` (Synchronized)

**Input Files (Required):**
* `rubric.txt`: Contains 5 lines of rubric text (e.g., "1, A").
* `exam01.txt` to `exam19.txt`: Text files containing 4-digit student IDs.
* `exam20.txt`: **Must** contain the terminator ID "9999" to stop the simulation.

---

## 3. Compilation Instructions
Use `gcc` to compile the programs. No special flags are required beyond standard linking.

**Compile Part 2a:**
```bash
gcc part2a_marking_101302106_101315124.c -o part2a
```
**Compile Part 2b:**
```bash
gcc part2b_marking_101302106_101315124.c -o part2b
```
---

## 4. Execution Instructions
Both programs accept a single command-line argument determining the number of TA processes to spawn.

**Syntax:**
```bash
./part2a <number_of_TAs>
./part2b <number_of_TAs>
```

**Test Cases:**
You can run the simulation with varying numbers of TAs to observe different behaviors (race conditions in Part A vs. sequential ordering in Part B).

**Test Case 1: 2 TAs**
```bash
./part2a 2
./part2b 2
```
**Test Case 2: 3 TAs**
```bash
./part2a 3
./part2b 3
```
**Test Case 3: 4 TAs**
```bash
./part2a 4
./part2b 4
```
**Test Case 4: 5 TAs (stress test)**
```bash
./part2a 5
./part2b 5
```

---

## 5. Expected Output
Part 2a: You may see output lines indicating race conditions, such as:
* `Question X was already marked by another TA`
* Interleaved output messages
* Duplicate marking by multiple TAs

Part 2b: You should see a clean, serialized execution log:
* `[REVIEW]` -> `[MARK]` sequence is orderly.
* No duplicate marking errors
* `Shared memory and semaphores cleaned up` at the end
