# SYSC4001_A3_P2

# Assignment 3 Part 2 - Concurrent Marking System

**Student 1:** Akshavi Baskaran (101315124)  
**Student 2:** Liam Addie (101302106)  
**Course:** SYSC 4001 - Operating Systems  

---

## 1. Project Overview
This project simulates a concurrent exam marking system where multiple Teaching Assistant (TA) processes access shared resources (a rubric and a pile of exams).

* **Part 2a:** Implements the simulation using Shared Memory but **without** synchronization or semaphores. Race conditions are expected.
* **Part 2b:** Adds System V Semaphores to synchronize access, satisfying the requirements of the Critical Section Solution.

---

## 2. File List
The following files should be present in your directory before running:

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

**Compile Part 2b:**
```bash
gcc part2b_marking_101302106_101315124.c -o part2b

