# CPUSim - Pipelined CPU and Memory Hierarchy Simulator💻

## Overview
 This program simulates how tasks get allocated to a CPU and how memory blocks move through a cache hierarchy.
 
### Highlights include:
- Round Robin CPU scheduling
- L1, L2, and L3 cache levels
- RAM access when data is not found in cache
- Cycle-by-cycle execution output
- Final performance summary

The simulator reads task details from "input.txt", 
executes the tasks using Round Robin scheduling, 
and shows how each memory request moves through the cache hierarchy.

## Files

cpusim.cpp   - Main C++ source code
input.txt    - Input file containing task information

## How to Compile and Run

g++ .\cpusim.cpp -o .\cpusim.exe; .\cpusim.exe

## Input Format

The program automatically reads from the file:
input.txt
Each valid task line should follow this format:
TASK <task_id> BURST <cpu_cycles> MEM <memory_block1> <memory_block2> ...

## Main Components-

### 1. Task

The Task structure representsa single process waiting to run on the CPU.
The task repeatedly requests memory blocks from its memory list while it runs.

### 2. CacheLevel

The CacheLevel structure represents one cache level such as L1, L2, or L3.
The cache uses FIFO replacement, This means the oldest block is removed first when the cache is full.

### 3. CacheHierarchy

The CacheHierarchy structure connects L1, L2, L3, and RAM.
When a block is found in L2 or L3, it is promoted to L1 so future accesses can be faster.

### 4. Scheduler

The Scheduler class uses Round Robin scheduling.
The default time quantum that we have used is of 3 cycles.
If the task does not finish within those 3 cycles, it goes to the back of the ready queue and the next task gets the CPU.
This keeps scheduling fair because every task gets a turn.

### 5. Simulator

The Simulator class controls the full simulation.

Each CPU cycle performs these steps:
1. Get the currently running task.
2. Ask the task which memory block it needs.
3. Access that block through the cache hierarchy.
4. Print cache hit/miss information.
5. Update task progress.
6. Let the scheduler decide whether to continue or switch tasks.

## Output

The program firstly prints the cycle-by-cycle details and then
ends by printing:-

- CPU time steps
- Total memory latency
- Total cycles
- Number of completed tasks
- Scheduler type
- RAM accesses
- Cache hit/miss breakdown

## Summary

CPUSim demonstrates how CPU scheduling and memory hierarchy work together. It shows how tasks are switched using Round Robin scheduling and how memory requests are handled through L1, L2, L3 cache, and RAM.

------------------------------------------THANK YOU!---------------------------------------
-------------------------------------------(★‿★)-----------------------------------------
