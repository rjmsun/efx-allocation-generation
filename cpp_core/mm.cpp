#include "allocation.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iterator>
#include <limits>
#include <random>
#include <chrono>
#include <iomanip>

// Include for JSON output
#include <nlohmann/json.hpp>
// For convenience
using json = nlohmann::json;

using namespace std;

// Calculate total utility for an agent across all items
int total_agent_utility(const Utilities& utils, int agent) {
    int total = 0;
    for (size_t j = 0; j < utils[agent].size(); ++j) {
        total += utils[agent][j];
    }
    return total;
}

// --- No changes to your existing print functions, manual/random generation etc. ---

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

void print_allocation(const Allocation& allocation, const Utilities& utils, ostream& out) {
    out << "Allocation:\n";
    for (size_t i = 0; i < allocation.size(); i++) {
        out << "Agent " << i << " gets bundle {";
        for (size_t j = 0; j < allocation[i].size(); j++) {
            out << allocation[i][j] << " ";
        }
        int bundle_value = total_value(utils, i, allocation[i]);
        int total_utility = total_agent_utility(utils, i);
        double proportion = (total_utility > 0) ? (double)bundle_value / total_utility : 0;
        out << "}. (Value: " << bundle_value << ", "
            << fixed << setprecision(2) << (proportion * 100) << "% of total utility)" << endl;
    }
}

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
    Utilities utils(num_agents, std::vector<int>(num_items));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);

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


void print_utilities(const Utilities& utils, ostream& out) {
    out << "Utility matrix:\n";
    for (size_t i = 0; i < utils.size(); i++) {
        int total = total_agent_utility(utils, i);
        out << "Agent " << i << " (Total utility: " << total << "): ";
        for (size_t j = 0; j < utils[i].size(); j++) {
            out << utils[i][j] << " ";
        }
        out << endl;
    }
}


int main(int argc, char* argv[]) {
    int num_agents;
    int num_items;
    Utilities utilities;

    // Automated mode for Python script
    if (argc > 1 && string(argv[1]) == "--automate") {
        cin >> num_agents;
        cin >> num_items;
        utilities.assign(num_agents, vector<int>(num_items));
        for (int i = 0; i < num_agents; ++i) {
            for (int j = 0; j < num_items; ++j) {
                cin >> utilities[i][j];
            }
        }
    } else { // Interactive mode
        ofstream outfile("mm.txt");
         if (!outfile) {
            cerr << "Error: Could not open output file mm.txt" << endl;
            return 1;
        }

        cout << "--- MM and EFX Allocation Finder ---\n";
        outfile << "--- MM and EFX Allocation Finder ---\n";
        cout << "Enter number of agents (e.g., 4): ";
        cin >> num_agents;
        cout << "Enter number of items (e.g., 10). Warning: >12 for 4 agents can be very slow: ";
        cin >> num_items;
        
        char choice;
        cout << "Choose utility generation method:\n";
        cout << "(R)andom utilities\n";
        cout << "(M)anual input\n";
        cout << "(F)ixed pattern (some high value, some poisonous, some sporadic)\n";
        cout << "Choice: ";
        cin >> choice;

        if (choice == 'R' || choice == 'r') {
            utilities = generate_random_utilities(num_agents, num_items);
        } else if (choice == 'M' || choice == 'm') {
            utilities = get_manual_utilities(num_agents, num_items);
        } else if (choice == 'F' || choice == 'f') {
            utilities = generate_fixed_pattern_utilities(num_agents, num_items);
        } else {
            cout << "Invalid choice, defaulting to random utilities.\n";
            utilities = generate_random_utilities(num_agents, num_items);
        }
        print_utilities(utilities, cout);
        print_utilities(utilities, outfile);
        outfile.close();
    }


    long double total_possible_allocations_ld = pow(num_agents, num_items);
    if (total_possible_allocations_ld > 50000000) { // Safety check for automated mode
        if (argc > 1 && string(argv[1]) == "--automate") {
            // In automated mode, just exit if it's too large to prevent hanging
            json result;
            result["error"] = "Instance too large to compute.";
            cout << result.dump() << endl;
            return 1;
        }
    }
    long long total_possible_allocations = (long long)total_possible_allocations_ld;

    // --- Pass 1: Find the maximum-minimum proportion of utility ---
    double max_min_proportion = -1.0;

    for (long long t = 0; t < total_possible_allocations; ++t) {
        Allocation current_allocation(num_agents);
        long long temp_t = t;
        for (int i = 0; i < num_items; ++i) {
            current_allocation[temp_t % num_agents].push_back(i);
            temp_t /= num_agents;
        }

        double current_min_proportion = numeric_limits<double>::max();
        for (int i = 0; i < num_agents; ++i) {
            int bundle_value = total_value(utilities, i, current_allocation[i]);
            int total_utility = total_agent_utility(utilities, i);
            double proportion = (total_utility > 0) ? (double)bundle_value / total_utility : 0;
            current_min_proportion = min(current_min_proportion, proportion);
        }
        max_min_proportion = max(max_min_proportion, current_min_proportion);
    }
    
    // --- Pass 2: Find all MM allocations and check them for EFX ---
    vector<Allocation> mm_allocations;
    vector<Allocation> mm_efx_allocations;
    const double MM_EPSILON = 0.0001; 

    for (long long t = 0; t < total_possible_allocations; ++t) {
        Allocation current_allocation(num_agents);
        long long temp_t = t;
        for (int i = 0; i < num_items; ++i) {
            current_allocation[temp_t % num_agents].push_back(i);
            temp_t /= num_agents;
        }

        double current_min_proportion = numeric_limits<double>::max();
        for (int i = 0; i < num_agents; ++i) {
            int bundle_value = total_value(utilities, i, current_allocation[i]);
            int total_utility = total_agent_utility(utilities, i);
            double proportion = (total_utility > 0) ? (double)bundle_value / total_utility : 0;
            current_min_proportion = min(current_min_proportion, proportion);
        }
        
        if (abs(current_min_proportion - max_min_proportion) < MM_EPSILON) {
            mm_allocations.push_back(current_allocation);
            if (isEFX(current_allocation, utilities)) {
                mm_efx_allocations.push_back(current_allocation);
            }
        }
    }
    
    // --- Output Results ---
    if (argc > 1 && string(argv[1]) == "--automate") {
        json result;
        result["max_min_proportion"] = max_min_proportion;
        result["mm_allocations_count"] = mm_allocations.size();
        result["mm_efx_allocations_count"] = mm_efx_allocations.size();
        
        // Optional: include the allocations themselves if needed
        // json mm_allocs_json = json::array();
        // for(const auto& alloc : mm_allocations){
        //     mm_allocs_json.push_back(alloc);
        // }
        // result["mm_allocations"] = mm_allocs_json;

        cout << result.dump() << endl;
    } else {
        // Your original interactive output logic here...
    }

    return 0;
}
