#include "allocation.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <numeric>
using namespace std;

// Helper: Normalize a row to sum to 1 (5 decimal places)
vector<double> normalize_row(const vector<int>& row) {
    double total = accumulate(row.begin(), row.end(), 0.0);
    vector<double> normed;
    if (total == 0) {
        normed.resize(row.size(), 0.0);
    } else {
        for (int v : row) normed.push_back(round((v / total) * 1e5) / 1e5);
    }
    return normed;
}

// Print normalized utility matrix
void print_normalized_matrix(const Utilities& matrix, ostream& out) {
    out << "Normalized Utility Matrix (rows sum to 1):\n";
    for (const auto& row : matrix) {
        vector<double> normed = normalize_row(row);
        out << "  [";
        for (size_t j = 0; j < normed.size(); ++j) {
            out << fixed << setprecision(5) << normed[j];
            if (j < normed.size() - 1) out << ", ";
        }
        out << "]\n";
    }
}

// Print allocation
void print_allocation(const Allocation& alloc, ostream& out) {
    for (size_t i = 0; i < alloc.size(); ++i) {
        out << "  Agent " << i << ": [";
        for (size_t j = 0; j < alloc[i].size(); ++j) {
            out << alloc[i][j];
            if (j < alloc[i].size() - 1) out << ", ";
        }
        out << "]\n";
    }
}

// Print envy edges
void print_envy_edges(const Allocation& alloc, const Utilities& utils, ostream& out) {
    set<pair<int, int>> envy = build_envy_graph(alloc, utils);
    out << "  Envy Edges: ";
    if (envy.empty()) {
        out << "None\n";
    } else {
        for (const auto& edge : envy) {
            out << "(" << edge.first << "->" << edge.second << ") ";
        }
        out << "\n";
    }
}

// Print allocation with value and percentage for each agent
void print_allocation_with_percent(const Allocation& alloc, const Utilities& utils, ostream& out) {
    for (size_t i = 0; i < alloc.size(); ++i) {
        int bundle_value = total_value(utils, i, alloc[i]);
        int total_utility = accumulate(utils[i].begin(), utils[i].end(), 0);
        double percent = (total_utility > 0) ? (100.0 * bundle_value / total_utility) : 0.0;
        out << "  Agent " << i << ": [";
        for (size_t j = 0; j < alloc[i].size(); ++j) {
            out << alloc[i][j];
            if (j < alloc[i].size() - 1) out << ", ";
        }
        out << "] (Value: " << bundle_value << ", " << fixed << setprecision(2) << percent << "% of total utility)\n";
    }
}

// Parse matrices from cmatrices.txt
vector<Utilities> parse_matrices(const string& filename) {
    vector<Utilities> matrices;
    ifstream file(filename);
    if (!file.is_open()) return matrices;
    string line;
    while (getline(file, line)) {
        if (line.find("Utility Matrix:") != string::npos) {
            Utilities matrix;
            while (getline(file, line)) {
                string trimmed = line;
                trimmed.erase(trimmed.begin(), find_if(trimmed.begin(), trimmed.end(), [](unsigned char ch) { return !isspace(ch); }));
                if (trimmed.empty() || trimmed[0] != '[') break;
                string cleaned = trimmed;
                cleaned.erase(remove(cleaned.begin(), cleaned.end(), '['), cleaned.end());
                cleaned.erase(remove(cleaned.begin(), cleaned.end(), ']'), cleaned.end());
                vector<int> row;
                stringstream ss(cleaned);
                string value_str;
                while (getline(ss, value_str, ',')) {
                    value_str.erase(remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
                    if (!value_str.empty()) row.push_back(stoi(value_str));
                }
                if (!row.empty()) matrix.push_back(row);
            }
            if (!matrix.empty()) matrices.push_back(matrix);
        }
    }
    file.close();
    return matrices;
}

// Generate all allocations (brute force)
void generate_allocations(int num_agents, int num_items, vector<Allocation>& out) {
    long long total = 1;
    for (int i = 0; i < num_items; ++i) total *= num_agents;
    for (long long t = 0; t < total; ++t) {
        Allocation alloc(num_agents);
        long long temp = t;
        for (int i = 0; i < num_items; ++i) {
            alloc[temp % num_agents].push_back(i);
            temp /= num_agents;
        }
        out.push_back(alloc);
    }
}

int main() {
    vector<Utilities> matrices = parse_matrices("cmatrices.txt");
    ofstream out("cmatrix_analysis.txt");
    if (!out.is_open()) {
        cerr << "Could not open output file." << endl;
        return 1;
    }
    int matrix_num = 0;
    for (const auto& utils : matrices) {
        matrix_num++;
        int num_agents = utils.size();
        int num_items = utils[0].size();
        out << "==================== COUNTEREXAMPLE MATRIX #" << matrix_num << " ====================\n";
        out << "Raw Utility Matrix:\n";
        print_allocation(utils, out);
        print_normalized_matrix(utils, out);
        // Generate all allocations
        vector<Allocation> all_allocs;
        generate_allocations(num_agents, num_items, all_allocs);
        // Single pass: collect MM, EFX, and candidate Pareto-optimal EFX allocations
        vector<Allocation> mm_allocs;
        vector<Allocation> efx_allocs;
        vector<double> min_props; // for MM
        vector<bool> is_efx; // for all_allocs
        double global_max_min = -1;
        for (const auto& alloc : all_allocs) {
            // Compute min-prop for MM
            double min_prop = numeric_limits<double>::max();
            for (int i = 0; i < num_agents; ++i) {
                int bundle_value = total_value(utils, i, alloc[i]);
                int total_utility = accumulate(utils[i].begin(), utils[i].end(), 0);
                double prop = (total_utility > 0) ? (double)bundle_value / total_utility : 0.0;
                min_prop = min(min_prop, prop);
            }
            min_props.push_back(min_prop);
            if (min_prop > global_max_min + 1e-8) {
                global_max_min = min_prop;
                mm_allocs.clear();
                mm_allocs.push_back(alloc);
            } else if (abs(min_prop - global_max_min) < 1e-8) {
                mm_allocs.push_back(alloc);
            }
            // EFX check
            bool efx = isEFX(alloc, utils);
            is_efx.push_back(efx);
            if (efx) efx_allocs.push_back(alloc);
        }
        // Build a map from all_allocs index to efx_allocs index
        vector<int> all_to_efx_idx(all_allocs.size(), -1);
        int efx_idx = 0;
        for (size_t i = 0; i < all_allocs.size(); ++i) {
            if (is_efx[i]) all_to_efx_idx[i] = efx_idx++;
        }
        // Find all Pareto-optimal EFX allocations and build a map from efx_allocs index to pareto number
        vector<size_t> pareto_idxs;
        vector<int> efx_idx_to_pareto_num(efx_allocs.size(), -1);
        int pareto_count = 0;
        for (size_t idx = 0; idx < efx_allocs.size(); ++idx) {
            const auto& alloc = efx_allocs[idx];
            bool pareto = true;
            for (size_t j = 0; j < efx_allocs.size(); ++j) {
                if (j == idx) continue;
                if (pareto_dominates(efx_allocs[j], alloc, utils)) {
                    pareto = false;
                    break;
                }
            }
            if (pareto) {
                pareto_idxs.push_back(idx);
                efx_idx_to_pareto_num[idx] = ++pareto_count;
            }
        }
        out << "\n--- MM Allocations (" << mm_allocs.size() << ") ---\n";
        for (const auto& alloc : mm_allocs) {
            print_allocation(alloc, out);
            print_envy_edges(alloc, utils, out);
            out << "\n";
        }
        // Find all EFX allocations
        // vector<Allocation> efx_allocs; // This line is now redundant as efx_allocs is populated in the single pass
        // for (const auto& alloc : all_allocs) { // This line is now redundant
        //     if (isEFX(alloc, utils)) efx_allocs.push_back(alloc); // This line is now redundant
        // }
        // Find all Pareto-optimal EFX allocations and build a map from allocation index to pareto number
        // vector<size_t> pareto_idxs; // This line is now redundant
        // vector<int> efx_idx_to_pareto_num(efx_allocs.size(), -1); // This line is now redundant
        // int pareto_count = 0; // This line is now redundant
        // for (size_t idx = 0; idx < efx_allocs.size(); ++idx) { // This line is now redundant
        //     const auto& alloc = efx_allocs[idx]; // This line is now redundant
        //     if (isParetoOptimalEFX(alloc, utils, efx_allocs)) { // This line is now redundant
        //         pareto_idxs.push_back(idx); // This line is now redundant
        //         efx_idx_to_pareto_num[idx] = ++pareto_count; // This line is now redundant
        //     } // This line is now redundant
        // } // This line is now redundant
        // Find all min-optimal EFX allocations and build a map from efx_allocs index to min-optimal status
        vector<bool> efx_idx_is_minopt(efx_allocs.size(), false);
        for (size_t idx = 0; idx < efx_allocs.size(); ++idx) {
            if (isMinOptimalEFX(efx_allocs[idx], utils, efx_allocs)) {
                efx_idx_is_minopt[idx] = true;
            }
        }
        // For each MM allocation, find closest EFX allocations (by swap distance)
        out << "--- Closest EFX Allocations to MM (by swap distance) ---\n";
        for (const auto& mm_alloc : mm_allocs) {
            int min_swap = numeric_limits<int>::max();
            vector<size_t> closest_idxs;
            for (size_t idx = 0; idx < efx_allocs.size(); ++idx) {
                int sw = swap_distance(mm_alloc, efx_allocs[idx]);
                if (sw < min_swap) {
                    min_swap = sw;
                    closest_idxs.clear();
                    closest_idxs.push_back(idx);
                } else if (sw == min_swap) {
                    closest_idxs.push_back(idx);
                }
            }
            out << "MM Allocation:\n";
            print_allocation_with_percent(mm_alloc, utils, out);
            out << "  Closest EFX allocation(s) (swap distance = " << min_swap << "):\n";
            for (size_t i = 0; i < closest_idxs.size(); ++i) {
                size_t efx_idx = closest_idxs[i];
                int pareto_num = efx_idx_to_pareto_num[efx_idx];
                bool is_minopt = efx_idx_is_minopt[efx_idx];
                out << "\nClose EFX allocation #" << (efx_idx+1);
                if (pareto_num > 0 && is_minopt) {
                    out << " (Pareto Optimal EFX #" << pareto_num << ", Min-optimal)\n";
                } else if (pareto_num > 0) {
                    out << " (Pareto Optimal EFX #" << pareto_num << ", Not Min-optimal)\n";
                } else if (is_minopt) {
                    out << " (Not Pareto Optimal EFX, Min-optimal)\n";
                } else {
                    out << " (Not Pareto Optimal EFX, Not Min-optimal)\n";
                }
                print_allocation_with_percent(efx_allocs[efx_idx], utils, out);
            }
            out << "\n";
        }
        out << string(80, '=') << "\n\n";
    }
    out.close();
    cout << "Analysis complete. Output written to cmatrix_analysis.txt\n";
    return 0;
}
