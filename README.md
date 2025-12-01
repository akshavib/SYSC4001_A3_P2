# SYSC4001_A3_P2

**Concurrent Marking System**  
**Students:** Akshavi Baskaran (101315124), Liam Addie (101302106)

## Overview
This project simulates multiple TA processes marking exams concurrently.  
- **Part 2A:** Shared Memory only → race conditions expected.  
- **Part 2B:** Shared Memory + Semaphores → synchronized, correct behavior.

## Files Needed
- `part2a_marking_101302106_101315124.c`  
- `part2b_marking_101302106_101315124.c`  
- `rubric.txt` (5 lines of rubric text)  
- `exam01.txt` … `exam19.txt` (student IDs)  
- `exam20.txt` (contains **9999** to end simulation)

## Compile
```bash
gcc part2a_marking_101302106_101315124.c -o part2a
gcc part2b_marking_101302106_101315124.c -o part2b

gcc part2b_marking_101302106_101315124.c -o part2b

Run

Both programs take one argument: number of TAs.

./part2a <num_TAs>
./part2b <num_TAs>

Test Cases
# 2 TAs
./part2a 2
./part2b 2

# 3 TAs
./part2a 3
./part2b 3

# 4 TAs
./part2a 4
./part2b 4

# 5 TAs (stress)
./part2a 5
./part2b 5

Expected Behavior

Part 2A: interleaving output, duplicated marking → race conditions.

Part 2B: ordered marking, no conflicts, clean shutdown message.

