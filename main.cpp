#include <iostream>
#include <fstream>
#include <chrono> // For measuring time
#include "big_nums.h"

#include <iostream>
#include <vector>
#include <set>

// Assuming BigInteger is already defined and includes the is_prime method.

#include <iostream>
#include <vector>

// Assuming BigInteger is already implemented and includes the is_prime method.

int main() {
    // Precomputed list of all primes up to 200
    std::vector<int> primes_up_to_200 = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61,
        67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137,
        139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199
    };

    std::string json_input = R"({
        "op": "+",
        "left": 123,
        "right": {
            "op": "*",
            "left": "12345678901234567890",
            "right": {
                "op": "%",
                "left": "34",
                "right": 1
            }
        }
    })";
    return 0;
}


