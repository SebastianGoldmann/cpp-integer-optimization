
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <algorithm> 
#include <functional>


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
std::vector<float> generate_weights(const std::string& pi){

    std::vector<float> weights;

    size_t limit = std::min(pi.length(), size_t(40000));
    if (limit < 4) return weights;

    weights.reserve(limit / 4);

    for (size_t i = 0; i + 3 < limit; i += 4) {

        int num = (pi[i]     - '0') * 1000 +
                  (pi[i + 1] - '0') * 100  +
                  (pi[i + 2] - '0') * 10   +
                  (pi[i + 3] - '0');

        weights.push_back(num / 10000.0f);
    }
    return weights;
}


// compute lower bound
double compute_lower_bound(const std::vector<float>& weights){
    
    double counter = 0;

    for (const auto& value : weights){
        counter = counter + value;
    }
    return std::ceil(counter);
}

float sum_bin(const std::vector<float>& bin, const float& new_weight){

    float counter = 0.0f;
    for (auto item : bin){
        counter += item;
    }
    return counter + new_weight;
}

float calc_score(const std::vector<std::vector<float>>& bins){
    float penalty = 0;
    float bin_volume;
    for (const auto& bin : bins){
        bin_volume = 0;
        for (auto value : bin){
            bin_volume += value;
        }
        if (bin_volume > 1){
            penalty += 1000;
        }
        else {
            penalty += bin_volume;
        }
    }
    return penalty;
}


int main() {

    // read in Pi
    std::string pi = read_pi("pi_digits.txt"); 

    // generate weights
    std::vector<float> weights = generate_weights(pi);

    // Task 1: generate lower bound
    double lower_bound = compute_lower_bound(weights);
    std::cout << "Lower bound of the optimization problem: " << lower_bound  << std::endl;

    // Task 2: Constructive heuristic

    // sort
    std::sort(weights.begin(), weights.end(), std::greater<float>());

    // create storage for bins empty array of zero's
    std::vector<std::vector<float>> bins;

    // loop over weights to allocate
    for (auto& weight : weights){
        // assign first bin if all are empty.
        if (bins.empty()){
            bins.push_back({weight});
        }
        else {
            bool placed = false;
            for (auto& bin : bins){

                if (sum_bin(bin, weight) <= 1.0f) {
                    // add to current bin
                    bin.push_back(weight);
                    placed = true;
                    break;
                }
            }
            if (!placed){
                // there were no bins available
                bins.push_back({weight});

            }
        }
    }
    
    float score = calc_score(bins);
    std::cout << "total score First Fit decreasing: " << score << std::endl;


    return 0;
}