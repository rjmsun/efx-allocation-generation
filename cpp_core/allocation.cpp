#include "allocation.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>
using namespace std;

// Computes total utility an agent assigns to a bundle
int total_value(const Utilities& utils, int agent, const vector<int>& bundle) {
    int total = 0;
    for (int item : bundle) {
        total += utils[agent][item];
    }
    return total;
}

// Computes utility excluding the least valued item in the bundle
int value_excluding_least(const Utilities& utils, int agent, const vector<int>& bundle) {
    if (bundle.empty()) return 0;
    
    int min_value = numeric_limits<int>::max();
    int total = 0;
    
    for (int item : bundle) {
        int value = utils[agent][item];
        total += value;
        min_value = min(min_value, value);
    }
    
    return total - min_value;
}

// Checks if a given allocation satisfies EFX
bool isEFX(const Allocation& allocation, const Utilities& utilities) {
    int num_agents = allocation.size();
    
    // For each agent i
    for (int i = 0; i < num_agents; ++i) {
        // For each other agent j
        for (int j = 0; j < num_agents; ++j) {
            if (i == j) continue;
            
            // Get the value of i's bundle
            int i_value = total_value(utilities, i, allocation[i]);
            
            // Get the value of j's bundle excluding least valued item
            int j_value_excluding_least = value_excluding_least(utilities, i, allocation[j]);
            
            // If i prefers j's bundle (minus one item) over their own, it's not EFX
            if (j_value_excluding_least > i_value) {
                return false;
            }
        }
    }
    return true;
}

// Brute-force all allocations to test if any satisfy EFX
bool hasEFXAllocation(const Utilities& utilities, int num_items) {
    int num_agents = utilities.size();
    long long total_possible_allocations = 1;
    for (int i = 0; i < num_items; ++i) {
        total_possible_allocations *= num_agents;
    }
    
    for (long long t = 0; t < total_possible_allocations; ++t) {
        Allocation current_allocation(num_agents);
        long long temp_t = t;
        for (int i = 0; i < num_items; ++i) {
            current_allocation[temp_t % num_agents].push_back(i);
            temp_t /= num_agents;
        }
        
        if (isEFX(current_allocation, utilities)) {
            return true;
        }
    }
    return false;
}

// Returns true if alloc1 Pareto dominates alloc2 (all agents at least as good, one strictly better)
bool pareto_dominates(const Allocation& alloc1, const Allocation& alloc2, const Utilities& utils) {
    int n = alloc1.size();
    bool at_least_one_strictly_better = false;
    for (int i = 0; i < n; ++i) {
        int v1 = total_value(utils, i, alloc1[i]);
        int v2 = total_value(utils, i, alloc2[i]);
        if (v1 < v2) return false;
        if (v1 > v2) at_least_one_strictly_better = true;
    }
    return at_least_one_strictly_better;
}

// Returns true if alloc is Pareto optimal among EFX allocations in the list
bool isParetoOptimalEFX(const Allocation& alloc, const Utilities& utils, const vector<Allocation>& efx_allocs) {
    for (const auto& other : efx_allocs) {
        if (&other == &alloc) continue;
        if (isEFX(other, utils) && pareto_dominates(other, alloc, utils)) {
            return false;
        }
    }
    return true;
}

// Returns the minimum percentage utility for an allocation
static double min_percentage_utility(const Allocation& alloc, const Utilities& utils) {
    int n = alloc.size();
    double min_percentage = numeric_limits<double>::max();
    for (int i = 0; i < n; ++i) {
        int bundle_value = total_value(utils, i, alloc[i]);
        int total_utility = 0;
        for (size_t item = 0; item < utils[i].size(); ++item) {
            total_utility += utils[i][item];
        }
        double percentage = (total_utility > 0) ? (double)bundle_value / total_utility : 0.0;
        min_percentage = min(min_percentage, percentage);
    }
    return min_percentage;
}

// Returns true if alloc1 min-dominates alloc2 (min percentage utility in alloc1 > min percentage utility in alloc2)
bool min_dominates(const Allocation& alloc1, const Allocation& alloc2, const Utilities& utils) {
    double min1 = min_percentage_utility(alloc1, utils);
    double min2 = min_percentage_utility(alloc2, utils);
    return min1 > min2;
}

// Returns true if alloc is min-optimal among EFX allocations in the list
bool isMinOptimalEFX(const Allocation& alloc, const Utilities& utils, const vector<Allocation>& efx_allocs) {
    double min_val = min_percentage_utility(alloc, utils);
    for (const auto& other : efx_allocs) {
        if (&other == &alloc) continue;
        if (isEFX(other, utils) && min_percentage_utility(other, utils) > min_val) {
            return false;
        }
    }
    return true;
}