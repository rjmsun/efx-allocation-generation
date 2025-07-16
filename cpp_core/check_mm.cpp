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

using namespace std;

// total utility for an agent across all items
int total_agent_utility(const Utilities& utils, int agent) {
    int total = 0;
    for (size_t j = 0; j < utils[agent].size(); ++j) {
        total += utils[agent][j];
    }
    return total;
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

void print_allocation(const Allocation& allocation, const Utilities& utils, ostream& out) {
    out << "Allocation:\n";
    for (size_t i = 0; i < allocation.size(); i++) {
        out << "Agent " << i << " gets bundle {";
        for (size_t j = 0; j < allocation[i].size(); j++) {
            out << allocation[i][j] << " ";
        }
        int bundle_value = total_value(utils, i, allocation[i]);
        int total_utility = total_agent_utility(utils, i);
        double proportion = (double)bundle_value / total_utility;
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
    
    uniform_int_distribution<> high_value(80, 100);    // Universally high value items
    uniform_int_distribution<> low_value(1, 20);       // Poisonous items
    uniform_int_distribution<> sporadic(1, 100);       // Sporadic value items
    
    // First item is universally high value
    for (int i = 0; i < num_agents; ++i) {
        utils[i][0] = high_value(gen);
    }
    
    // Second item is poisonous
    for (int i = 0; i < num_agents; ++i) {
        utils[i][1] = low_value(gen);
    }
    
    // Third item is universally high value
    for (int i = 0; i < num_agents; ++i) {
        utils[i][2] = high_value(gen);
    }
    
    // Fourth item is poisonous
    for (int i = 0; i < num_agents; ++i) {
        utils[i][3] = low_value(gen);
    }
    
    // Remaining items have sporadic values
    for (int j = 4; j < num_items; ++j) {
        for (int i = 0; i < num_agents; ++i) {
            utils[i][j] = sporadic(gen);
        }
    }
    
    return utils;
}

Utilities read_utilities_from_file(const string& filename, int num_agents, int num_items) {
    Utilities utils(num_agents, vector<int>(num_items));
    ifstream file(filename);
    
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        // return default utilities if file can't be opened
        return generate_random_utilities(num_agents, num_items);
    }
    
    string line;
    int agent = 0;
    
    while (getline(file, line) && agent < num_agents) {
        // remove brackets and extra whitespace
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

Utilities generate_builtin_utilities() {
    Utilities utils(4, vector<int>(10));
    utils[0] = {1, 1, 1, 1, 100, 1, 100, 1, 1, 1};
    utils[1] = {1, 1, 1, 1, 100, 1, 100, 1, 1, 1};
    utils[2] = {75, 7, 6, 73, 95, 97, 66, 28, 85, 27};
    utils[3] = {60, 13, 87, 81, 35, 28, 62, 23, 33, 94};
    return utils;
}

int main() {
    // Open output file
    ofstream outfile("mm.txt");
    if (!outfile) {
        cerr << "Error: Could not open output file mm.txt" << endl;
        return 1;
    }

    int num_agents = 4;
    int num_items = 10; 

    cout << "--- MM and EFX Allocation Finder ---\n";
    outfile << "--- MM and EFX Allocation Finder ---\n";
    /*
    cout << "Enter number of agents (e.g., 4): ";
    cin >> num_agents;
    cout << "Enter number of items (e.g., 10). Warning: >12 for 4 agents can be very slow: ";
    cin >> num_items;
    */

    Utilities utilities;
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
        utilities = generate_random_utilities(num_agents, num_items);
    } else if (choice == 'M' || choice == 'm') {
        utilities = get_manual_utilities(num_agents, num_items);
    } else if (choice == 'F' || choice == 'f') {
        utilities = generate_fixed_pattern_utilities(num_agents, num_items);
    } else if (choice == 'B' || choice == 'b') {
        utilities = generate_builtin_utilities();
    } else if (choice == 'T' || choice == 't') {
        utilities = read_utilities_from_file("check_mm_utilities.txt", num_agents, num_items);
    } else {
        cout << "Invalid choice, defaulting to random utilities.\n";
        utilities = generate_random_utilities(num_agents, num_items);
    }
    
    print_utilities(utilities, cout);
    print_utilities(utilities, outfile);
    
    // Safety check for computation time
    long double total_possible_allocations_ld = pow(num_agents, num_items);
    if (total_possible_allocations_ld > 2000000000.0) { // Approx 2 billion
         cout << "\nWarning: The number of possible allocations (" << (long long)total_possible_allocations_ld 
              << ") is very large. This may take a long time.\nContinue? (y/n): ";
        outfile << "\nWarning: The number of possible allocations (" << (long long)total_possible_allocations_ld 
                << ") is very large. This may take a long time.\n";
        char proceed;
        cin >> proceed;
        if (proceed != 'y' && proceed != 'Y') {
            return 0;
        }
    }
    long long total_possible_allocations = (long long)total_possible_allocations_ld;

    auto start_time = chrono::high_resolution_clock::now();

    // --- Pass 1: Find the maximum-minimum proportion of utility ---
    cout << "\nStarting Pass 1: Finding the maximinal proportion of utility...\n";
    outfile << "\nStarting Pass 1: Finding the maximinal proportion of utility...\n";
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
            double proportion = (double)bundle_value / total_utility;
            current_min_proportion = min(current_min_proportion, proportion);
        }
        max_min_proportion = max(max_min_proportion, current_min_proportion);
    }
    cout << "Pass 1 Complete. The highest possible minimum proportion of utility is: " 
         << fixed << setprecision(2) << (max_min_proportion * 100) << "%\n";
    outfile << "Pass 1 Complete. The highest possible minimum proportion of utility is: " 
            << fixed << setprecision(2) << (max_min_proportion * 100) << "%\n";

    // --- Pass 2: Find all MM allocations and check them for EFX ---
    cout << "\nStarting Pass 2: Finding all MM allocations and checking for EFX...\n";
    outfile << "\nStarting Pass 2: Finding all MM allocations and checking for EFX...\n";
    vector<Allocation> mm_allocations;
    vector<Allocation> mm_efx_allocations;

    const double MM_EPSILON = 0.0003; // 0.03% tolerance

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
            double proportion = (double)bundle_value / total_utility;
            current_min_proportion = min(current_min_proportion, proportion);
        }
        
        if (current_min_proportion >= max_min_proportion - MM_EPSILON) {
            mm_allocations.push_back(current_allocation);
            if (isEFX(current_allocation, utilities)) {
                mm_efx_allocations.push_back(current_allocation);
            }
        }
    }
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;

    cout << "\n--- ANALYSIS COMPLETE ---\n";
    outfile << "\n--- ANALYSIS COMPLETE ---\n";
    cout << "Time taken: " << elapsed.count() << " seconds\n";
    outfile << "Time taken: " << elapsed.count() << " seconds\n";
    cout << "Total allocations checked: " << total_possible_allocations << "\n";
    outfile << "Total allocations checked: " << total_possible_allocations << "\n";

    cout << "\nFound " << mm_allocations.size() << " Maximinal (MM) Allocations:\n";
    outfile << "\nFound " << mm_allocations.size() << " Maximinal (MM) Allocations:\n";
    for (const auto& alloc : mm_allocations) {
        print_allocation(alloc, utilities, cout);
        print_allocation(alloc, utilities, outfile);
    }

    cout << "\nFound " << mm_efx_allocations.size() << " Allocations that are both MM and EFX:\n";
    outfile << "\nFound " << mm_efx_allocations.size() << " Allocations that are both MM and EFX:\n";
    if (mm_efx_allocations.empty()) {
        cout << "  None.\n";
        outfile << "  None.\n";
    } else {
        for (const auto& alloc : mm_efx_allocations) {
            print_allocation(alloc, utilities, cout);
            print_allocation(alloc, utilities, outfile);
        }
    }

    outfile.close();
    return 0;
}