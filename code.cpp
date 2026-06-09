
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>

std::string read_pi(const std::string filename) {
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


int main() {
    
    std::string pi = read_pi("pi_digits.txt"); 

    if (!pi.empty()) {
        std::cout << "Successfully read pi: " << pi.substr(0, 10) << "...\n";
    }


    std::cout << "First 50 characters: " << pi.substr(0, 50) << std::endl;

    std::vector<float> weights;
    weights.reserve(10000);

    for (int i = 0; i < 40000; i += 4) {
        std::string sub = pi.substr(i, 4);
        float value = std::stod(sub) / 10000.0;  // 3141 → 0.3141
        weights.push_back(value);
    }

    // Print normalized values
    std::cout << "Length of the array: " << weights.size() <<  std::endl;


    // compute lower bound
    double lower_bound;
    lower_bound = 0;

    for (const auto& value : weights){
        std::cout << "adding the following number " << value << std::endl;
        lower_bound = lower_bound + value;

    }
    lower_bound = std::ceil(lower_bound);
    std::cout << "Lower bound of the optimization problem: " << lower_bound  << std::endl;

    return 0;
}

