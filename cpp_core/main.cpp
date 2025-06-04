#include "allocation.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <iterator>

using namespace std;

void print_matrix(const Utilities& utils) {
    cout << "[\n";
    for (const auto& row : utils) {
        cout << "  [ ";
        for (int val : row) cout << val << " ";
        cout << "]\n";
    }
    cout << "]\n";
}

int main() {
    ifstream infile("input.txt");
    if (!infile) {
        cerr << "Failed to open input.txt\n";
        return 1;
    }

    Utilities utils;
    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        vector<int> row((istream_iterator<int>(iss)), istream_iterator<int>());
        if (!row.empty()) utils.push_back(row);
    }

    if (utils.empty() || utils[0].empty()) {
        cerr << "Input matrix is empty or invalid.\n";
        return 1;
    }

    cout << "Input matrix:\n";
    print_matrix(utils);

    int num_items = static_cast<int>(utils[0].size());
    auto start = chrono::high_resolution_clock::now();
    bool result = hasEFXAllocation(utils, num_items);
    auto end = chrono::high_resolution_clock::now();

    cout << "EFX allocation exists: " << (result ? "YES" : "NO") << "\n";
    cout << "Time taken: " << chrono::duration<double>(end - start).count() << " seconds\n";

    return 0;
}