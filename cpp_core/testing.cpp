#include "allocation.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
using namespace std;

// Function to generate random utility matrix
vector<vector<int>> generate_random_utilities(int num_agents, int num_items, mt19937& rng) {
    vector<vector<int>> utilities(num_agents, vector<int>(num_items));
    uniform_int_distribution<int> dist(1, 99);
    for (int i = 0; i < num_agents; ++i)
        for (int j = 0; j < num_items; ++j)
            utilities[i][j] = dist(rng);
    return utilities;
}

// Print utility matrix
void print_utility_matrix(const Utilities& utilities, ostream& out) {
    out << "Utility Matrix:" << endl;
    for (size_t i = 0; i < utilities.size(); ++i) {
        out << "Agent " << i << ": [";
        for (size_t j = 0; j < utilities[i].size(); ++j) {
            if (j > 0) out << ", ";
            out << utilities[i][j];
        }
        out << "]" << endl;
    }
    out << endl;
}

// Compute the min utility across all agents for an allocation
int min_utility(const Allocation& alloc, const Utilities& utils) {
    int n = alloc.size();
    int min_val = numeric_limits<int>::max();
    for (int i = 0; i < n; ++i) {
        int val = total_value(utils, i, alloc[i]);
        min_val = min(min_val, val);
    }
    return min_val;
}

// Returns true if alloc is Pareto optimal among EFX allocations in the list
bool isParetoOptimalEFX(const Allocation& alloc, const Utilities& utils, const std::vector<Allocation>& efx_allocs) {
    for (const auto& other : efx_allocs) {
        if (&other == &alloc) continue;
        if (isEFX(other, utils) && pareto_dominates(other, alloc, utils)) {
            return false;
        }
    }
    return true;
}

// Returns true if alloc is min-optimal among EFX allocations in the list
bool isMinOptimalEFX(const Allocation& alloc, const Utilities& utils, const std::vector<Allocation>& efx_allocs) {
    double min_val = numeric_limits<double>::max();
    int n = alloc.size();
    for (int i = 0; i < n; ++i) {
        int bundle_value = total_value(utils, i, alloc[i]);
        int total_utility = 0;
        for (size_t item = 0; item < utils[i].size(); ++item) {
            total_utility += utils[i][item];
        }
        double percentage = (total_utility > 0) ? (double)bundle_value / total_utility : 0.0;
        min_val = min(min_val, percentage);
    }
    for (const auto& other : efx_allocs) {
        if (&other == &alloc) continue;
        double other_min = numeric_limits<double>::max();
        for (int i = 0; i < n; ++i) {
            int bundle_value = total_value(utils, i, other[i]);
            int total_utility = 0;
            for (size_t item = 0; item < utils[i].size(); ++item) {
                total_utility += utils[i][item];
            }
            double percentage = (total_utility > 0) ? (double)bundle_value / total_utility : 0.0;
            other_min = min(other_min, percentage);
        }
        if (other_min > min_val) {
            return false;
        }
    }
    return true;
}

// Brute-force all allocations for a utility matrix and print counts
void brute_force_counts(const Utilities& utilities, ostream& out) {
    int num_agents = utilities.size();
    int num_items = utilities[0].size();
    long long total_possible_allocations = 1;
    for (int i = 0; i < num_items; ++i)
        total_possible_allocations *= num_agents;

    // Find the max-min value for all allocations (to identify MM allocations)
    int global_max_min = numeric_limits<int>::min();
    vector<Allocation> all_allocs;
    vector<int> all_min_utils;

    for (long long t = 0; t < total_possible_allocations; ++t) {
        Allocation current_allocation(num_agents);
        long long temp_t = t;
        for (int i = 0; i < num_items; ++i) {
            current_allocation[temp_t % num_agents].push_back(i);
            temp_t /= num_agents;
        }
        int min_util = min_utility(current_allocation, utilities);
        all_allocs.push_back(current_allocation);
        all_min_utils.push_back(min_util);
        global_max_min = max(global_max_min, min_util);
    }

    // Find EFX allocations
    vector<Allocation> efx_allocs;
    vector<int> efx_min_utils;
    for (size_t idx = 0; idx < all_allocs.size(); ++idx) {
        const Allocation& alloc = all_allocs[idx];
        if (isEFX(alloc, utilities)) {
            efx_allocs.push_back(alloc);
            efx_min_utils.push_back(all_min_utils[idx]);
        }
    }

    // Count MM+EFX
    int mm_efx_count = 0;
    for (size_t i = 0; i < efx_allocs.size(); ++i) {
        if (efx_min_utils[i] == global_max_min) {
            mm_efx_count++;
        }
    }

    // Count min-optimal EFX
    int min_optimal_efx_count = 0;
    for (size_t i = 0; i < efx_allocs.size(); ++i) {
        if (isMinOptimalEFX(efx_allocs[i], utilities, efx_allocs)) {
            min_optimal_efx_count++;
        }
    }

    // Count Pareto-optimal EFX
    int pareto_optimal_efx_count = 0;
    for (size_t i = 0; i < efx_allocs.size(); ++i) {
        if (isParetoOptimalEFX(efx_allocs[i], utilities, efx_allocs)) {
            pareto_optimal_efx_count++;
        }
    }

    out << "Number of EFX allocations: " << efx_allocs.size() << endl;
    out << "Number of min-optimal EFX allocations: " << min_optimal_efx_count << endl;
    out << "Number of Pareto-optimal EFX allocations: " << pareto_optimal_efx_count << endl;
    out << "Number of MM + EFX allocations: " << mm_efx_count << endl;
}

int main() {
    random_device rd;
    mt19937 rng(rd());
    vector<int> item_counts = {7}; // For larger item counts, brute force will be slow!
    const int num_agents = 4;
    const int tests_per_config = 1; // For demo, set to 1. Increase as needed.

    ofstream outfile("testing_results.txt");
    if (!outfile.is_open()) {
        cerr << "Error: Could not open testing_results.txt for writing" << endl;
        return 1;
    }

    int total_test_count = 0;
    for (int num_items : item_counts) {
        outfile << "Testing configuration: " << num_agents << " agents, " << num_items << " items" << endl;
        outfile << string(50, '-') << endl << endl;
        for (int test = 1; test <= tests_per_config; ++test) {
            total_test_count++;
            Utilities utilities = generate_random_utilities(num_agents, num_items, rng);
            outfile << "=============================================================" << endl;
            outfile << "Test #" << total_test_count << " - " << num_agents << " agents, " << num_items << " items" << endl;
            outfile << "=============================================================" << endl << endl;
            print_utility_matrix(utilities, outfile);
            brute_force_counts(utilities, outfile);
            outfile << endl;
        }
        outfile << endl;
    }
    outfile << "Testing completed. Total tests: " << total_test_count << endl;
    outfile << "Results saved to testing_results.txt" << endl;
    outfile.close();
    cout << "Testing completed successfully!" << endl;
    cout << "Total tests run: " << total_test_count << endl;
    cout << "Results saved to testing_results.txt" << endl;
    return 0;
}