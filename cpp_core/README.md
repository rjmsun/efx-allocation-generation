# EFX Allocation MM Checker

This directory contains C++ code for finding Maximin (MM) allocations and checking if they satisfy Envy-Free up to One Item (EFX).

## Files

- `check_mm.cpp` - Main program for finding MM allocations and checking EFX
- `allocation.cpp` - Implementation of allocation utilities and EFX checking
- `allocation.hpp` - Header file with function declarations
- `Makefile` - Build configuration
- `run_mm_checker.sh` - Convenient script to compile and run

## How to Compile and Run

### Option 1: Using Makefile

```bash
# Compile the program
make

# Run the program
./mm_checker

# Or compile and run in one command
make run

# Clean build artifacts
make clean
```

### Option 2: Using the shell script

```bash
# Make the script executable (if not already)
chmod +x run_mm_checker.sh

# Run the script
./run_mm_checker.sh
```

### Option 3: Manual compilation

```bash
# Compile manually
g++ -std=c++11 -Wall -Wextra -O2 -c check_mm.cpp -o check_mm.o
g++ -std=c++11 -Wall -Wextra -O2 -c allocation.cpp -o allocation.o
g++ check_mm.o allocation.o -o mm_checker

# Run
./mm_checker
```

## Program Usage

When you run the program, it will prompt you for:

1. **Number of agents** (e.g., 4)
2. **Number of items** (e.g., 10) - Warning: >12 for 4 agents can be very slow
3. **Utility generation method**:
   - `R` - Random utilities
   - `M` - Manual input
   - `F` - Fixed pattern (some high value, some poisonous, some sporadic)

## Output

The program will:
1. Display the utility matrix
2. Find the maximum minimum proportion of utility (Pass 1)
3. Find all MM allocations and check them for EFX (Pass 2)
4. Display results to console and save detailed output to `mm.txt`

## Example Output

```
--- MM and EFX Allocation Finder ---
Enter number of agents (e.g., 4): 4
Enter number of items (e.g., 10). Warning: >12 for 4 agents can be very slow: 10
Choose utility generation method:
(R)andom utilities
(M)anual input
(F)ixed pattern (some high value, some poisonous, some sporadic)
Choice: R

Utility matrix:
Agent 0 (Total utility: 536): 62 94 52 42 84 59 5 59 57 22 
Agent 1 (Total utility: 459): 60 9 76 90 27 84 54 15 15 29 
Agent 2 (Total utility: 610): 80 63 31 34 53 87 72 29 94 67 
Agent 3 (Total utility: 446): 98 64 7 93 63 2 8 13 88 10 

Starting Pass 1: Finding the maximinal proportion of utility...
Pass 1 Complete. The highest possible minimum proportion of utility is: 36.32%

Starting Pass 2: Finding all MM allocations and checking for EFX...

--- ANALYSIS COMPLETE ---
Time taken: 0.81 seconds
Total allocations checked: 1048576

Found 1 Maximinal (MM) Allocations:
Allocation:
Agent 0 gets bundle {2 4 7 }. (Value: 195, 36.38% of total utility)
Agent 1 gets bundle {3 5 }. (Value: 174, 37.91% of total utility)
Agent 2 gets bundle {6 8 9 }. (Value: 233, 38.20% of total utility)
Agent 3 gets bundle {0 1 }. (Value: 162, 36.32% of total utility)

Found 1 Allocations that are both MM and EFX:
Allocation:
Agent 0 gets bundle {2 4 7 }. (Value: 195, 36.38% of total utility)
Agent 1 gets bundle {3 5 }. (Value: 174, 37.91% of total utility)
Agent 2 gets bundle {6 8 9 }. (Value: 233, 38.20% of total utility)
Agent 3 gets bundle {0 1 }. (Value: 162, 36.32% of total utility)
```

## Performance Notes

- The program uses brute force enumeration, so computation time grows exponentially with the number of items
- For 4 agents and 10 items, it checks 4^10 = 1,048,576 allocations
- For larger instances, the program will warn you before proceeding
- The program includes safety checks to prevent extremely long computations

## Dependencies

- C++11 compatible compiler (g++ recommended)
- Standard C++ libraries (no external dependencies) 