#include <iostream>
#include "big_nums.h"

int main() {
    // try {
        // Test Constructors
        BigInteger num1(std::numeric_limits<int64_t>::min());
        BigInteger num2("18041289057982347");
        BigInteger num3("6658999999999999999999999999999999999999999999999999999999999999999999999999999999999000888590423848943890401980942340925809848943890401980942340925884894389040198094234092588489438904019809423409258000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555000000055555555");

        BigInteger num4("999999999999999999999999999");

        std::cout << "num1: " << num1 << std::endl;
        // std::cout << "num2: " << -(-num2) << std::endl;
        // std::cout << "num3: " << num3 << std::endl;
        // std::cout << "num4: " << num4 << std::endl;

    //     // Test Multiplication
    // BigInteger product = num4 * num3;
    // std::cout << "num4 * num3: " << product << std::endl;

    //     // Test Addition
    // BigInteger sum = num1 + num3;
    // std::cout << "num1 + num3: " << sum << std::endl;
    //
    //     // Test Subtraction
    // BigInteger diff = num3 - num2;
    // std::cout << "num3 - num2: " << diff << std::endl;

    //
    //     // Test Division
    // BigInteger quotient = num1 / num4;
    // std::cout << "num1 / num4 : " << quotient << std::endl;
    //
    //     // Test Modulo
    //     BigInteger mod = num3 % num1;
    //     std::cout << "num3 % num1: " << mod << std::endl;
    //
    //
    //     // Test Comparison Operators
    //     std::cout << "num1 == num2: " << (num1 == num2) << std::endl;
    //     std::cout << "num1 != num2: " << (num1 != num2) << std::endl;
    //     std::cout << "num1 < num3: " << (num1 < num3) << std::endl;
    //     std::cout << "num1 > num2: " << (num1 > num2) << std::endl;
    //
    //     // Test Square Root
    // double sqrt_num3 = num3.sqrt();
    // double sqrt_num4 = num4.sqrt();
    // std::cout << "sqrt(num3): " << sqrt_num3 << std::endl;
    // std::cout << "sqrt(num4): " << sqrt_num4 << std::endl;
    //
    //     // Test Error on Negative Square Root
    //     try {
    //         std::cout << "Trying sqrt(num2): ";
    //         double sqrt_num2 = num2.sqrt();
    //         std::cout << sqrt_num2 << std::endl;
    //     } catch (const std::runtime_error& e) {
    //         std::cout << e.what() << std::endl;
    //     }
    //
    //     // Test Large Number Square Root
    //     BigInteger big_num("1234567890123456789012345678901234567890");
    //     try {
    //         std::cout << "sqrt(big_num): " << big_num.sqrt() << std::endl;
    //     } catch (const std::runtime_error& e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // } catch (const std::exception& e) {
    //     std::cerr << "Exception occurred: " << e.what() << std::endl;
    // }

    return 0;
}
