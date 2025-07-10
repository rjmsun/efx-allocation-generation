#include "allocation.hpp"
#include <iostream>
#include <iomanip>
using namespace std;

void print_allocation(const Allocation& alloc, const string& name) {
    cout << name << ":" << endl;
    for (size_t i = 0; i < alloc.size(); ++i) {
        cout << "  Agent " << i << ": [";
        for (size_t j = 0; j < alloc[i].size(); ++j) {
            if (j > 0) cout << ", ";
            cout << alloc[i][j];
        }
        cout << "]" << endl;
    }
}

void print_valuations(const Utilities& vals) {
    cout << "Valuations:" << endl;
    for (size_t i = 0; i < vals.size(); ++i) {
        cout << "  Agent " << i << ": [";
        for (size_t j = 0; j < vals[i].size(); ++j) {
            if (j > 0) cout << ", ";
            cout << vals[i][j];
        }
        cout << "]" << endl;
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
    cout << endl;
    print_allocation(allocation1, "Allocation 1");
    cout << endl;
    print_allocation(allocation2, "Allocation 2");
    cout << endl;
    
    // Calculate all distances
    cout << "Distance Metrics:" << endl;
    cout << "  Swap Distance: " << swap_distance(allocation1, allocation2) << endl;
    cout << "  Normalized Euclidean Distance: " << fixed << setprecision(4) 
              << normalized_euclidean_distance(allocation1, allocation2, valuations) << endl;
    cout << "  Chebyshev Distance: " << chebyshev_distance(allocation1, allocation2, valuations) << endl;
    cout << "  Earth Mover's Distance: " << earth_movers_distance(allocation1, allocation2) << endl;
    cout << "  Envy Graph Distance: " << envy_graph_distance(allocation1, allocation2, valuations) << endl;
    cout << "  Hamming Distance: " << hamming_distance(allocation1, allocation2) << endl;
    
    // Show utility calculations for verification
    cout << endl << "Utility Analysis:" << endl;
    for (size_t agent = 0; agent < 2; ++agent) {
        int u1 = total_value(valuations, agent, allocation1[agent]);
        int u2 = total_value(valuations, agent, allocation2[agent]);
        cout << "  Agent " << agent << ":" << endl;
        cout << "    Utility in Allocation 1: " << u1 << endl;
        cout << "    Utility in Allocation 2: " << u2 << endl;
        cout << "    Difference: " << abs(u1 - u2) << endl;
    }
}

void test_identical_allocations() {
    Utilities valuations = {{3, 2, 1}, {1, 2, 3}};
    Allocation allocation = {{0, 1}, {2}};
    
    cout << endl << "Testing identical allocations:" << endl;
    cout << "  Swap Distance: " << swap_distance(allocation, allocation) << endl;
    cout << "  Normalized Euclidean Distance: " << normalized_euclidean_distance(allocation, allocation, valuations) << endl;
    cout << "  Chebyshev Distance: " << chebyshev_distance(allocation, allocation, valuations) << endl;
    cout << "  Earth Mover's Distance: " << earth_movers_distance(allocation, allocation) << endl;
    cout << "  Envy Graph Distance: " << envy_graph_distance(allocation, allocation, valuations) << endl;
    cout << "  Hamming Distance: " << hamming_distance(allocation, allocation) << endl;
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
    
    cout << endl << "Extreme case - completely different allocations:" << endl;
    cout << "  Swap Distance: " << swap_distance(allocation1, allocation2) << endl;
    cout << "  Normalized Euclidean Distance: " << normalized_euclidean_distance(allocation1, allocation2, valuations) << endl;
    cout << "  Chebyshev Distance: " << chebyshev_distance(allocation1, allocation2, valuations) << endl;
    cout << "  Earth Mover's Distance: " << earth_movers_distance(allocation1, allocation2) << endl;
    cout << "  Envy Graph Distance: " << envy_graph_distance(allocation1, allocation2, valuations) << endl;
    cout << "  Hamming Distance: " << hamming_distance(allocation1, allocation2) << endl;
}

int main() {
    cout << "Testing Allocation Distance Functions" << endl;
    cout << "=====================================" << endl;
    
    test_distance_functions();
    test_identical_allocations();
    test_extreme_cases();
    
    return 0;
} 