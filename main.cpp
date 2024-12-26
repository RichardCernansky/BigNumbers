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

    // Convert the vector of primes into a set for quick lookup
    std::set<int> prime_set(primes_up_to_200.begin(), primes_up_to_200.end());

    // Iterate through all numbers from 2 to 200 and test primality
    for (int n = 2; n <= 200; n++) {
        BigInteger num(n);
        bool result = num.is_prime();
        bool expected = (prime_set.find(n) != prime_set.end()); // Check if `n` is in the list of primes

        // Print test results
        std::cout << "Testing " << n << ": "
                  << (result ? "PRIME" : "NOT PRIME")
                  << " (Expected: " << (expected ? "PRIME" : "NOT PRIME") << ")\n";

        // Check if the test passed
        if (result != expected) {
            std::cerr << "Test failed for number: " << n << std::endl;
            return 1; // Exit with error
        }
    }

    std::cout << "All tests passed successfully for primes up to 200!\n";
    return 0;
}


