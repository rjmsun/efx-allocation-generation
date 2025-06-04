#include "allocation.hpp"
#include <climits>
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
    int min_val = INT_MAX, total = 0;
    for (int item : bundle) {
        int val = utils[agent][item];
        total += val;
        if (val < min_val) min_val = val;
    }
    return total - min_val;
}

// Checks if a given allocation satisfies EFX
bool isEFX(const Allocation& allocation, const Utilities& utilities) {
    int n = allocation.size();
    for (int i = 0; i < n; ++i) {
        int val_i = total_value(utilities, i, allocation[i]);
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            if (!allocation[j].empty()) {
                int val_j = value_excluding_least(utilities, i, allocation[j]);
                if (val_j > val_i) return false;
            }
        }
    }
    return true;
}

// Brute-force all allocations to test if any satisfy EFX
bool hasEFXAllocation(const Utilities& utilities, int num_items) {
    int n = utilities.size();
    std::vector<int> assignment(num_items);
    int total = std::pow(n, num_items);

    for (int t = 0; t < total; ++t) {
        int x = t;
        for (int i = 0; i < num_items; ++i) {
            assignment[i] = x % n;
            x /= n;
        }

        Allocation allocation(n);
        for (int i = 0; i < num_items; ++i) {
            allocation[assignment[i]].push_back(i);
        }

        if (isEFX(allocation, utilities)) {
            return true;
        }
    }
    return false;
}