#include "allocation.hpp"
#include <iostream>
#include <iomanip>

void print_allocation(const Allocation& alloc, const std::string& name) {
    std::cout << name << ":" << std::endl;
    for (size_t i = 0; i < alloc.size(); ++i) {
        std::cout << "  Agent " << i << ": [";
        for (size_t j = 0; j < alloc[i].size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << alloc[i][j];
        }
        std::cout << "]" << std::endl;
    }
}

void print_valuations(const Utilities& vals) {
    std::cout << "Valuations:" << std::endl;
    for (size_t i = 0; i < vals.size(); ++i) {
        std::cout << "  Agent " << i << ": [";
        for (size_t j = 0; j < vals[i].size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << vals[i][j];
        }
        std::cout << "]" << std::endl;
    }
}

void test_distance_functions() {
    // Example valuations: 2 agents, 4 items
    Utilities valuations = {
        {5, 3, 2, 1},  // Agent 0 valuations for items 0,1,2,3
        {1, 2, 3, 5}   // Agent 1 valuations for items 0,1,2,3
    };
    
    // Two different allocations
    Allocation allocation1 = {{0, 1}, {2, 3}};  // Agent 0 gets items 0,1; Agent 1 gets items 2,3
    Allocation allocation2 = {{0, 2}, {1, 3}};  // Agent 0 gets items 0,2; Agent 1 gets items 1,3
    
    print_valuations(valuations);
    std::cout << std::endl;
    print_allocation(allocation1, "Allocation 1");
    std::cout << std::endl;
    print_allocation(allocation2, "Allocation 2");
    std::cout << std::endl;
    
    // Calculate all distances
    std::cout << "Distance Metrics:" << std::endl;
    std::cout << "  Swap Distance: " << swap_distance(allocation1, allocation2) << std::endl;
    std::cout << "  Normalized Euclidean Distance: " << std::fixed << std::setprecision(4) 
              << normalized_euclidean_distance(allocation1, allocation2, valuations) << std::endl;
    std::cout << "  Chebyshev Distance: " << chebyshev_distance(allocation1, allocation2, valuations) << std::endl;
    std::cout << "  Earth Mover's Distance: " << earth_movers_distance(allocation1, allocation2) << std::endl;
    std::cout << "  Envy Graph Distance: " << envy_graph_distance(allocation1, allocation2, valuations) << std::endl;
    std::cout << "  Hamming Distance: " << hamming_distance(allocation1, allocation2) << std::endl;
    
    // Show utility calculations for verification
    std::cout << std::endl << "Utility Analysis:" << std::endl;
    for (size_t agent = 0; agent < 2; ++agent) {
        int u1 = total_value(valuations, agent, allocation1[agent]);
        int u2 = total_value(valuations, agent, allocation2[agent]);
        std::cout << "  Agent " << agent << ":" << std::endl;
        std::cout << "    Utility in Allocation 1: " << u1 << std::endl;
        std::cout << "    Utility in Allocation 2: " << u2 << std::endl;
        std::cout << "    Difference: " << std::abs(u1 - u2) << std::endl;
    }
}

void test_identical_allocations() {
    Utilities valuations = {{3, 2, 1}, {1, 2, 3}};
    Allocation allocation = {{0, 1}, {2}};
    
    std::cout << std::endl << "Testing identical allocations:" << std::endl;
    std::cout << "  Swap Distance: " << swap_distance(allocation, allocation) << std::endl;
    std::cout << "  Normalized Euclidean Distance: " << normalized_euclidean_distance(allocation, allocation, valuations) << std::endl;
    std::cout << "  Chebyshev Distance: " << chebyshev_distance(allocation, allocation, valuations) << std::endl;
    std::cout << "  Earth Mover's Distance: " << earth_movers_distance(allocation, allocation) << std::endl;
    std::cout << "  Envy Graph Distance: " << envy_graph_distance(allocation, allocation, valuations) << std::endl;
    std::cout << "  Hamming Distance: " << hamming_distance(allocation, allocation) << std::endl;
}

void test_extreme_cases() {
    // 3 agents, 6 items
    Utilities valuations = {
        {5, 4, 3, 2, 1, 0},
        {0, 1, 2, 3, 4, 5},
        {3, 3, 3, 3, 3, 3}
    };
    
    // Completely different allocations
    Allocation allocation1 = {{0, 1}, {2, 3}, {4, 5}};
    Allocation allocation2 = {{4, 5}, {0, 1}, {2, 3}};
    
    std::cout << std::endl << "Extreme case - completely different allocations:" << std::endl;
    std::cout << "  Swap Distance: " << swap_distance(allocation1, allocation2) << std::endl;
    std::cout << "  Normalized Euclidean Distance: " << normalized_euclidean_distance(allocation1, allocation2, valuations) << std::endl;
    std::cout << "  Chebyshev Distance: " << chebyshev_distance(allocation1, allocation2, valuations) << std::endl;
    std::cout << "  Earth Mover's Distance: " << earth_movers_distance(allocation1, allocation2) << std::endl;
    std::cout << "  Envy Graph Distance: " << envy_graph_distance(allocation1, allocation2, valuations) << std::endl;
    std::cout << "  Hamming Distance: " << hamming_distance(allocation1, allocation2) << std::endl;
}

int main() {
    std::cout << "Testing Allocation Distance Functions" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    test_distance_functions();
    test_identical_allocations();
    test_extreme_cases();
    
    return 0;
} 