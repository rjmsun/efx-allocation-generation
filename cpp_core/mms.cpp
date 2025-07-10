#include "allocation.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iterator>
#include <limits>
using namespace std;

int mms_value(const Utilities& utils, int agent, int n_agents, int n_items) {
    int max_min = 0;
    int total_allocs = pow(n_agents, n_items);
    for (int mask = 0; mask < total_allocs; ++mask) {
        vector<vector<int>> bundles(n_agents);
        int x = mask;
        for (int i = 0; i < n_items; ++i) {
            int a = x % n_agents;
            bundles[a].push_back(i);
            x /= n_agents;
        }

        int min_val = numeric_limits<int>::max();
        for (int a = 0; a < n_agents; ++a) {
            int sum = 0;
            for (int item : bundles[a]) {
                sum += utils[agent][item];
            }
            min_val = min(min_val, sum);
        }
        max_min = max(max_min, min_val);
    }
    return max_min;
}

bool satisfies_mms(const Allocation& alloc, const Utilities& utils, double factor = 2.0/3.0) {
    int n = alloc.size();
    int num_items = utils[0].size();
    for (int i = 0; i < n; ++i) {
        int val = total_value(utils, i, alloc[i]);
        int mms = mms_value(utils, i, n, num_items);
        if (val < factor * mms) return false;
    }
    return true;
}

int main() {
    ifstream infile("input.txt");
    if (!infile) {
        cerr << "Error: Cannot open input.txt" << endl;
        return 1;
    }

    Utilities utils;
    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        vector<int> row((istream_iterator<int>(iss)), istream_iterator<int>());
        if (!row.empty()) utils.push_back(row);
    }

    int n_agents = utils.size();
    int n_items = utils[0].size();
    int total = pow(n_agents, n_items);
    int found = 0;

    for (int mask = 0; mask < total; ++mask) {
        vector<vector<int>> bundles(n_agents);
        int x = mask;
        for (int i = 0; i < n_items; ++i) {
            int a = x % n_agents;
            bundles[a].push_back(i);
            x /= n_agents;
        }

        Allocation alloc(n_agents);
        alloc = bundles;

        if (isEFX(alloc, utils) && satisfies_mms(alloc, utils)) {
            cout << "Found EFX and 2/3-MMS Allocation:\\n";
            for (int i = 0; i < n_agents; ++i) {
                cout << "Agent " << i << ": ";
                for (int item : alloc[i]) cout << item << " ";
                cout << " | Value: " << total_value(utils, i, alloc[i]) << endl;
            }
            ++found;
        }
    }

    cout << "Total qualifying allocations found: " << found << endl;
    return 0;
}