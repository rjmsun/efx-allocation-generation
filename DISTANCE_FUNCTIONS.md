# Allocation Distance Functions

This document describes the distance functions implemented for measuring the similarity between allocations in fair division problems.

## Overview

The distance functions are implemented in both Python (`efx.py`) and C++ (`cpp_core/allocation.cpp`). They provide various metrics to quantify how "close" or "similar" two allocations are to each other.

## Available Distance Metrics

### 1. Swap Distance
- **Definition**: The minimum number of swaps needed to transform one allocation into another
- **Algorithm**: Uses cycle decomposition of the swap graph
- **Complexity**: O(n²) where n is the number of agents
- **Range**: 0 to n-1 (where n is the number of agents)

### 2. Normalized Euclidean Distance
- **Definition**: Euclidean distance between normalized utility vectors
- **Normalization**: Utilities are divided by the sum of all valuations for each agent
- **Range**: 0 to √n (where n is the number of agents)
- **Use case**: Measures utility differences while accounting for different scales of valuations

### 3. Chebyshev Distance (L-infinity norm)
- **Definition**: Maximum absolute difference in utilities across all agents
- **Range**: 0 to max possible utility difference
- **Use case**: Focuses on the agent who experiences the largest change in utility

### 4. Earth Mover's Distance
- **Definition**: Minimum "work" required to transform one distribution of items into another
- **Simplified version**: Counts the number of items that need to be moved between agents
- **Range**: 0 to total number of items
- **Use case**: Measures the structural difference in item distribution

### 5. Envy Graph Distance
- **Definition**: Edit distance between envy graphs of two allocations
- **Algorithm**: Counts edge additions/deletions needed to transform one envy graph into another
- **Range**: 0 to n²-n (where n is the number of agents)
- **Use case**: Measures how the envy relationships change between allocations

### 6. Hamming Distance
- **Definition**: Number of items that need to be reassigned to transform one allocation into another
- **Range**: 0 to total number of items
- **Use case**: Simple measure of how many items are in different positions

## Usage Examples

### Python Usage

```python
from efx import *

# Example valuations and allocations
valuations = [
    [5, 3, 2, 1],  # Agent 0 valuations
    [1, 2, 3, 5]   # Agent 1 valuations
]

allocation1 = [[0, 1], [2, 3]]  # Agent 0 gets items 0,1; Agent 1 gets items 2,3
allocation2 = [[0, 2], [1, 3]]  # Agent 0 gets items 0,2; Agent 1 gets items 1,3

# Calculate all distances at once
distances = calculate_all_distances(allocation1, allocation2, valuations)
print(distances)

# Or calculate individual distances
swap_dist = swap_distance(allocation1, allocation2)
euclidean_dist = normalized_euclidean_distance(allocation1, allocation2, valuations)
chebyshev_dist = chebyshev_distance(allocation1, allocation2, valuations)
emd = earth_movers_distance(allocation1, allocation2)
envy_dist = envy_graph_distance(allocation1, allocation2, valuations)
hamming_dist = hamming_distance(allocation1, allocation2)
```

### C++ Usage

```cpp
#include "allocation.hpp"

// Example valuations and allocations
Utilities valuations = {
    {5, 3, 2, 1},  // Agent 0 valuations
    {1, 2, 3, 5}   // Agent 1 valuations
};

Allocation allocation1 = {{0, 1}, {2, 3}};
Allocation allocation2 = {{0, 2}, {1, 3}};

// Calculate individual distances
int swap_dist = swap_distance(allocation1, allocation2);
double euclidean_dist = normalized_euclidean_distance(allocation1, allocation2, valuations);
int chebyshev_dist = chebyshev_distance(allocation1, allocation2, valuations);
int emd = earth_movers_distance(allocation1, allocation2);
int envy_dist = envy_graph_distance(allocation1, allocation2, valuations);
int hamming_dist = hamming_distance(allocation1, allocation2);
```

## Compilation and Testing

### Python
```bash
python test_distances.py
```

### C++
```bash
cd cpp_core
g++ -std=c++11 -Wall -Wextra -O2 test_distances.cpp allocation.cpp -o test_distances
./test_distances
```

## Expected Output

The test programs will output results like:

```
Distance Metrics:
  Swap Distance: 1
  Normalized Euclidean Distance: 0.1286
  Chebyshev Distance: 1
  Earth Mover's Distance: 0
  Envy Graph Distance: 0
  Hamming Distance: 2
```

## When to Use Each Metric

- **Swap Distance**: When you want to understand the operational complexity of transforming one allocation into another
- **Normalized Euclidean Distance**: When comparing utility profiles across different valuation scales
- **Chebyshev Distance**: When you want to focus on the worst-case agent experience
- **Earth Mover's Distance**: When you care about the structural distribution of items
- **Envy Graph Distance**: When analyzing how fairness properties change between allocations
- **Hamming Distance**: When you want a simple, intuitive measure of allocation similarity

## Notes

- All functions handle edge cases like identical allocations (returning 0 distance)
- The implementations are optimized for small to medium-sized problems (up to ~10 agents)
- For larger problems, some metrics (especially swap distance) may become computationally expensive
- The C++ implementation provides better performance for repeated calculations 