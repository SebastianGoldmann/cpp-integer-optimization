
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <sstream>
#include <functional>
#include <iomanip>
#include <vector>
#include <utility>
#include <set>
#include <algorithm>


struct Item {
    int id;       
    float weight; 
};

struct Bin {
    std::vector<Item> items;
    float current_weight = 0.0f;
};


// Read Pi into memory
std::string read_pi(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return ""; 
    }
    
    // Read all digits from file
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

// generate weights of the items
std::vector<float> generate_weights(const std::string& pi, const int& n){

    std::vector<float> weights;

    size_t limit = std::min(pi.length(), size_t(4*n));
    if (limit < 5) return weights;

    weights.reserve(limit / 4);

    for (size_t i = 0; i + 3 < limit; i += 4) {

        int num = (pi[i]     - '0') * 1000 +
                  (pi[i + 1] - '0') * 100  +
                  (pi[i + 2] - '0') * 10   +
                  (pi[i + 3] - '0');

        float value = 0.26f + (num / 10000.f) * 0.25f;;
        weights.push_back(value);
    }
    
    return weights;
}

using Edge = std::pair<int, int>;
using EdgeList = std::vector<Edge>;

// generate conflicts of the items
EdgeList generate_edge_conflicts(const std::string& pi, const int& conflict_nmbr, const int& n) {

    std::vector<int> conflicts;
    
    // 2. Parse 5-digit chunks efficiently
    for (size_t i = 0; i + 4 < pi.length(); i += 5) {
        int num = (pi[i]     - '0') * 10000 +
                  (pi[i + 1] - '0') * 1000  +
                  (pi[i + 2] - '0') * 100   +
                  (pi[i + 3] - '0') * 10    + 
                  (pi[i + 4] - '0');

        conflicts.push_back(num % n);
    }

    std::set<Edge> unique_edges;
    size_t target = static_cast<size_t>(conflict_nmbr);
    bool found_target = false;

    // 3. Generate pairs
    for (size_t i = 0; i + 1 < conflicts.size(); ++i) {
            
        if (unique_edges.size() == target) {
                found_target = true;
                break;
            }

        int u = conflicts[i];
        int v = conflicts[i+1];
        if (u == v) continue; // skip if same number

        // std::minmax returns a std::pair
        unique_edges.insert(std::minmax(u, v));
    }

    if (!found_target) {
        std::clog << "[ERROR] The Pi number was not long enough to generate enough edges.\n";
    }
    return EdgeList(unique_edges.begin(), unique_edges.end());;
    
}

// compute lower bound
double compute_lower_bound(const std::vector<float>& weights){
    
    double counter = 0;
    const double epsilon = 1e-9;
    for (const auto& value : weights){
        counter = counter + value;
    }
    return std::ceil(counter - epsilon);
}


float calc_score(const std::vector<Bin>& bins) {
    float penalty = 0.0f;

    for (const auto& bin : bins) {
        // 1. Read the pre-calculated volume directly from the struct
        float bin_volume = bin.current_weight;

        // 2. Apply your conditional logic using an epsilon buffer for safety
        const float epsilon = 1e-5f;
        if (bin_volume > 1.0f + epsilon) {
            penalty += 1000.0f;
        } 
        else {
            // Hint: Add the wasted space to your penalty tracker
            penalty += (1.0f - bin_volume);
        }
    }
    return penalty;
}

void export_bins_to_csv(const std::vector<Bin>& bins, const std::string& filename) {
    std::ofstream csv_file(filename);

    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return;
    }

    // Write header
    csv_file << "Bin_ID,Total_Weight,Items\n";
    csv_file << std::fixed << std::setprecision(4);

    for (size_t i = 0; i < bins.size(); ++i) {
        float total_weight = bins[i].current_weight;
        
        std::ostringstream items_stream;
        items_stream << std::fixed << std::setprecision(4);

        // Loop through the items vector stored inside this specific Bin
        for (size_t j = 0; j < bins[i].items.size(); ++j) {
            
            // UNDER THE HOOD: Stream out the ID, followed by the weight in parentheses
            items_stream << bins[i].items[j].id << "(" << bins[i].items[j].weight << ")";
            
            // Separate items within the same bin using a semicolon
            if (j < bins[i].items.size() - 1) {
                items_stream << "; ";
            }
        }

        // Write the row to the CSV
        csv_file << "Bin " << (i + 1) << "," 
                 << total_weight << ",\"" 
                 << items_stream.str() << "\"\n";
    }

    csv_file.close();
    std::cout << "Successfully exported bins data to " << filename << "\n";
}

void export_weights_to_csv(const std::vector<float>& weights, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return;
    }

    file << "weight\n"; // Header
    file << std::fixed << std::setprecision(4);

    for (const auto& weight : weights) {
        file << weight << "\n";
    }

    file.close();
    std::cout << "Successfully exported weights to " << filename << "\n";
}

void export_edges_to_csv(const EdgeList& edges, const std::string& filename) {
    std::ofstream edge_file(filename);

    if (!edge_file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        return;
    }

    edge_file << "Node_A,Node_B\n"; // CSV Header
    for (const auto& edge : edges) {
        edge_file << edge.first << "," << edge.second << "\n";
    }

    edge_file.close();
    std::cout << "Successfully exported edges to " << filename << "\n";
}

bool edge_conflict(const Bin& current_bin , const Item& current_item, const EdgeList& edge_list){

    // select all links not allowed with current_item
    for (const Item& existing_item : current_bin.items) {
        // 1. Create the pair to test using minmax
        std::pair<int, int> test_pair = std::minmax(current_item.id, existing_item.id);

        if (std::find(edge_list.begin(), edge_list.end(), test_pair) != edge_list.end()) {
        return true; // Conflict found! Stop immediately and return true.
        }
    }
    return false;
}

void run_tests() {
    std::cout << "\n=== RUNNING CODES TESTS ===\n";

    // ----------------------------------------------------------------
    // TEST 1: Basic Fit (Item should successfully fit into the bin)
    // ----------------------------------------------------------------
    {
        Bin test_bin;
        test_bin.items.push_back({0, 0.4f});
        test_bin.current_weight = 0.4f;

        Item new_item = {1, 0.3f}; // Weight 0.3 fits perfectly with 0.4
        EdgeList no_edges;         // No conflicts

        if (test_bin.current_weight + new_item.weight <= 1.0f && !edge_conflict(test_bin, new_item, no_edges)) {
            std::cout << "[PASS] Test 1: Item successfully allowed in bin.\n";
        } else {
            std::cout << "[FAIL] Test 1: Item was blocked unexpectedly.\n";
        }
    }

    // ----------------------------------------------------------------
    // TEST 2: Capacity Block (Item is too heavy for the remaining space)
    // ----------------------------------------------------------------
    {
        Bin test_bin;
        test_bin.items.push_back({0, 0.7f});
        test_bin.current_weight = 0.7f;

        Item heavy_item = {1, 0.4f}; // 0.7 + 0.4 = 1.1 (Exceeds 1.0 limit!)
        EdgeList no_edges;

        if (test_bin.current_weight + heavy_item.weight <= 1.0f && !edge_conflict(test_bin, heavy_item, no_edges)) {
            std::cout << "[FAIL] Test 2: Heavy item accidentally allowed in bin.\n";
        } else {
            std::cout << "[PASS] Test 2: Heavy item correctly blocked by capacity rules.\n";
        }
    }

    // ----------------------------------------------------------------
    // TEST 3: Graph Conflict Block (Item has space, but is forbidden by edges)
    // ----------------------------------------------------------------
    {
        Bin test_bin;
        test_bin.items.push_back({5, 0.2f}); // Contains Item ID 5
        test_bin.current_weight = 0.2f;

        Item conflict_item = {12, 0.2f}; // Contains Item ID 12. Combined weight is only 0.4f (fits!)
        
        // Formally establish that Item 5 and Item 12 hate each other
        EdgeList edges;
        edges.push_back(std::minmax(5, 12)); 

        if (test_bin.current_weight + conflict_item.weight <= 1.0f && !edge_conflict(test_bin, conflict_item, edges)) {
            std::cout << "[FAIL] Test 3: Conflicting item accidentally allowed into the bin.\n";
        } else {
            std::cout << "[PASS] Test 3: Conflicting item correctly blocked by graph constraints.\n";
        }
    }

    std::cout << "=== TESTS COMPLETE ===\n\n";
}

int main() {
    run_tests();
    auto n = 500; // number of weights
    auto M = 25000; // number of unique edge links not allowed

    // read in Pi
    std::string pi = read_pi("pi_digits.txt"); 
    std::cout << "The length of pi is: " << pi.length() << std::endl;

    // generate weights and edges
    std::vector<float> weights = generate_weights(pi, n);
    EdgeList edges = generate_edge_conflicts(pi, M, n);

    // save weights
    export_weights_to_csv(weights, "weights.csv");
    export_edges_to_csv(edges, "edges_data.csv");

    // 1. Create the mapping while weights are in original order
    std::vector<Item> items;
    for (int i = 0; i < weights.size(); ++i) {
        items.push_back({i, weights[i]}); 
    }

    // 2. Sort the items vector by weight instead
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        return a.weight > b.weight;
    });

    // Task 1: generate lower bound
    double lower_bound = compute_lower_bound(weights);
    std::cout << "Lower bound of the optimization problem: " << lower_bound  << std::endl;

    // create empty storage for bins empty array of zero's
    std::vector<Bin> bins;

    // loop over weights to allocate
    for (auto& item : items){
        // assign first bin if all are empty.
        if (bins.empty()){
            Bin first_bin;
    
            first_bin.items.push_back(item);
            first_bin.current_weight = item.weight;
            bins.push_back(first_bin);
        }
        else {
            bool placed = false;
            for (auto& bin : bins){
                if (bin.current_weight + item.weight <= 1.0f and !edge_conflict(bin, item, edges)) {
                    // add to current bin
                    bin.items.push_back(item);
                    bin.current_weight = bin.current_weight + item.weight;
                    placed = true;
                    break;
                }
            }
            if (!placed){
                // there were no bins available generate a new bin and add the item
                Bin new_bin;
                new_bin.items.push_back(item);
                new_bin.current_weight = item.weight;
                bins.push_back(new_bin);
            }
        }
    }
    export_bins_to_csv(bins, "bin_packing_results.csv");

    float score = calc_score(bins);
    std::cout << "total bins used: " << bins.size() << std::endl;
    std::cout << "total score First Fit decreasing: " << score << std::endl;

    return 0;
}