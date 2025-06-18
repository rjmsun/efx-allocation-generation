#include "allocation.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>

// Computes total utility an agent assigns to a bundle
int total_value(const Utilities& utils, int agent, const std::vector<int>& bundle) {
    int total = 0;
    for (int item : bundle) {
        total += utils[agent][item];
    }
    return total;
}

// Computes utility excluding the least valued item in the bundle
int value_excluding_least(const Utilities& utils, int agent, const std::vector<int>& bundle) {
    if (bundle.empty()) return 0;
    
    int min_value = std::numeric_limits<int>::max();
    int total = 0;
    
    for (int item : bundle) {
        int value = utils[agent][item];
        total += value;
        min_value = std::min(min_value, value);
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