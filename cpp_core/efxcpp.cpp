#include <iostream>
#include <vector>
#include <string>
#include <cmath>

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

// Print an allocation
void print_allocation(const Allocation& allocation) {
    cout << "Allocation:" << endl;
    for (int i = 0; i < allocation.size(); i++) {
        cout << "Agent " << i << " gets items: ";
        for (int item : allocation[i]) {
            cout << item << " ";
        }
        cout << endl;
    }
    cout << endl;
}

// Generate all possible allocations recursively
void generate_allocations(int item, int n_agents, int n_items, 
                         Allocation& current, vector<Allocation>& all_allocations) {
    if (item == n_items) {
        all_allocations.push_back(current);
        return;
    }
    
    // Try giving item to each agent
    for (int agent = 0; agent < n_agents; agent++) {
        current[agent].push_back(item);
        generate_allocations(item + 1, n_agents, n_items, current, all_allocations);
        current[agent].pop_back();
    }
}

int main() {
    // Get input dimensions
    int n_agents, n_items;
    cout << "Enter number of agents: ";
    cin >> n_agents;
    cout << "Enter number of items: ";
    cin >> n_items;
    
    // Get utility matrix
    cout << "Enter utility matrix (" << n_agents << "x" << n_items << "):" << endl;
    Utilities utils(n_agents, vector<int>(n_items));
    for (int i = 0; i < n_agents; i++) {
        cout << "Enter utilities for agent " << i << ": ";
        for (int j = 0; j < n_items; j++) {
            cin >> utils[i][j];
        }
    }
    
    // Generate all possible allocations
    vector<Allocation> all_allocations;
    Allocation current(n_agents);
    generate_allocations(0, n_agents, n_items, current, all_allocations);
    
    // Check each allocation for EFX
    cout << "\nChecking " << all_allocations.size() << " possible allocations...\n" << endl;
    
    int efx_count = 0;
    for (const auto& allocation : all_allocations) {
        if (is_efx(utils, allocation)) {
            cout << "Found EFX allocation #" << ++efx_count << ":" << endl;
            print_allocation(allocation);
        }
    }
    
    cout << "Total EFX allocations found: " << efx_count << endl;
    cout << "Total allocations checked: " << all_allocations.size() << endl;
    
    return 0;
}
