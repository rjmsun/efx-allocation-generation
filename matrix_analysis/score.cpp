// Enhanced H-score Analysis Tool
// Analyzes counterexample matrices and compares with various random matrix types
//
// The H-score is a weighted average of three metrics:
// 1. Item Contention Index (ICI): quantifies disagreement among agents
// 2. Agent Specialization Index (ASI): quantifies agent specialization
// 3. Top Value Asymmetry (TVA): quantifies imbalance in top values
//
// H-score = w1 * ICI + w2 * ASI + w3 * TVA

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>
#include <string>
#include <regex>
#include <iomanip>
using namespace std;

using Utilities = vector<vector<int>>;

// Statistics structure
struct MatrixStats {
    double ici;
    double asi;
    double tva;
    double h_score;
    
    MatrixStats(double i, double a, double t, double h) : ici(i), asi(a), tva(t), h_score(h) {}
};

// Gini coefficient calculation for vector<int>
double gini(const vector<int>& values) {
    int n = values.size();
    if (n == 0) return 0.0;
    double mean = accumulate(values.begin(), values.end(), 0.0) / n;
    if (mean == 0) return 0.0;
    double total_diff = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            total_diff += abs(values[i] - values[j]);
        }
    }
    return total_diff / (2.0 * n * n * mean);
}

// Gini coefficient calculation for vector<double>
double gini(const vector<double>& values) {
    int n = values.size();
    if (n == 0) return 0.0;
    double mean = accumulate(values.begin(), values.end(), 0.0) / n;
    if (mean == 0) return 0.0;
    double total_diff = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            total_diff += abs(values[i] - values[j]);
        }
    }
    return total_diff / (2.0 * n * n * mean);
}

// Variance calculation
double variance(const vector<double>& vals) {
    int n = vals.size();
    if (n == 0) return 0.0;
    double mean = accumulate(vals.begin(), vals.end(), 0.0) / n;
    double var = 0.0;
    for (double v : vals) var += (v - mean) * (v - mean);
    return var / n;
}

// ICI: mean Gini coefficient of columns
double ici(const Utilities& matrix) {
    if (matrix.empty() || matrix[0].empty()) return 0.0;
    int m = matrix[0].size();
    int n = matrix.size();
    double sum_gini = 0.0;
    for (int j = 0; j < m; ++j) {
        vector<int> col;
        for (int i = 0; i < n; ++i) col.push_back(matrix[i][j]);
        sum_gini += gini(col);
    }
    return sum_gini / m;
}

// Helper: Normalize a row so it sums to 1
vector<double> normalize_row_sum(const vector<int>& row) {
    double total = accumulate(row.begin(), row.end(), 0.0);
    vector<double> normed;
    if (total == 0) {
        normed.resize(row.size(), 0.0);
    } else {
        for (int v : row) normed.push_back(v / total);
    }
    return normed;
}

// Row variance for normalized row
double row_variance(const vector<double>& row) {
    int n = row.size();
    if (n == 0) return 0.0;
    double mean = accumulate(row.begin(), row.end(), 0.0) / n;
    double var = 0.0;
    for (double v : row) var += (v - mean) * (v - mean);
    return var / n;
}

// ASI: variance of agent specialization scores (variance of agent-wise Gini coefficients after normalization)
double asi(const Utilities& matrix) {
    if (matrix.empty()) return 0.0;
    vector<double> agent_ginis;
    for (const auto& row : matrix) {
        vector<double> normed = normalize_row_sum(row);
        agent_ginis.push_back(gini(normed));
    }
    return variance(agent_ginis);
}

// TVA: mean top-value asymmetry across items
double tva(const Utilities& matrix) {
    if (matrix.empty() || matrix[0].empty()) return 0.0;
    int m = matrix[0].size();
    int n = matrix.size();
    double sum_tva = 0.0;
    for (int j = 0; j < m; ++j) {
        vector<int> col;
        for (int i = 0; i < n; ++i) col.push_back(matrix[i][j]);
        sort(col.begin(), col.end(), greater<int>());
        int v1 = col[0];
        int v2 = (col.size() > 1) ? col[1] : 0;
        double tva_j = (v1 > 0) ? (double)(v1 - v2) / v1 : 0.0;
        sum_tva += tva_j;
    }
    return sum_tva / m;
}

// Calculate H-score
double h_score(const Utilities& matrix) {
    return ici(matrix) + 20 * asi(matrix) + tva(matrix);
}

// Calculate complete statistics for a matrix
MatrixStats calculate_stats(const Utilities& matrix) {
    return MatrixStats(ici(matrix), asi(matrix), tva(matrix), h_score(matrix));
}

// Print matrix to output stream
void print_matrix(const Utilities& matrix, ostream& out, const string& label = "") {
    if (!label.empty()) {
        out << label << ":\n";
    }
    for (size_t i = 0; i < matrix.size(); i++) {
        out << "  [";
        for (size_t j = 0; j < matrix[i].size(); j++) {
            out << matrix[i][j];
            if (j < matrix[i].size() - 1) out << ", ";
        }
        out << "]\n";
    }
}

// Print statistics to output stream
void print_stats(const MatrixStats& stats, ostream& out, const string& label = "") {
    if (!label.empty()) {
        out << label << ":\n";
    }
    out << "  ICI: " << fixed << setprecision(4) << stats.ici << "\n";
    out << "  ASI: " << fixed << setprecision(4) << stats.asi << "\n";
    out << "  TVA: " << fixed << setprecision(4) << stats.tva << "\n";
    out << "  H-score: " << fixed << setprecision(4) << stats.h_score << "\n";
}

// Calculate mean and standard deviation
pair<double, double> calculate_mean_std(const vector<double>& values) {
    if (values.empty()) return {0.0, 0.0};
    
    double mean = accumulate(values.begin(), values.end(), 0.0) / values.size();
    double variance = 0.0;
    for (double v : values) {
        variance += (v - mean) * (v - mean);
    }
    variance /= values.size();
    
    return {mean, sqrt(variance)};
}

// Parse counterexample matrices from file
vector<Utilities> parse_counterexamples(const string& filename) {
    vector<Utilities> matrices;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        return matrices;
    }

    string line;
    regex matrix_start("\\s*Utility Matrix:");
    while (getline(file, line)) {
        if (regex_search(line, matrix_start)) {
            Utilities matrix;
            // Read only lines that start with '[' immediately after 'Utility Matrix:'
            streampos last_pos = file.tellg();
            while (getline(file, line)) {
                string trimmed = line;
                // Remove leading whitespace
                trimmed.erase(trimmed.begin(), find_if(trimmed.begin(), trimmed.end(), [](unsigned char ch) { return !isspace(ch); }));
                if (trimmed.empty() || trimmed[0] != '[') {
                    // Not a matrix row, rewind one line for next section
                    if (file.good()) file.seekg(last_pos);
                    break;
                }
                // Parse matrix row - handle comma-separated values
                string cleaned = trimmed;
                cleaned.erase(remove(cleaned.begin(), cleaned.end(), '['), cleaned.end());
                cleaned.erase(remove(cleaned.begin(), cleaned.end(), ']'), cleaned.end());
                
                vector<int> row;
                stringstream ss(cleaned);
                string value_str;
                while (getline(ss, value_str, ',')) {
                    // Remove any remaining whitespace
                    value_str.erase(remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
                    if (!value_str.empty()) {
                        try {
                            row.push_back(stoi(value_str));
                        } catch (const exception& e) {
                            // Skip invalid values
                            continue;
                        }
                    }
                }
                if (!row.empty()) {
                    matrix.push_back(row);
                }
                last_pos = file.tellg();
            }
            if (!matrix.empty()) {
                matrices.push_back(matrix);
            }
        }
    }
    file.close();
    return matrices;
}

// Generate uniform random matrix
Utilities generate_uniform_matrix(int num_agents, int num_items, mt19937& gen) {
    Utilities matrix(num_agents, vector<int>(num_items));
    uniform_int_distribution<> distrib(1, 100);
    
    for (int i = 0; i < num_agents; ++i) {
        for (int j = 0; j < num_items; ++j) {
            matrix[i][j] = distrib(gen);
        }
    }
    return matrix;
}

// Generate matrix with poison items (low value items that most agents dislike)
Utilities generate_poison_matrix(int num_agents, int num_items, mt19937& gen) {
    Utilities matrix(num_agents, vector<int>(num_items));
    uniform_int_distribution<> high_value(50, 100);
    uniform_int_distribution<> poison_value(1, 20);
    uniform_int_distribution<> normal_value(10, 80);
    
    // First 30% of items are poison items
    int poison_count = max(1, num_items / 3);
    
    for (int j = 0; j < num_items; ++j) {
        if (j < poison_count) {
            // Poison items: most agents have low values
            for (int i = 0; i < num_agents; ++i) {
                matrix[i][j] = poison_value(gen);
            }
        } else {
            // Normal items
            for (int i = 0; i < num_agents; ++i) {
                matrix[i][j] = normal_value(gen);
            }
        }
    }
    
    return matrix;
}

// Generate matrix with universally high value items
Utilities generate_high_value_matrix(int num_agents, int num_items, mt19937& gen) {
    Utilities matrix(num_agents, vector<int>(num_items));
    uniform_int_distribution<> high_value(80, 100);
    uniform_int_distribution<> normal_value(20, 60);
    
    // First 30% of items are universally high value
    int high_count = max(1, num_items / 3);
    
    for (int j = 0; j < num_items; ++j) {
        if (j < high_count) {
            // High value items: all agents have high values
            for (int i = 0; i < num_agents; ++i) {
                matrix[i][j] = high_value(gen);
            }
        } else {
            // Normal items
            for (int i = 0; i < num_agents; ++i) {
                matrix[i][j] = normal_value(gen);
            }
        }
    }
    
    return matrix;
}

// Generate competitive matrix (items with high variance in agent preferences)
Utilities generate_competitive_matrix(int num_agents, int num_items, mt19937& gen) {
    Utilities matrix(num_agents, vector<int>(num_items));
    uniform_int_distribution<> high_value(70, 100);
    uniform_int_distribution<> low_value(1, 30);
    uniform_int_distribution<> normal_value(20, 60);
    
    // First 40% of items are competitive (high variance)
    int competitive_count = max(1, (num_items * 2) / 5);
    
    for (int j = 0; j < num_items; ++j) {
        if (j < competitive_count) {
            // Competitive items: some agents love them, others hate them
            for (int i = 0; i < num_agents; ++i) {
                if (gen() % 2 == 0) {
                    matrix[i][j] = high_value(gen);
                } else {
                    matrix[i][j] = low_value(gen);
                }
            }
        } else {
            // Normal items
            for (int i = 0; i < num_agents; ++i) {
                matrix[i][j] = normal_value(gen);
            }
        }
    }
    
    return matrix;
}

// Generate specialized matrix (agents have distinct preferences)
Utilities generate_specialized_matrix(int num_agents, int num_items, mt19937& gen) {
    Utilities matrix(num_agents, vector<int>(num_items));
    uniform_int_distribution<> high_value(70, 100);
    uniform_int_distribution<> low_value(1, 20);
    uniform_int_distribution<> normal_value(20, 50);
    
    // Each agent specializes in different items
    int items_per_specialty = max(1, num_items / num_agents);
    
    for (int i = 0; i < num_agents; ++i) {
        for (int j = 0; j < num_items; ++j) {
            if (j >= i * items_per_specialty && j < (i + 1) * items_per_specialty) {
                // Agent's specialty items
                matrix[i][j] = high_value(gen);
            } else {
                // Other items: low to normal values
                matrix[i][j] = (gen() % 3 == 0) ? low_value(gen) : normal_value(gen);
            }
        }
    }
    
    return matrix;
}

// Helper: Normalize all rows so each sums to 1
vector<vector<double>> normalize_rows(const Utilities& matrix) {
    vector<vector<double>> normed;
    for (const auto& row : matrix) {
        normed.push_back(normalize_row_sum(row));
    }
    return normed;
}

// Compute item-wise Gini coefficients (after row normalization)
vector<double> itemwise_gini(const Utilities& matrix) {
    auto normed = normalize_rows(matrix);
    int m = normed[0].size(), n = normed.size();
    vector<double> ginis;
    for (int j = 0; j < m; ++j) {
        vector<double> col;
        for (int i = 0; i < n; ++i) col.push_back(normed[i][j]);
        ginis.push_back(gini(col));
    }
    return ginis;
}

// Compute max pairwise normalized dot product between agent rows (after row normalization)
double max_pairwise_dot(const Utilities& matrix) {
    auto normed = normalize_rows(matrix);
    int n = normed.size();
    double max_dot = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dot = 0.0, norm_i = 0.0, norm_j = 0.0;
            for (size_t k = 0; k < normed[i].size(); ++k) {
                dot += normed[i][k] * normed[j][k];
                norm_i += normed[i][k] * normed[i][k];
                norm_j += normed[j][k] * normed[j][k];
            }
            if (norm_i > 0 && norm_j > 0) {
                dot /= (sqrt(norm_i) * sqrt(norm_j));
            } else {
                dot = 0.0;
            }
            if (dot > max_dot) max_dot = dot;
        }
    }
    return max_dot;
}

// Print item-wise Gini vector
void print_itemwise_gini(const vector<double>& ginis, ostream& out) {
    out << "  Item-wise Gini: [";
    for (size_t i = 0; i < ginis.size(); ++i) {
        out << fixed << setprecision(3) << ginis[i];
        if (i < ginis.size() - 1) out << ", ";
    }
    out << "]\n";
}

// Print max pairwise dot product
void print_max_pairwise_dot(double max_dot, ostream& out) {
    out << "  Max pairwise normalized dot product: " << fixed << setprecision(3) << max_dot << "\n";
}

int main() {
    cout << "=== Enhanced H-Score Analysis Tool ===\n";
    
    // Parse counterexamples
    cout << "Parsing counterexample matrices...\n";
    vector<Utilities> counterexamples = parse_counterexamples("../counterexample.txt");
    cout << "Found " << counterexamples.size() << " counterexample matrices\n";
    
    if (counterexamples.empty()) {
        cerr << "No counterexamples found. Exiting.\n";
        return 1;
    }
    
    // Get dimensions from first counterexample
    int num_agents = counterexamples[0].size();
    int num_items = counterexamples[0][0].size();
    cout << "Matrix dimensions: " << num_agents << " agents, " << num_items << " items\n";
    
    // Initialize random generator
    random_device rd;
    mt19937 gen(rd());
    
    // Generate different types of random matrices
    const int NUM_MATRICES = 100;
    vector<Utilities> uniform_matrices, poison_matrices, high_value_matrices, 
                     competitive_matrices, specialized_matrices;
    
    cout << "Generating " << NUM_MATRICES << " matrices of each type...\n";
    
    for (int i = 0; i < NUM_MATRICES; ++i) {
        uniform_matrices.push_back(generate_uniform_matrix(num_agents, num_items, gen));
        poison_matrices.push_back(generate_poison_matrix(num_agents, num_items, gen));
        high_value_matrices.push_back(generate_high_value_matrix(num_agents, num_items, gen));
        competitive_matrices.push_back(generate_competitive_matrix(num_agents, num_items, gen));
        specialized_matrices.push_back(generate_specialized_matrix(num_agents, num_items, gen));
    }
    
    // Analyze counterexamples
    cout << "Analyzing counterexamples...\n";
    ofstream counterfile("counterexample_analysis.txt");
    if (!counterfile.is_open()) {
        cerr << "Error: Could not open counterexample_analysis.txt\n";
        return 1;
    }
    
    counterfile << "=== COUNTEREXAMPLE ANALYSIS ===\n\n";
    vector<MatrixStats> counterexample_stats;
    
    for (size_t i = 0; i < counterexamples.size(); ++i) {
        counterfile << "Counterexample " << (i + 1) << ":\n";
        print_matrix(counterexamples[i], counterfile);
        MatrixStats stats = calculate_stats(counterexamples[i]);
        print_stats(stats, counterfile);
        print_itemwise_gini(itemwise_gini(counterexamples[i]), counterfile);
        print_max_pairwise_dot(max_pairwise_dot(counterexamples[i]), counterfile);
        counterexample_stats.push_back(stats);
        counterfile << "\n";
    }
    
    // Calculate statistics for counterexamples
    vector<double> counter_h_scores;
    for (const auto& stats : counterexample_stats) {
        counter_h_scores.push_back(stats.h_score);
    }
    
    auto counter_mean_std = calculate_mean_std(counter_h_scores);
    counterfile << "=== COUNTEREXAMPLE SUMMARY ===\n";
    counterfile << "Total counterexamples: " << counterexamples.size() << "\n";
    counterfile << "Mean H-score: " << fixed << setprecision(4) << counter_mean_std.first << "\n";
    counterfile << "Standard deviation: " << fixed << setprecision(4) << counter_mean_std.second << "\n";
    counterfile.close();
    
    // Analyze uniform matrices
    cout << "Analyzing uniform matrices...\n";
    ofstream uniformfile("uniform_analysis.txt");
    uniformfile << "=== UNIFORM MATRIX ANALYSIS ===\n\n";
    
    vector<double> uniform_h_scores;
    for (size_t i = 0; i < uniform_matrices.size(); ++i) {
        uniformfile << "Uniform Matrix " << (i + 1) << ":\n";
        print_matrix(uniform_matrices[i], uniformfile);
        MatrixStats stats = calculate_stats(uniform_matrices[i]);
        print_stats(stats, uniformfile);
        print_itemwise_gini(itemwise_gini(uniform_matrices[i]), uniformfile);
        print_max_pairwise_dot(max_pairwise_dot(uniform_matrices[i]), uniformfile);
        uniform_h_scores.push_back(stats.h_score);
        uniformfile << "\n";
    }
    
    auto uniform_mean_std = calculate_mean_std(uniform_h_scores);
    uniformfile << "=== UNIFORM MATRIX SUMMARY ===\n";
    uniformfile << "Total matrices: " << uniform_matrices.size() << "\n";
    uniformfile << "Mean H-score: " << fixed << setprecision(4) << uniform_mean_std.first << "\n";
    uniformfile << "Standard deviation: " << fixed << setprecision(4) << uniform_mean_std.second << "\n";
    uniformfile.close();
    
    // Analyze poison matrices
    cout << "Analyzing poison matrices...\n";
    ofstream poisonfile("poison_analysis.txt");
    poisonfile << "=== POISON MATRIX ANALYSIS ===\n\n";
    
    vector<double> poison_h_scores;
    for (size_t i = 0; i < poison_matrices.size(); ++i) {
        poisonfile << "Poison Matrix " << (i + 1) << ":\n";
        print_matrix(poison_matrices[i], poisonfile);
        MatrixStats stats = calculate_stats(poison_matrices[i]);
        print_stats(stats, poisonfile);
        print_itemwise_gini(itemwise_gini(poison_matrices[i]), poisonfile);
        print_max_pairwise_dot(max_pairwise_dot(poison_matrices[i]), poisonfile);
        poison_h_scores.push_back(stats.h_score);
        poisonfile << "\n";
    }
    
    auto poison_mean_std = calculate_mean_std(poison_h_scores);
    poisonfile << "=== POISON MATRIX SUMMARY ===\n";
    poisonfile << "Total matrices: " << poison_matrices.size() << "\n";
    poisonfile << "Mean H-score: " << fixed << setprecision(4) << poison_mean_std.first << "\n";
    poisonfile << "Standard deviation: " << fixed << setprecision(4) << poison_mean_std.second << "\n";
    poisonfile.close();
    
    // Analyze high value matrices
    cout << "Analyzing high value matrices...\n";
    ofstream highvaluefile("high_value_analysis.txt");
    highvaluefile << "=== HIGH VALUE MATRIX ANALYSIS ===\n\n";
    
    vector<double> high_value_h_scores;
    for (size_t i = 0; i < high_value_matrices.size(); ++i) {
        highvaluefile << "High Value Matrix " << (i + 1) << ":\n";
        print_matrix(high_value_matrices[i], highvaluefile);
        MatrixStats stats = calculate_stats(high_value_matrices[i]);
        print_stats(stats, highvaluefile);
        print_itemwise_gini(itemwise_gini(high_value_matrices[i]), highvaluefile);
        print_max_pairwise_dot(max_pairwise_dot(high_value_matrices[i]), highvaluefile);
        high_value_h_scores.push_back(stats.h_score);
        highvaluefile << "\n";
    }
    
    auto high_value_mean_std = calculate_mean_std(high_value_h_scores);
    highvaluefile << "=== HIGH VALUE MATRIX SUMMARY ===\n";
    highvaluefile << "Total matrices: " << high_value_matrices.size() << "\n";
    highvaluefile << "Mean H-score: " << fixed << setprecision(4) << high_value_mean_std.first << "\n";
    highvaluefile << "Standard deviation: " << fixed << setprecision(4) << high_value_mean_std.second << "\n";
    highvaluefile.close();
    
    // Analyze competitive matrices
    cout << "Analyzing competitive matrices...\n";
    ofstream competitivefile("competitive_analysis.txt");
    competitivefile << "=== COMPETITIVE MATRIX ANALYSIS ===\n\n";
    
    vector<double> competitive_h_scores;
    for (size_t i = 0; i < competitive_matrices.size(); ++i) {
        competitivefile << "Competitive Matrix " << (i + 1) << ":\n";
        print_matrix(competitive_matrices[i], competitivefile);
        MatrixStats stats = calculate_stats(competitive_matrices[i]);
        print_stats(stats, competitivefile);
        print_itemwise_gini(itemwise_gini(competitive_matrices[i]), competitivefile);
        print_max_pairwise_dot(max_pairwise_dot(competitive_matrices[i]), competitivefile);
        competitive_h_scores.push_back(stats.h_score);
        competitivefile << "\n";
    }
    
    auto competitive_mean_std = calculate_mean_std(competitive_h_scores);
    competitivefile << "=== COMPETITIVE MATRIX SUMMARY ===\n";
    competitivefile << "Total matrices: " << competitive_matrices.size() << "\n";
    competitivefile << "Mean H-score: " << fixed << setprecision(4) << competitive_mean_std.first << "\n";
    competitivefile << "Standard deviation: " << fixed << setprecision(4) << competitive_mean_std.second << "\n";
    competitivefile.close();
    
    // Analyze specialized matrices
    cout << "Analyzing specialized matrices...\n";
    ofstream specializedfile("specialized_analysis.txt");
    specializedfile << "=== SPECIALIZED MATRIX ANALYSIS ===\n\n";
    
    vector<double> specialized_h_scores;
    for (size_t i = 0; i < specialized_matrices.size(); ++i) {
        specializedfile << "Specialized Matrix " << (i + 1) << ":\n";
        print_matrix(specialized_matrices[i], specializedfile);
        MatrixStats stats = calculate_stats(specialized_matrices[i]);
        print_stats(stats, specializedfile);
        print_itemwise_gini(itemwise_gini(specialized_matrices[i]), specializedfile);
        print_max_pairwise_dot(max_pairwise_dot(specialized_matrices[i]), specializedfile);
        specialized_h_scores.push_back(stats.h_score);
        specializedfile << "\n";
    }
    
    auto specialized_mean_std = calculate_mean_std(specialized_h_scores);
    specializedfile << "=== SPECIALIZED MATRIX SUMMARY ===\n";
    specializedfile << "Total matrices: " << specialized_matrices.size() << "\n";
    specializedfile << "Mean H-score: " << fixed << setprecision(4) << specialized_mean_std.first << "\n";
    specializedfile << "Standard deviation: " << fixed << setprecision(4) << specialized_mean_std.second << "\n";
    specializedfile.close();
    
    // Print summary to console
    cout << "\n=== ANALYSIS COMPLETE ===\n";
    cout << "Files generated:\n";
    cout << "- counterexample_analysis.txt\n";
    cout << "- uniform_analysis.txt\n";
    cout << "- poison_analysis.txt\n";
    cout << "- high_value_analysis.txt\n";
    cout << "- competitive_analysis.txt\n";
    cout << "- specialized_analysis.txt\n\n";
    
    cout << "=== SUMMARY STATISTICS ===\n";
    cout << "Counterexamples: " << fixed << setprecision(4) << counter_mean_std.first 
         << " ± " << counter_mean_std.second << "\n";
    cout << "Uniform: " << fixed << setprecision(4) << uniform_mean_std.first 
         << " ± " << uniform_mean_std.second << "\n";
    cout << "Poison: " << fixed << setprecision(4) << poison_mean_std.first 
         << " ± " << poison_mean_std.second << "\n";
    cout << "High Value: " << fixed << setprecision(4) << high_value_mean_std.first 
         << " ± " << high_value_mean_std.second << "\n";
    cout << "Competitive: " << fixed << setprecision(4) << competitive_mean_std.first 
         << " ± " << competitive_mean_std.second << "\n";
    cout << "Specialized: " << fixed << setprecision(4) << specialized_mean_std.first 
         << " ± " << specialized_mean_std.second << "\n";
    
    return 0;
}