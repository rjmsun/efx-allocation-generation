#ifndef ALLOCATION_HPP
#define ALLOCATION_HPP

#include <vector>
#include <set>
#include <cmath>
#include <algorithm>

using Allocation = std::vector<std::vector<int>>;
using Utilities = std::vector<std::vector<int>>;

// total utiliy that an agent assigns to bundle
int total_value(const Utilities& utils, int agent, const std::vector<int>& bundle);

// total utiliy that an agent assigns to bundle after removal
int value_excluding_least(const Utilities& utils, int agent, const std::vector<int>& bundle);

// checks EFX
bool isEFX(const Allocation& allocation, const Utilities& utilities);

// brute force
bool hasEFXAllocation(const Utilities& utilities, int num_items);

// Returns true if alloc1 Pareto dominates alloc2 (all agents at least as good, one strictly better)
bool pareto_dominates(const Allocation& alloc1, const Allocation& alloc2, const Utilities& utils);

// Returns true if alloc is Pareto optimal among EFX allocations in the list
bool isParetoOptimalEFX(const Allocation& alloc, const Utilities& utils, const std::vector<Allocation>& efx_allocs);

// Returns true if alloc1 min-dominates alloc2 (min percentage utility in alloc1 > min percentage utility in alloc2)
bool min_dominates(const Allocation& alloc1, const Allocation& alloc2, const Utilities& utils);

// Returns true if alloc is min-optimal among EFX allocations in the list
bool isMinOptimalEFX(const Allocation& alloc, const Utilities& utils, const std::vector<Allocation>& efx_allocs);

// Distance functions between allocations

// Calculate the minimum number of swaps needed to transform allocation1 into allocation2
int swap_distance(const Allocation& allocation1, const Allocation& allocation2);

// Calculate Euclidean distance between normalized utility vectors
double normalized_euclidean_distance(const Allocation& allocation1, const Allocation& allocation2, const Utilities& valuations);

// Calculate Chebyshev distance (L-infinity norm) between utility vectors
int chebyshev_distance(const Allocation& allocation1, const Allocation& allocation2, const Utilities& valuations);

// Calculate Earth Mover's Distance between allocations
int earth_movers_distance(const Allocation& allocation1, const Allocation& allocation2);

// Calculate the edit distance between envy graphs of two allocations
int envy_graph_distance(const Allocation& allocation1, const Allocation& allocation2, const Utilities& valuations);

// Calculate Hamming distance between allocations
int hamming_distance(const Allocation& allocation1, const Allocation& allocation2);

// Helper function to build envy graph
std::set<std::pair<int, int>> build_envy_graph(const Allocation& allocation, const Utilities& valuations);

#endif