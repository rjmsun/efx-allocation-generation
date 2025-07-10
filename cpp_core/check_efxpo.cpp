#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include "allocation.hpp"
#include <fstream>
#include <sstream>
#include <random>

using namespace std;

// Type aliases for clarity
using Utilities = vector<vector<int>>;
using Allocation = vector<vector<int>>;

// Calculate bundle value for an agent
int bundle_value(const Utilities& utils, int agent, const vector<int>& bundle) {
    int value = 0;
    for (int item : bundle) {
        value += utils[agent][item];
    }
    return value;
}

void print_matrix(const Utilities& utils, ostream& out) {
    out << "Utility matrix:\n";
    for (size_t i = 0; i < utils.size(); i++) {
        out << "Agent " << i << ": ";
        for (size_t j = 0; j < utils[i].size(); j++) {
            out << utils[i][j] << " ";
        }
        out << endl;
    }
}

// Check if an allocation is EFX
bool is_efx(const Utilities& utils, const Allocation& allocation) {
    int n = utils.size(); // number of agents

    // Check EFX condition for each pair of agents
    for (int i = 0; i < n; i++) {
        int i_value = bundle_value(utils, i, allocation[i]);
        
        // Compare with every other agent's bundle
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            
            // If j's bundle is empty, continue
            if (allocation[j].empty()) continue;

            // Check EFX condition: i should not envy j even after removing any item
            for (int item : allocation[j]) {
                // Create j's bundle without the item
                vector<int> j_bundle_minus = allocation[j];
                j_bundle_minus.erase(
                    remove(j_bundle_minus.begin(), j_bundle_minus.end(), item),
                    j_bundle_minus.end()
                );
                
                // Calculate value after removing item
                int j_value_minus = bundle_value(utils, i, j_bundle_minus);
                
                // If i envies j after removing any item, allocation is not EFX
                if (j_value_minus > i_value) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Print an allocation in the same format as check_mm.cpp
void print_allocation(const Allocation& allocation, const Utilities& utils, ostream& out) {
    out << "Allocation:" << endl;
    for (size_t i = 0; i < allocation.size(); i++) {
        out << "Agent " << i << " gets bundle {";
        for (size_t j = 0; j < allocation[i].size(); j++) {
            out << allocation[i][j] << " ";
        }
        int bundle_value = total_value(utils, i, allocation[i]);
        int total_utility = total_value(utils, i, vector<int>(utils[i].size()));
        for (int item = 0; item < utils[i].size(); ++item) total_utility += 0; // just to avoid unused var warning
        total_utility = 0;
        for (int item = 0; item < utils[i].size(); ++item) total_utility += utils[i][item];
        double proportion = (total_utility > 0) ? (double)bundle_value / total_utility : 0.0;
        out << "}. (Value: " << bundle_value << ", "
            << fixed << setprecision(2) << (proportion * 100) << "% of total utility)" << endl;
    }
    out << endl;
}

// Utility matrix generation methods (copied from check_mm.cpp)
Utilities get_manual_utilities(int num_agents, int num_items) {
    Utilities utils(num_agents, vector<int>(num_items));
    cout << "Enter utility matrix (values for each item separated by spaces):\n";
    for (int i = 0; i < num_agents; i++) {
        cout << "Agent " << i << "(" << num_items << " items): ";
        for (int j = 0; j < num_items; j++) {
            cin >> utils[i][j];
        }
    }
    return utils;
}

Utilities generate_random_utilities(int num_agents, int num_items) {
    Utilities utils(num_agents, vector<int>(num_items));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, 100);
    for (int i = 0; i < num_agents; ++i) {
        for (int j = 0; j < num_items; ++j) {
            utils[i][j] = distrib(gen);
        }
    }
    return utils;
}

Utilities generate_fixed_pattern_utilities(int num_agents, int num_items) {
    Utilities utils(num_agents, vector<int>(num_items));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> high_value(80, 100);
    uniform_int_distribution<> low_value(1, 20);
    uniform_int_distribution<> sporadic(1, 100);
    for (int j = 0; j < num_items; ++j) {
        for (int i = 0; i < num_agents; ++i) {
            if (j < num_items / 2) utils[i][j] = (j%2 == 0) ? high_value(gen) : low_value(gen);
            else utils[i][j] = sporadic(gen);
        }
    }
    return utils;
}

Utilities read_utilities_from_file(const string& filename, int num_agents, int num_items) {
    Utilities utils(num_agents, vector<int>(num_items));
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return generate_random_utilities(num_agents, num_items);
    }
    string line;
    int agent = 0;
    while (getline(file, line) && agent < num_agents) {
        string cleaned_line = line;
        cleaned_line.erase(remove(cleaned_line.begin(), cleaned_line.end(), '['), cleaned_line.end());
        cleaned_line.erase(remove(cleaned_line.begin(), cleaned_line.end(), ']'), cleaned_line.end());
        stringstream ss(cleaned_line);
        int item = 0;
        int value;
        while (ss >> value && item < num_items) {
            utils[agent][item] = value;
            item++;
        }
        if (item != num_items) {
            cerr << "Warning: Agent " << agent << " has " << item << " values, expected " << num_items << endl;
        }
        agent++;
    }
    if (agent != num_agents) {
        cerr << "Warning: Read " << agent << " agents, expected " << num_agents << endl;
    }
    file.close();
    return utils;
}

Utilities generate_builtin_utilities() {
    Utilities utils(4, vector<int>(10));
    utils[0] = {5, 17, 10, 6, 89, 1, 4, 19, 17, 16};
    utils[1] = {16, 9, 11, 94, 7, 12, 10, 2, 2, 2};
    utils[2] = {85, 18, 8, 85, 96, 10, 6, 16, 7, 1};
    utils[3] = {85, 2, 8, 9, 13, 10, 2, 8, 9, 11};
    return utils;
}

int main() {
    int n_agents, n_items;
    cout << "Enter number of agents: ";
    cin >> n_agents;
    cout << "Enter number of items: ";
    cin >> n_items;

    Utilities utils;
    char choice;
    cout << "Choose utility generation method:\n";
    cout << "(R)andom utilities\n";
    cout << "(M)anual input\n";
    cout << "(F)ixed pattern (some high value, some poisonous, some sporadic)\n";
    cout << "(B)uilt in utilities\n";
    cout << "(T)ext file (check_mm_utilities.txt)\n";
    cout << "Choice: ";
    cin >> choice;

    if (choice == 'R' || choice == 'r') {
        utils = generate_random_utilities(n_agents, n_items);
    } else if (choice == 'M' || choice == 'm') {
        utils = get_manual_utilities(n_agents, n_items);
    } else if (choice == 'F' || choice == 'f') {
        utils = generate_fixed_pattern_utilities(n_agents, n_items);
    } else if (choice == 'B' || choice == 'b') {
        utils = generate_builtin_utilities();
    } else if (choice == 'T' || choice == 't') {
        utils = read_utilities_from_file("check_mm_utilities.txt", n_agents, n_items);
    } else {
        cout << "Invalid choice, defaulting to random utilities.\n";
        utils = generate_random_utilities(n_agents, n_items);
    }

    print_matrix(utils, cout);

    // Generate all possible allocations
    vector<Allocation> all_allocations;
    Allocation current(n_agents);
    function<void(int)> generate = [&](int item) {
        if (item == n_items) {
            all_allocations.push_back(current);
            return;
        }
        for (int agent = 0; agent < n_agents; agent++) {
            current[agent].push_back(item);
            generate(item + 1);
            current[agent].pop_back();
        }
    };
    generate(0);
    
    // Collect all EFX allocations
    vector<Allocation> efx_allocs;
    for (const auto& allocation : all_allocations) {
        if (is_efx(utils, allocation)) {
            efx_allocs.push_back(allocation);
        }
    }
    
    // Find Pareto best EFX allocations
    vector<Allocation> pareto_efx_allocs;
    for (const auto& alloc : efx_allocs) {
        if (isParetoOptimalEFX(alloc, utils, efx_allocs)) {
            pareto_efx_allocs.push_back(alloc);
        }
    }
    
    // Find min-optimal EFX allocations (minimizing the minimum percentage utility)
    vector<Allocation> min_optimal_efx_allocs;
    for (const auto& alloc : efx_allocs) {
        if (isMinOptimalEFX(alloc, utils, efx_allocs)) {
            min_optimal_efx_allocs.push_back(alloc);
        }
    }

    cout << "\nTotal allocations checked: " << all_allocations.size() << endl;
    cout << "Total EFX allocations found: " << efx_allocs.size() << endl;
    cout << "Total Pareto maxing EFX allocations: " << pareto_efx_allocs.size() << endl;
    cout << "Total min-optimal EFX allocations: " << min_optimal_efx_allocs.size() << endl;
    cout << endl;
    int count = 0;
    for (const auto& alloc : pareto_efx_allocs) {
        cout << "Pareto maxing EFX allocation #" << (++count) << ":" << endl;
        print_allocation(alloc, utils, cout);
    }

    if (!min_optimal_efx_allocs.empty()) {
        int minopt_count = 0;
        for (const auto& alloc : min_optimal_efx_allocs) {
            cout << "Min-optimal EFX allocation #" << (++minopt_count) << ":" << endl;
            print_allocation(alloc, utils, cout);
        }
    } else {
        cout << "No min-optimal EFX allocation found." << endl;
    }

    return 0;
}
