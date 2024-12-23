#include <iostream>
#include <fstream>
#include <chrono> // For measuring time
#include "big_nums.h"

int main() {
    // Start measuring time
    auto start = std::chrono::high_resolution_clock::now();

    // Your existing code
    std::ifstream inFile("bigNum.txt");
    std::string largeNumber;
    std::getline(inFile, largeNumber);
    inFile.close();

    // Construct the BigInteger from the file-loaded string
    BigInteger numFile(largeNumber);
    BigInteger num1(123);
    BigInteger num2("-0");
    BigInteger num3("6658999999999999999999999999999999999999999999999999999999999999999999999999999999999000888590423848943890401980942340925809848943890401980942340925884894389040198094234092588489438904019809423409258000000055555555");
    BigInteger num4("-999999999999999999999999999");

    // Print numbers
    std::cout << "numFile: " << numFile << std::endl;
    std::cout << "num1: " << num1 << std::endl;
    std::cout << "num2: " << -(-num2) << std::endl;
    std::cout << "num3: " << num3 << std::endl;
    std::cout << "num4: " << num4 << std::endl;

    // Test Addition
    BigInteger sum = num1 + num2;
    std::cout << "num4 + num1: " << sum << std::endl;

    // Test Subtraction
    BigInteger diff = num1 - num2;
    std::cout << "num1 - num4: " << diff << std::endl;

    // End measuring time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate duration
    std::chrono::duration<double> elapsed = end - start;

    // Print execution time
    std::cout << "Execution time: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}
