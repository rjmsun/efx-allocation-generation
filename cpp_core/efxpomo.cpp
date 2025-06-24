#include "allocation.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <cmath>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    int num_agents, num_items;
    Utilities utils;

    // Automated mode: read from stdin
    if (argc > 1 && string(argv[1]) == "--automate") {
        cin >> num_agents;
        cin >> num_items;
        utils.assign(num_agents, vector<int>(num_items));
        for (int i = 0; i < num_agents; ++i) {
            for (int j = 0; j < num_items; ++j) {
                cin >> utils[i][j];
            }
        }
    } else {
        cout << "Automated mode only. Please run with --automate and provide input via stdin." << endl;
        return 1;
    }

    // Generate all possible allocations
    vector<Allocation> all_allocations;
    Allocation current(num_agents);
    function<void(int)> generate = [&](int item) {
        if (item == num_items) {
            all_allocations.push_back(current);
            return;
        }
        for (int agent = 0; agent < num_agents; agent++) {
            current[agent].push_back(item);
            generate(item + 1);
            current[agent].pop_back();
        }
    };
    generate(0);

    // Collect all EFX allocations
    vector<Allocation> efx_allocs;
    for (const auto& allocation : all_allocations) {
        if (isEFX(allocation, utils)) {
            efx_allocs.push_back(allocation);
        }
    }

    // Find Pareto-optimal EFX allocations
    vector<Allocation> pareto_efx_allocs;
    for (const auto& alloc : efx_allocs) {
        if (isParetoOptimalEFX(alloc, utils, efx_allocs)) {
            pareto_efx_allocs.push_back(alloc);
        }
    }

    // Find min-optimal EFX allocations
    vector<Allocation> min_optimal_efx_allocs;
    for (const auto& alloc : efx_allocs) {
        if (isMinOptimalEFX(alloc, utils, efx_allocs)) {
            min_optimal_efx_allocs.push_back(alloc);
        }
    }

    // Output JSON
    json result;
    result["min_optimal_efx_allocations"] = json::array();
    for (const auto& alloc : min_optimal_efx_allocs) {
        result["min_optimal_efx_allocations"].push_back(alloc);
    }
    result["pareto_optimal_efx_allocations"] = json::array();
    for (const auto& alloc : pareto_efx_allocs) {
        result["pareto_optimal_efx_allocations"].push_back(alloc);
    }
    cout << result.dump() << endl;
    return 0;
} 