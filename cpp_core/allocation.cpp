#include "allocation.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <functional>
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

// Distance functions implementation

int swap_distance(const Allocation& allocation1, const Allocation& allocation2) {
    int n = allocation1.size();
    
    // Check if allocations are identical
    if (allocation1 == allocation2) {
        return 0;
    }
    
    // Create a mapping from items to their target agent
    unordered_map<int, int> item_to_target;
    for (int agent = 0; agent < n; ++agent) {
        for (int item : allocation2[agent]) {
            item_to_target[item] = agent;
        }
    }
    
    // Build the swap graph: edge (i,j) means agent i has an item that should go to agent j
    vector<vector<int>> swap_graph(n);
    for (int agent = 0; agent < n; ++agent) {
        for (int item : allocation1[agent]) {
            int target_agent = item_to_target[item];
            if (target_agent != agent) {
                swap_graph[agent].push_back(target_agent);
            }
        }
    }
    
    // Count cycles using DFS
    vector<bool> visited(n, false);
    int cycles = 0;
    
    function<bool(int, int)> dfs = [&](int node, int start) -> bool {
        if (visited[node]) {
            return node == start;
        }
        visited[node] = true;
        for (int neighbor : swap_graph[node]) {
            if (dfs(neighbor, start)) {
                return true;
            }
        }
        return false;
    };
    
    for (int i = 0; i < n; ++i) {
        if (!visited[i] && !swap_graph[i].empty()) {
            if (dfs(i, i)) {
                cycles++;
            }
        }
    }
    
    // Minimum swaps = n - cycles
    return n - cycles;
}

double normalized_euclidean_distance(const Allocation& allocation1, const Allocation& allocation2, const Utilities& valuations) {
    int n = allocation1.size();
    
    // Calculate utility vectors for both allocations
    vector<double> utils1(n), utils2(n);
    
    for (int agent = 0; agent < n; ++agent) {
        // Calculate total utility for agent in allocation1
        double u1 = 0;
        for (int item : allocation1[agent]) {
            u1 += valuations[agent][item];
        }
        utils1[agent] = u1;
        
        // Calculate total utility for agent in allocation2
        double u2 = 0;
        for (int item : allocation2[agent]) {
            u2 += valuations[agent][item];
        }
        utils2[agent] = u2;
    }
    
    // Normalize utilities by dividing by sum of all valuations for each agent
    vector<double> normalized_utils1(n), normalized_utils2(n);
    
    for (int agent = 0; agent < n; ++agent) {
        double total_valuation = 0;
        for (size_t item = 0; item < valuations[agent].size(); ++item) {
            total_valuation += valuations[agent][item];
        }
        
        if (total_valuation > 0) {
            normalized_utils1[agent] = utils1[agent] / total_valuation;
            normalized_utils2[agent] = utils2[agent] / total_valuation;
        } else {
            normalized_utils1[agent] = 0;
            normalized_utils2[agent] = 0;
        }
    }
    
    // Calculate Euclidean distance
    double squared_diff_sum = 0;
    for (int i = 0; i < n; ++i) {
        double diff = normalized_utils1[i] - normalized_utils2[i];
        squared_diff_sum += diff * diff;
    }
    
    return sqrt(squared_diff_sum);
}

int chebyshev_distance(const Allocation& allocation1, const Allocation& allocation2, const Utilities& valuations) {
    int n = allocation1.size();
    
    int max_diff = 0;
    for (int agent = 0; agent < n; ++agent) {
        int u1 = 0, u2 = 0;
        
        for (int item : allocation1[agent]) {
            u1 += valuations[agent][item];
        }
        for (int item : allocation2[agent]) {
            u2 += valuations[agent][item];
        }
        
        int diff = abs(u1 - u2);
        max_diff = max(max_diff, diff);
    }
    
    return max_diff;
}

int earth_movers_distance(const Allocation& allocation1, const Allocation& allocation2) {
    int n = allocation1.size();
    
    // Count items per agent in each allocation
    vector<int> count1(n), count2(n);
    for (int i = 0; i < n; ++i) {
        count1[i] = allocation1[i].size();
        count2[i] = allocation2[i].size();
    }
    
    // Calculate total flow needed
    int total_flow = 0;
    for (int i = 0; i < n; ++i) {
        // If agent i has more items in allocation1 than allocation2, 
        // we need to move the excess to other agents
        if (count1[i] > count2[i]) {
            total_flow += count1[i] - count2[i];
        }
    }
    
    return total_flow;
}

set<pair<int, int>> build_envy_graph(const Allocation& allocation, const Utilities& valuations) {
    int n = allocation.size();
    set<pair<int, int>> graph;
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                // Calculate utility of agent i for their own bundle
                int u_own = 0;
                for (int item : allocation[i]) {
                    u_own += valuations[i][item];
                }
                
                // Calculate utility of agent i for agent j's bundle
                int u_other = 0;
                for (int item : allocation[j]) {
                    u_other += valuations[i][item];
                }
                
                if (u_other > u_own) {
                    graph.insert({i, j});
                }
            }
        }
    }
    
    return graph;
}

int envy_graph_distance(const Allocation& allocation1, const Allocation& allocation2, const Utilities& valuations) {
    auto envy_graph1 = build_envy_graph(allocation1, valuations);
    auto envy_graph2 = build_envy_graph(allocation2, valuations);
    
    // Calculate edit distance
    int additions = 0, deletions = 0;
    
    // Count additions (edges in graph2 but not in graph1)
    for (const auto& edge : envy_graph2) {
        if (envy_graph1.find(edge) == envy_graph1.end()) {
            additions++;
        }
    }
    
    // Count deletions (edges in graph1 but not in graph2)
    for (const auto& edge : envy_graph1) {
        if (envy_graph2.find(edge) == envy_graph2.end()) {
            deletions++;
        }
    }
    
    return additions + deletions;
}

int hamming_distance(const Allocation& allocation1, const Allocation& allocation2) {
    int n = allocation1.size();
    
    // Count total items
    int total_items = 0;
    for (const auto& bundle : allocation1) {
        total_items += bundle.size();
    }
    
    // Count items that are in the same position in both allocations
    int same_position = 0;
    for (int agent = 0; agent < n; ++agent) {
        // Create sets for efficient intersection calculation
        set<int> set1(allocation1[agent].begin(), allocation1[agent].end());
        set<int> set2(allocation2[agent].begin(), allocation2[agent].end());
        
        // Count intersection
        for (int item : set1) {
            if (set2.find(item) != set2.end()) {
                same_position++;
            }
        }
    }
    
    // Hamming distance = total items - items in same position
    return total_items - same_position;
}